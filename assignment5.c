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

//TODO SKIP control is wrong
//TODO Skipbit is also wrong needs to skip the correct datasize 
//		Instead of the number of bytes
//TODO 	checksum needs to skip the right amount of bytes
//		based on length and type of data
//TODO	input file needs to be edited to have valid checksums and not valid

//TODO	Add your names
//TODO	rewrite/clean up make pretty the code
//TODO	Choose either bitfield struct way or masking way


void read_sixteenbit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_f(FILE * fp, uint8_t length);
void read_sixtyfourbit_f(FILE * fp, uint8_t length);
void read_junk(FILE * fp, uint8_t length);
void read_burn(FILE * fp, uint8_t length);
void read_skip(FILE * fp, uint8_t length);
void read_ascii(FILE * fp, uint8_t length);
int read_data(FILE * fp, uint8_t type, uint8_t length, uint8_t version);
uint8_t checkchecksum(dummy * dp);

typedef struct dummy2
{
	union versiontype2
	{
		uint8_t version :4;
		uint8_t type : 4;
	}vt;
	uint8_t length;
	union validate_stuff
	{
		struct v1
		{
			uint16_t s : 1;
		} v1;
		struct v2
		{
			uint8_t s : 1;
			uint8_t d : 1;
			uint8_t checksum;
		} v2;
		struct v3
		{
			uint8_t s : 1;
			uint8_t id : 7;
			uint8_t checksum;
		} v3;

	} vs;
	
} dummy2;


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
	//dummy2 * headbuff = malloc(4);

	if (headbuff==NULL) { printf("Memory error\n"); return -1;}
	fp = fopen(*argv, "rb");
	if (fp==NULL) { printf("Error opening file\n"); return -2; }

	size_t result;

	while((result = fread(headbuff, 4, 1, fp)) == 1)
	{
		//old way
		uint8_t theversion = headbuff->vt.version & 0x0F; //gets lower 4 bits
		uint8_t thetype = (headbuff->vt.type & 0xF0) >> 4; 	//gets higher 4 bits
		uint8_t thelength = headbuff->length - 4;			//takes away the header length
		uint8_t skipbit = headbuff->s & 0x1;				//takes first bit
		
		//maybe more readable way
		//uint8_t theversion = headbuff->vt.version;
		//uint8_t thetype = headbuff->vt.type;
		//uint8_t thelength = headbuff->length;
		//uint8_t skipbit = headbuff->vs.v1.s;
		//uint8_t dupbit = headbuff->vs.v2.d;
		//uint8_t id = headbuff->vs.v3.id;
		//uint8_t checksum = headbuff->vs.v3.checksum;	//shouldnt matter if its v2 or v3

		//checks the skip bit if true proccess the next thelength bytes as junk
		if(skipbit) // if skipbit is set
		{
			printf("\nWe read version %d type %d length %d skipbit %d\n",
					(int)theversion, (int)thetype, 
					(int)thelength, (int)skipbit);
			read_junk(fp, typetosize(thetype) * thelength);//hack to skip right number of bytes
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
				printf("Bad Checksum\n");
				read_junk(fp, typetosize(thetype) * (int)thelength);//hack to skip right number of bytes
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
			//TODO do proper things instead of 'continue'
			if(checksum != checkchecksum(headbuff))
			{ 
				printf("Bad Checksum\n");
				read_junk(fp, typetosize(thetype) * (int)thelength);//hack to skip right number of bytes
				continue;  
			}

			printf("\nWe read version: %d type: %d length: %d skipbit: %d id: %d checksum: %d\n",
					(int)theversion, (int)thetype,
					(int)thelength, (int)skipbit,
					(int)id, (int)checksum);

			if(read_data(fp, thetype, thelength, theversion)){ break;} //if true were done
			//TODO should care about checksum after he specifies what the algorithm is
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
			read_skip(fp,length);
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
	uint16_t * numbers = malloc(sizeof(uint16_t) * length); //allocate space
	//uint16_t * numbers = thenumbers;
	uint16_t * temp = numbers;				//pointer for iterating
	int numbytes = length * 2;		//debugging
	int result;	//for storing number of successful things read by fread
	int i = 0;	//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint16_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < length;i++)
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
	uint32_t * numbers = malloc(sizeof(uint32_t) * length); //allocate space
	uint32_t * temp = numbers; //get an iterating pointer
	int numbytes = length * 4; //debuging
	int result; 	//for storing number of successful things read by fread
	int i = 0;		//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint32_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < length;i++)
	{
		printf("32 bit number %d\n",*temp);
		temp++;
	}

	//printf("Bytes: %d\n", numbytes);
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads in length number of 32 bit floating point numbers
void read_thirtytwobit_f(FILE * fp, uint8_t length)
{
	uint32_t * numbers = malloc(sizeof(uint32_t) * length);
	uint32_t * temp = numbers;
	int numbytes = (int)length * 4;

	int result; 	//for storing number of successful things read by fread
	int i = 0;		//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint32_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < length;i++)
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
	uint64_t * numbers = malloc(sizeof(uint64_t) * length);
	uint64_t * temp = numbers;
	int numbytes = (int)length * 8;

	int result; 	//for storing number of successful things read by fread
	int i = 0;		//for our printing loop below

	//Didnt have enough file to read the number of items
	if((result = fread(numbers,sizeof(uint64_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	//print what we read in TODO:double check this is correct representation
	for(;i < length;i++)
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
	uint8_t * asciichars = malloc((sizeof(uint8_t) * length) +1); //one more for \0
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
//TODO THIS IS WRONG
//TODO Read N number of datagrams each has its own variable length dictated 
//		by the respective header
//Different from skip bit, skips length number of 32 bit integers
void read_skip(FILE * fp, uint8_t length)
{
	int numbytes = (int)length * 4; //number of 32 bit integers to skip
	int i = 0;
	for(i; i < numbytes; i++)
	{
		getc(fp);
	}
	printf("Skipped %d 32 bit ints:\n", length);
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

//Takes in the type returns the factor length needs to multiplied by
//for the correct number of bytes
int typetosize(int type)
{
	switch(type)
	{
		case 0:
		{
			return 2;
		}
		case 1:
		{
			return 4;
		}
		case 2:
		{
			return 4;
		}
		case 3:
		{
			return 8;
		}
		case 4:
		{
			return 1;
		}
		default:
		{
			return 1;
		}
	}//end switch
}//end typetosize




