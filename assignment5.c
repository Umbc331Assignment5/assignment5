#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct datagram_v1
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint16_t length;
	uint16_t s : 1;
	//uint32_t data;
} datagram_v1;

typedef struct datagram_v2
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint16_t length;
	uint8_t s : 1;
	uint8_t duplicate : 1;
	uint16_t checksum;
	//uint32_t data;
} datagram_v2;

typedef struct datagram_v3
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint16_t length;
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
	char * headbuff = malloc(4); // all versions have a header 4 bytes long
	if (headbuff==NULL) { printf("Memory error\n"); return -1;}
	fp = fopen(*argv, "rb");
	if (fp==NULL) { printf("Error opening file\n"); return -2; }
	
	while(c != EOF)
	{
		c = getc(fp);
		printf("C: %c", c);
	}
	free(headbuff);
	fclose(fp);
	return 0;
}

void read_header(FILE * fp)
{
	
}













