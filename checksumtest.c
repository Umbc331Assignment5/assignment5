#include <stdio.h>
#include <stdint.h>


int main(int arc, char **argv)
{
	uint8_t billy = 23;
	uint8_t inverse = 256 - billy;
	uint8_t totsmcgoats = billy + inverse;
	printf("should be 0: %d\n",totsmcgoats);
	return 0;
}
