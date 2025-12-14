#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "utils.h"

int main(){

	uint8_t PORTB = 0b10100100;

	printf("0b" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(PORTB));
	
	return 0; 
}
