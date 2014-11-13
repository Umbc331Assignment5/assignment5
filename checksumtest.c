#include <stdio.h>
#include <stdint.h>


int main(int arc, char **argv)
{
	uint8_t typeversion = 2;
	uint8_t length = 5;
	uint8_t SkipDupIDbits = 2;
	uint8_t totsmcgoats = 256;
	totsmcgoats = totsmcgoats - typeversion - length - SkipDupIDbits;
	printf("should be proper checksum: %d\n",totsmcgoats);
	totsmcgoats = totsmcgoats + typeversion + length + SkipDupIDbits;
	printf("should be zero: %d\n",totsmcgoats);
	return 0;
}
