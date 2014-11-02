#include <stdio.h>
#include <stdint.h>

typedef struct datagram_v1
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint16_t length;
	uint16_t s : 1;
	uint32_t data;
	/*
	char version : 4;
	char type : 4;
	short length;
	short s : 1;
	int data;
	*/

} datagram_v1;

typedef struct datagram_v2
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint16_t length;
	uint8_t s : 1;
	uint8_t duplicate : 1;
	uint16_t checksum;
	uint32_t data;
} datagram_v2;

typedef struct datagram_v3
{
	uint8_t version : 4;
	uint8_t type : 4;
	uint16_t length;
	uint16_t s : 1;
	uint16_t id : 15;
	uint16_t checksum;
	uint32_t data;
} datagram_v3;

typedef struct typemasks
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
} typemask;

int main(int argc, char ** argv)
{
	argv++;
	//char * first = *argv;
	//struct Datagram d;
	//d.i = 7;
	//d.y = 6;
	datagram_v1 d;
	d.version = 3;
	printf("HELLO\n %lu\n", sizeof(d));
	int c;
	const char * fname = *argv;
	FILE * fp = NULL;
	fp = fopen(fname, 'r');
	if (fp == NULL)
	{
		printf("Error opening file\n");
	}
	else
	{
		do 
		{
			c = getc(fp);
		} while(c != EOF);
	}
	
	return 0;
}















