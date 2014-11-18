#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/*
*	Authors:	Matt Henry, Katie Swanson, Jin Hui Xu, Richard Buck
*	Usage:		on linux gcc -o assignment5 assignment5.c
*				./assignment5 inputfile
*/

//TYPES
#define SIXTEEN_BIT_INT 0
#define THIRTY_TWO_BIT_INTEGER 1
#define THIRTY_TWO_BIT_FLOATING 2
#define SIXTY_FOUR_BIT_FLOATING 3
#define ASCII_CHAR 7
#define JUNK 8
#define SKIP 9
#define BURN 10
#define STOP 11
typedef struct header
{
	union versiontype 
	{
		uint8_t version;
		uint8_t type;
	}vt;
	uint8_t length;
	uint16_t s;
} header;

//TODO	rewrite/clean up make pretty the code
//function declarations
void read_sixteenbit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_f(FILE * fp, uint8_t length);
void read_sixtyfourbit_f(FILE * fp, uint8_t length);
void read_junk(FILE * fp, uint8_t length);
void read_burn(FILE * fp, uint8_t length);
void read_ascii(FILE * fp, uint8_t length);
int read_data(FILE * fp, uint8_t type, uint8_t length, uint8_t version);
uint8_t checkchecksum(header * dp);
uint32_t read_skipinstruction(FILE * fp);

//global for skipping N datagrams
uint32_t skipnumber = 0;

int main(int argc, char ** argv)
{
	argv++; // make it look at first argument
	FILE * fp = NULL;
	header * headbuff = malloc(4); // all versions have a header 4 bytes long

	if (headbuff==NULL) { printf("Memory error\n"); return -1;}
	fp = fopen(*argv, "rb");
	if (fp==NULL) { printf("Error opening file\n"); return -2; }

	size_t result;	//stores the number of succesfull items fread has read
	
	while((result = fread(headbuff, 4, 1, fp)) == 1)
	{
		//Read in the header
		uint8_t theversion = headbuff->vt.version & 0x0F; //gets lower 4 bits
		uint8_t thetype = (headbuff->vt.type & 0xF0) >> 4; 	//gets higher 4 bits
		uint8_t thelength = headbuff->length - 4;			//takes away the header length
		uint8_t skipbit = headbuff->s & 0x1;				//takes first bit
		
		if(skipnumber > 0)	//We are skipping N(skipnumber) datagrams
		{
			read_junk(fp, (int)thelength);	//skip the datagrams data section
			skipnumber--;	//decrement the count
			continue;		//Done time for next header
		}
		
		//checks the skip bit if true proccess the next thelength bytes as junk
		if(skipbit) //If skipbit is set
		{
			read_junk(fp, (int)thelength);	//Just eats thelength number chars
			continue;	//Goes to proccess next header
		}

		//version 1 datagram
		if(theversion == 1)
		{
			if(read_data(fp, thetype, thelength, theversion))//this is where the read happens returns 1 on stop command and burn command
			{ 
				break;	//if true we're done
			} 
			continue;
		} //end version 1
		
		//version 2 datagram
		if(theversion == 2)
		{
			uint8_t dupbit = headbuff->s & 0x2;	//look only at the dupbit
			uint8_t checksum = (headbuff->s & 0xFF00) >> 8;	//take only the checksum byte mask and then 8 bit shift to the right
			//if checksum is 0 we assume that its valid
			if((checksum != 0) && checksum != checkchecksum(headbuff))
			{
				read_junk(fp, (int)thelength);//ignore if checksum doesnt work
				continue; //proccess next datagram
			}

			if(dupbit)
			{
				fpos_t pos;
				fgetpos(fp, &pos);	//remember position
				if(read_data(fp, thetype, thelength, theversion)){ break;} //if true we're done
				fsetpos(fp, &pos);	//restore position
				if(read_data(fp, thetype, thelength, theversion)){ break;} //Read it again
				continue;
			}//end if dupbit

			if(read_data(fp, thetype, thelength, theversion)){ break;} //if true were done
			continue;
		} //end version 2

		//version 3 datagram
		if(theversion == 3)
		{
			uint8_t id = headbuff->s & 0xFE;	//take the 7 bit id field
			uint8_t checksum = (headbuff->s & 0xFF00) >> 8;	//take the checksum byte
			//if checksum is 0 we assume that its valid
			if((checksum != 0) && checksum != checkchecksum(headbuff))
			{
				read_junk(fp, (int)thelength);	//ignore data if checksums bad
				continue;  //proccess next checksum
			}
			
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
		case SIXTEEN_BIT_INT: // 16 bit int
		{
			read_sixteenbit_i(fp,length);
			break;
		}
		case THIRTY_TWO_BIT_INTEGER: // 32 bit int
		{
			read_thirtytwobit_i(fp, length);
			break;
		}
		case THIRTY_TWO_BIT_FLOATING: // 32 bit float
		{
			read_thirtytwobit_f(fp,length);
			break;
		}
		case SIXTY_FOUR_BIT_FLOATING: // 64 bit float
		{
			read_sixtyfourbit_f(fp,length);
			break;
		}
		case ASCII_CHAR: // ascii
		{
			read_ascii(fp,length);
			break;
		}
		case JUNK: // junk
		{
			read_junk(fp,length);
			break;
		}
		case SKIP: // skip
		{
			skipnumber += (int) read_skipinstruction(fp); //increase the skip number by specified amount
			break;
		}
		case BURN: // burn
		{
			read_burn(fp,length);
			return 4143; //We're on fire (also exits)
			break;
		}
		case STOP: // stop
		{
			return 1; //Were done exit program
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
	//print loop
	for(;i < numberthings;i++)
	{
		printf("%d",(short)*temp);
		temp++;
	}
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads length number of 32 bit integers
void read_thirtytwobit_i(FILE * fp, uint8_t length)
{
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
	//print loop
	for(;i < numberthings;i++)
	{
		printf("%d",*temp);
		temp++;
	}
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads in length number of 32 bit floating point numbers
void read_thirtytwobit_f(FILE * fp, uint8_t length)
{
	uint32_t * numbers = malloc(length);	//allocate space
	uint32_t * temp = numbers;				//iterator
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
	//print loop
	for(;i < numberthings;i++)
	{
		printf("%f",(float)*temp);
		temp++;
	}
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads in length number of 64 bit floating point numbers
void read_sixtyfourbit_f(FILE * fp, uint8_t length)
{
	uint64_t * numbers = malloc(length);	//allocate space
	uint64_t * temp = numbers;				//iterator
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
	//print loop
	for(;i < numberthings;i++)
	{
		printf("64 bit number %f\n",(double)*temp);
		printf("%f",(double)*temp);
		temp++;
	}
	free(numbers);
}
///////////////////////////////////////////////////////////

//Reads in length number of bytes and tries to print them, adds '\0' 
void read_ascii(FILE * fp, uint8_t length)
{
	uint8_t * asciichars = malloc(length + 1); //one more for \0
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
	temp = temp + length;	//goto end of string with pointer arithmatic
	*temp = '\0'; 			//add the nullbyte
	printf("%s", asciichars);	//prints whole string
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

//returns checksum value calculated from the header struct pointer
uint8_t checkchecksum(header * dp)
{
	uint8_t checksum = 0;
	checksum += dp->vt.version; //has version and type inside
	checksum += dp->length;
	checksum += dp->s & 0xFF; //takes just the first byte
	return 256 - checksum;
}
///////////////////////////////////////////////////////////

//Reads length number of 32 bit integers
uint32_t read_skipinstruction(FILE * fp)
{
	uint32_t * number = malloc(4); //allocate space 32 bits 4 bytes
	int result; 	//for storing number of successful things read by fread
	uint32_t value;
	//Didnt have enough file to read the number of items
	if((result = fread(number,sizeof(uint32_t),1,fp)) != 1)
	{
		printf("Invalid number of items ran out of file\n");
		free(number);
		return;
	}
	value = *number;
	free(number);
	return value;
}


