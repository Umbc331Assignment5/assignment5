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

static const struct typemasks
{
	uint8_t sixteen_bit_int;// = 0;
	uint8_t thirty_two_bit_integer;// = 1;
	uint8_t thirty_two_bit_floating;// = 2;
	uint8_t sixty_four_bit_floating;// = 3;
	uint8_t ascii_char;// = 7;
	uint8_t junk;// = 8;
	uint8_t skip;// = 9;
	uint8_t burn;// = 10;
	uint8_t stop;// = 11;
} typemask = { 0, 1, 2, 3, 7, 8, 9, 10, 11};

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
	result = fread(headbuff, 4, 1, fp);

	int theversion = headbuff->vt.version & 0x0F;
	int thetype = (headbuff->vt.type & 0xF0) >> 4;
	int thelength = headbuff->length;
	int skipbit = headbuff->s & 0x1;

	printf("We read version %d type %d length %d skipbit %d\n",
			theversion, thetype ,thelength ,skipbit);
	//while(c != EOF)
	//{
	//	c = getc(fp);
	//	printf("C: %c", c);
	//}
	free(headbuff);
	fclose(fp);

	return 0;
}

void read_header(FILE * fp)
{
	
}













