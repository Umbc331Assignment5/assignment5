#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
	uint8_t version : 4;
	uint8_t type : 4;
	uint8_t length;
	uint16_t s : 1;
	//uint32_t data;
} datagram_v1;

typedef struct datagram_v2
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint8_t length;
	uint8_t s : 1;
	uint8_t duplicate : 1;
	uint16_t checksum;
	//uint32_t data;
} datagram_v2;

typedef struct datagram_v3
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint8_t length;
	uint16_t s : 1;
	uint16_t id : 15;
	uint16_t checksum;
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
	printf("Dummy size %lu\n", sizeof(d)); // should be 4
	
	dummy * headbuff = malloc(4); // all versions have a header 4 bytes long
	if (headbuff==NULL) { printf("Memory error\n"); return -1;}
	fp = fopen(*argv, "rb");
	if (fp==NULL) { printf("Error opening file\n"); return -2; }

	int i;
	while((result = fread(headbuff, 4, 1, fp)) == 1)
	{
		int theversion = headbuff->vt.version & 0x0F;
		int thetype = (headbuff->vt.type & 0xF0) >> 4;
		int thelength = headbuff->length;
		int skipbit = headbuff->s & 0x1;
		printf("We read version %d type %d length %d skipbit %d\n",
			theversion, thetype ,thelength ,skipbit);
		
	}

	free(headbuff);
	fclose(fp);

	return 0;
}

void read_header(FILE * fp)
{
	
}

void version(uint8_t v)
{
	switch(v)
	{
		case 0: // 16 bit int
		{
			break;
		}
		case 1: // 32 bit int
		{
			break;
		}
		case 2: // 32 bit float
		{
			break;
		}
		case 3: // 64 bit float
		{
			break;
		}
		case 7: // ascii
		{
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
}

void read_thirtytwobit_i(FILE * fp, uint8_t length)
{
	int numbytes = length * 4;
}

void read_thirtytwobit_f(FILE * fp, uint8_t length)
{
	int numbytes = length * 4;
}

void read_sixtyfourbit_f(FILE * fp, uint8_t length)
{
	int numbytes = length * 8;
}

void read_ascii(FILE * fp, uint8_t length)
{

}






















