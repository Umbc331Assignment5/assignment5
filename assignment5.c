#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void read_sixteenbit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_i(FILE * fp, uint8_t length);
void read_thirtytwobit_f(FILE * fp, uint8_t length);
void read_sixtyfourbit_f(FILE * fp, uint8_t length);
void read_ascii(FILE * fp, uint8_t length);
void read_data(FILE * fp, uint8_t type, uint8_t length);

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
	dummy d;
	printf("Datagram3 size %lu\n", sizeof(datagram_v3)); // should be 4
	
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
		read_data(fp, thetype, thelength);
		
	}

	free(headbuff);
	fclose(fp);

	return 0;
}

void read_header(FILE * fp)
{
	
}

void read_data(FILE * fp, uint8_t type, uint8_t length)
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
			break;
		}
		case 9: // skip
		{
			break;
		}
		case 10: // burn
		{
			break;
		}
		case 11: // stop
		{
			break;
		}
		default:
		{
			break;
		}
	}


}


//Assumes length is the number of the items specified
void read_sixteenbit_i(FILE * fp, uint8_t length)
{
	int numbytes = length * 2;
	printf("Bytes: %d\n", numbytes);
}

void read_thirtytwobit_i(FILE * fp, uint8_t length)
{
	int numbytes = length * 4;
	printf("Bytes: %d\n", numbytes);
}

void read_thirtytwobit_f(FILE * fp, uint8_t length)
{
	int numbytes = length * 4;
	printf("Bytes: %d\n", numbytes);
}

void read_sixtyfourbit_f(FILE * fp, uint8_t length)
{
	int numbytes = length * 8;
	printf("Bytes: %d\n", numbytes);
}

void read_ascii(FILE * fp, uint8_t length)
{
	int numbytes = length;
	printf("Bytes: %d\n", numbytes);
}






















