#include <stdio.h>
#include <string.h>

void GetString(char *__string, unsigned int input_size){
	
	fgets(__string, input_size, stdin);

	unsigned int size=strlen(__string);

	__string[size-1]='\0';
}