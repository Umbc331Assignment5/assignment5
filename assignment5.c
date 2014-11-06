#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void read_sixteenbit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_f(FILE * fp, uint8_t length);
void read_sixtyfourbit_f(FILE * fp, uint8_t length);
void read_junk(FILE * fp, uint8_t length);
void read_burn(FILE * fp, uint8_t length);
void read_skip(FILE * fp, uint8_t length);
void read_stop(FILE * fp, uint8_t length);
void read_ascii(FILE * fp, uint8_t length);
int read_data(FILE * fp, uint8_t type, uint8_t length);


typedef struct dummy
{

	union versiontype //instead of bit fields we should implement masks and shiffting
	{
		uint8_t version;
		uint8_t type;
	}vt;
	uint8_t length;
	uint16_t s;
} dummy;
/*
typedef struct datagram_v1
{
	union versiontype vt;
	uint8_t length;
	uint8_t s : 1;
	//uint32_t data;
} datagram_v1;

typedef struct datagram_v2
{
	union versiontype vt;
	uint8_t length;
	union skipdup {
		uint8_t s;
		uint8_t duplicate;
	} skipdup;
	uint8_t checksum;
	//uint32_t data;
} datagram_v2;

typedef struct datagram_v3
{
	union versiontype vt;
	uint8_t length;
	union skip_id {
		uint8_t s;
		uint8_t id;
	} sid;
	uint8_t checksum;
	//uint32_t data;
} datagram_v3;
*/

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
	argv++;
	char c = 0;
	FILE * fp = NULL;
	size_t result;
	//dummy d;
	//printf("Datagram3 size %lu\n", sizeof(datagram_v3)); // should be 4
	
	dummy * headbuff = malloc(4); // all versions have a header 4 bytes long
	if (headbuff==NULL) { printf("Memory error\n"); return -1;}
	fp = fopen(*argv, "rb");
	if (fp==NULL) { printf("Error opening file\n"); return -2; }

	int i;
	while((result = fread(headbuff, 4, 1, fp)) == 1)
	{
		uint8_t theversion = headbuff->vt.version & 0x0F; //gets lower 4 bits
		uint8_t thetype = (headbuff->vt.type & 0xF0) >> 4; 	//gets higher 4 bits
		uint8_t thelength = headbuff->length;
		uint8_t skipbit = headbuff->s & 0x1;				//takes first bit
		printf("We read version %d type %d length %d skipbit %d\n",
			(int)theversion, (int)thetype ,(int)thelength ,(int)skipbit);
		//checks the skip bit if true proccess the next thelength bytes as junk
		if(skipbit) // if skipbit is set
		{
			read_junk(fp,thelength);
			continue;
		}

		if(theversion == 1)
		{
			if(read_data(fp, thetype, thelength)){ break;} //if true were done
			continue;
		} //end version 1
		if(theversion == 2)
		{
			uint8_t dupbit = headbuff->s & 0x2;
			if(dupbit)
			{
				fpos_t pos;
				fgetpos(fp, &pos); //remember position
				if(read_data(fp, thetype, thelength)){ break;} //if true we're done
				fsetpos(fp, &pos);
				if(read_data(fp, thetype, thelength)){ break;} //Read it again
				continue;
			}

			if(read_data(fp, thetype, thelength)){ break;} //if true were done
			continue;
		} //end version 2
		if(theversion == 3)
		{
			if(read_data(fp, thetype, thelength)){ break;} //if true were done
			//TODO should care about checksum after he specifies what the algorithm is
			continue;
		} //end version 3
	}//end while

	free(headbuff);
	fclose(fp);

	return 0;
}

void read_header(FILE * fp)
{
	
}

int read_data(FILE * fp, uint8_t type, uint8_t length)
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


//Assumes length is the number of the items specified
void read_sixteenbit_i(FILE * fp, uint8_t length)
{
	uint16_t * numbers = malloc(sizeof(uint16_t) * length);
	uint16_t * temp = numbers;
	int numbytes = length * 2;
	int result;
	int i = 0;
	if((result = fread(numbers,sizeof(uint16_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}
	
	for(;i < length;i++)
	{
		printf("16 bit number %u\n",*temp);
		temp++;
	}
	printf("Bytes: %d\n", numbytes);
	free(numbers);
}

void read_thirtytwobit_i(FILE * fp, uint8_t length)
{
	uint32_t * numbers = malloc(sizeof(uint32_t) * length);
	uint32_t * temp = numbers;
	int numbytes = length * 4;
	int result;
	int i = 0;
	if((result = fread(numbers,sizeof(uint32_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(numbers);
		return;
	}

	for(;i < length;i++)
	{
		printf("32 bit number %d\n",*temp);
		temp++;
	}

	printf("Bytes: %d\n", numbytes);
	free(numbers);
}

void read_thirtytwobit_f(FILE * fp, uint8_t length)
{
	uint32_t * numbers = malloc(sizeof(uint32_t) * length);
	int numbytes = (int)length * 4;
	printf("Bytes: %d\n", numbytes);
	free(numbers);
}

void read_sixtyfourbit_f(FILE * fp, uint8_t length)
{
	uint64_t * numbers = malloc(sizeof(uint64_t) * length);
	int numbytes = (int)length * 8;
	printf("Bytes: %d\n", numbytes);
	free(numbers);
}

void read_ascii(FILE * fp, uint8_t length)
{
	uint8_t * asciichars = malloc((sizeof(uint8_t) * length) +1); //one more for \0
	uint8_t * temp = asciichars;	
	int numbytes = (int)length;
	int result;
	int i = 0;
	if((result = fread(asciichars,sizeof(uint8_t),length,fp)) != length)
	{
		printf("Invalid number of items ran out of file\n");
		free(asciichars);
		return;
	}
	temp = temp + length;
	*temp = '\0'; //add the nullbyte
	printf("Ascii chars: %s\n", asciichars);
	printf("Bytes: %d\n", numbytes);
	free(asciichars);
}

void read_junk(FILE * fp, uint8_t length)
{
	int numbytes = (int)length;
	int i = 0;
	for(i; i < length; i++)
	{
		getc(fp);
	}
	printf("Bytes: %d\n", numbytes);
}

void read_skip(FILE * fp, uint8_t length)
{
	int numbytes = (int)length;
	int i = 0;
	for(i; i < length; i++)
	{
		getc(fp);
	}
	printf("Bytes: %d\n", numbytes);
}

void read_burn(FILE * fp, uint8_t length)
{
	int numbytes = (int)length;
	//FYYYARRR
	printf("Bytes: %d\n", numbytes);
}

void read_stop(FILE * fp, uint8_t length)
{
	int numbytes = (int)length;
	printf("Bytes: %d\n", numbytes);
}






















