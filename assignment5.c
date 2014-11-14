#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/*
*	Authors: Matt Henry
*
*/
typedef struct dummy
{
	union versiontype 
	{
		uint8_t version;
		uint8_t type;
	}vt;
	uint8_t length;
	uint16_t s;
} dummy;

//TODO Length is wrong should be strictly number of bytes
//TODO	Add your names
//TODO	rewrite/clean up make pretty the code

void read_sixteenbit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_f(FILE * fp, uint8_t length);
void read_sixtyfourbit_f(FILE * fp, uint8_t length);
void read_junk(FILE * fp, uint8_t length);
void read_burn(FILE * fp, uint8_t length);
void read_ascii(FILE * fp, uint8_t length);
int read_data(FILE * fp, uint8_t type, uint8_t length, uint8_t version);
uint8_t checkchecksum(dummy * dp);

//global for skipping N datagrams
int skipnumber = 0;

//TYPES
const uint8_t SIXTEEN_BIT_INT = 0;
const uint8_t THIRTY_TWO_BIT_INTEGER = 1;
const uint8_t THIRTY_TWO_BIT_FLOATING = 2;
const uint8_t SIXTY_FOUR_BIT_FLOATING = 3;
const uint8_t ASCII_CHAR = 7;
const uint8_t JUNK = 8;
const uint8_t SKIP = 9;
const uint8_t BURN = 10;
const uint8_t STOP = 11;

int main(int argc, char ** argv)
{
	argv++; // make it look at first argument
	FILE * fp = NULL;
	dummy * headbuff = malloc(4); // all versions have a header 4 bytes long

	if (headbuff==NULL) { printf("Memory error\n"); return -1;}
	fp = fopen(*argv, "rb");
	if (fp==NULL) { printf("Error opening file\n"); return -2; }

	size_t result;
	
	while((result = fread(headbuff, 4, 1, fp)) == 1)
	{
		//Read in the header
		uint8_t theversion = headbuff->vt.version & 0x0F; //gets lower 4 bits
		uint8_t thetype = (headbuff->vt.type & 0xF0) >> 4; 	//gets higher 4 bits
		uint8_t thelength = headbuff->length - 4;			//takes away the header length
		uint8_t skipbit = headbuff->s & 0x1;				//takes first bit
		
		if(skipnumber > 0)
		{
			printf("\nWe read version %d type %d length %d skipbit %d\n",
					(int)theversion, (int)thetype, 
					(int)thelength, (int)skipbit);
			//read_junk(fp, typetosize(thetype) * thelength);//hack to skip right number of bytes
			read_junk(fp, (int)thelength);
			skipnumber--;			
		}

		//checks the skip bit if true proccess the next thelength bytes as junk
		if(skipbit) // if skipbit is set
		{
			printf("\nWe read version %d type %d length %d skipbit %d\n",
					(int)theversion, (int)thetype, 
					(int)thelength, (int)skipbit);
			//read_junk(fp, typetosize(thetype) * thelength);//hack to skip right number of bytes
			read_junk(fp, (int)thelength);
			continue;
		}

		//version 1 datagram
		if(theversion == 1)
		{
			printf("\nWe read version: %d type: %d length: %d skipbit: %d\n",
					(int)theversion, (int)thetype,
					(int)thelength ,(int)skipbit);

			if(read_data(fp, thetype, thelength, theversion)){ break;} //if true were done
			continue;
		} //end version 1
		
		//version 2 datagram
		if(theversion == 2)
		{
			uint8_t dupbit = headbuff->s & 0x2;
			uint8_t checksum = (headbuff->s & 0xFF00) >> 8;
			
			if(checksum != checkchecksum(headbuff))
			{
				printf("\nWe read version: %d type %d length: %d skipbit: %d dupbit: %d checksum: %d\n",
						(int)theversion, (int)thetype,
						(int)thelength, (int)skipbit,
						(int)dupbit,(int)checksum);
				printf("Bad Checksum\n");
				//read_junk(fp, typetosize(thetype) * (int)thelength);//hack to skip right number of bytes
				read_junk(fp, (int)thelength);
				continue; 
			}

			if(dupbit)
			{
				printf("\nWe read version: %d type %d length: %d skipbit: %d dupbit: %d\n",
						(int)theversion, (int)thetype,
						(int)thelength, (int)skipbit,
						(int)dupbit);

				fpos_t pos;
				fgetpos(fp, &pos); //remember position
				if(read_data(fp, thetype, thelength, theversion)){ break;} //if true we're done
				fsetpos(fp, &pos);
				if(read_data(fp, thetype, thelength, theversion)){ break;} //Read it again
				continue;
			}//end if dupbit

			if(read_data(fp, thetype, thelength, theversion)){ break;} //if true were done
			continue;
		} //end version 2

		//version 3 datagram
		if(theversion == 3)
		{
			uint8_t id = headbuff->s & 0xFE;
			uint8_t checksum = (headbuff->s & 0xFF00) >> 8;

			if(checksum != checkchecksum(headbuff))
			{ 
				printf("\nWe read version: %d type %d length: %d skipbit: %d checksum: %d\n",
						(int)theversion, (int)thetype,
						(int)thelength, (int)skipbit,
						(int)checksum);

				printf("Bad Checksum\n");
				//read_junk(fp, typetosize(thetype) * (int)thelength);//hack to skip right number of bytes
				read_junk(fp, (int)thelength);
				continue;  
			}
			

			printf("\nWe read version: %d type: %d length: %d skipbit: %d id: %d checksum: %d\n",
					(int)theversion, (int)thetype,
					(int)thelength, (int)skipbit,
					(int)id, (int)checksum);

			if(read_data(fp, thetype, thelength, theversion)){ break;} //if true were done
			continue;
		} //end version 3
	}//end while

	free(headbuff);
	fclose(fp);
	return 0;
}//end main

///////////////////////////////////////////////////////////
////////////////// Functions //////////////////////////////
///////////////////////////////////////////////////////////
//Big ol switch for which kind of data to read in
int read_data(FILE * fp, uint8_t type, uint8_t length, uint8_t version)
{
	switch(type)
	{
		case 0: // 16 bit int
		{
			read_sixteenbit_i(fp,length);
			break;
		}
		case 1: // 32 bit int
		{
			read_thirtytwobit_i(fp, length);
			break;
		}
		case 2: // 32 bit float
		{
			read_thirtytwobit_f(fp,length);
			break;
		}
		case 3: // 64 bit float
		{
			read_sixtyfourbit_f(fp,length);
			break;
		}
		case 7: // ascii
		{
			read_ascii(fp,length);
			break;
		}
		case 8: // junk
		{
			read_junk(fp,length);
			break;
		}
		case 9: // skip
		{
			skipnumber = length;
			//read_skip(fp,length);
			break;
		}
		case 10: // burn
		{
			read_burn(fp,length);
			break;
		}
		case 11: // stop
		{
			return 1; //Were done
			break;
		}
		default:
		{
			break;
		}
	}
	return 0;
}


/////All assume length is the number of the items specified
///////////////////////////////////////////////////////////
//Reads length number of 16 bit integers
void read_sixteenbit_i(FILE * fp, uint8_t length)
{
	//uint16_t * numbers = malloc(sizeof(uint16_t) * length); //allocate space
	uint16_t * numbers = malloc(length); //allocate space

	uint16_t * temp = numbers;				//pointer for iterating
	int numberthings = length / 2;
	int result;	//for storing number of successful things read by fread
	int i = 0;	//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint16_t),numberthings,fp)) != numberthings)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < numberthings;i++)
	{
		printf("16 bit number %u\n",(short)*temp);
		temp++;
	}

	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads length number of 32 bit integers
void read_thirtytwobit_i(FILE * fp, uint8_t length)
{
	//uint32_t * numbers = malloc(sizeof(uint32_t) * length); //allocate space
	uint32_t * numbers = malloc(length); //allocate space
	uint32_t * temp = numbers; //get an iterating pointer
	int numberthings = length / 4;
	int result; 	//for storing number of successful things read by fread
	int i = 0;		//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint32_t),numberthings,fp)) != numberthings)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < numberthings;i++)
	{
		printf("32 bit number %d\n",*temp);
		temp++;
	}
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads in length number of 32 bit floating point numbers
void read_thirtytwobit_f(FILE * fp, uint8_t length)
{
	//uint32_t * numbers = malloc(sizeof(uint32_t) * length);
	uint32_t * numbers = malloc(length);
	uint32_t * temp = numbers;
	int numberthings = length / 4;

	int result; 	//for storing number of successful things read by fread
	int i = 0;		//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint32_t),numberthings,fp)) != numberthings)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < numberthings;i++)
	{
		printf("32 bit number %f\n",(float)*temp);
		temp++;
	}

	//printf("Bytes: %d\n", numbytes);
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads in length number of 64 bit floating point numbers
void read_sixtyfourbit_f(FILE * fp, uint8_t length)
{
	//uint64_t * numbers = malloc(sizeof(uint64_t) * length);
	uint64_t * numbers = malloc(length);
	uint64_t * temp = numbers;
	int numberthings = (int)length / 8;

	int result; 	//for storing number of successful things read by fread
	int i = 0;		//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint64_t),numberthings,fp)) != numberthings)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < numberthings;i++)
	{
		printf("64 bit number %f\n",(double)*temp);
		temp++;
	}

	//printf("Bytes: %d\n", numbytes);
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads in length number of bytes and tries to print them, adds '\0' 
void read_ascii(FILE * fp, uint8_t length)
{
	//uint8_t * asciichars = malloc((sizeof(uint8_t) * length) +1); //one more for \0
	uint8_t * asciichars = malloc(length +1); //one more for \0
	uint8_t * temp = asciichars;	//iterator pointer
	int numbytes = (int)length;		//Debug
	int result;
	int i = 0;
	//make sure we have length number of bytes to read in the file
	if((result = fread(asciichars,sizeof(uint8_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(asciichars);
		return;
	}
	temp = temp + length;	//goto end of string
	*temp = '\0'; 			//add the nullbyte
	printf("Ascii chars: %s\n", asciichars);	//prints whole string
	//printf("Bytes: %d\n", numbytes);	//Debug
	free(asciichars);
}
///////////////////////////////////////////////////////////

//iterates through length number of bytes
void read_junk(FILE * fp, uint8_t length)
{
	int numbytes = (int)length;
	int i = 0;
	for(i; i < length; i++)
	{
		getc(fp); //grabs another char
	}
	printf("Ignored %d Bytes: \n", numbytes);
}
///////////////////////////////////////////////////////////

//FYARRR
void read_burn(FILE * fp, uint8_t length)
{
	int numbytes = (int)length;
	//FYYYARRR
	printf("Bytes: %d\n", numbytes);
}
///////////////////////////////////////////////////////////

//returns checksum value calculated from the dummy struct pointer
uint8_t checkchecksum(dummy * dp)
{
	uint8_t checksum = 0;
	checksum += dp->vt.version; //has version and type inside
	checksum += dp->length;
	checksum += dp->s & 0xFF; //takes just the first byte
	return 256 - checksum;
}
///////////////////////////////////////////////////////////






