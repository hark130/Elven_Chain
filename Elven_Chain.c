#define ERROR_BAD_ARGS 	((int)-1)

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NULL
#define NULL ((void*)0)
#endif // NULL

#define DEBUGLEROAD

#ifndef DEBUGLEROAD
#ifndef PERROR
#define PERROR(errno) \
do { if (errno) { printf("Error Number:\t%d\nError Description:\t%s\n", errno, strerror(errno)); return errno; } } while(0);
#endif // PERROR
#else
#ifndef PERROR
#define PERROR(errno) ;
#endif // PERROR
#endif // DEBUGLEROAD


size_t file_len(FILE* openFile);
size_t print_it(char* buff, size_t size);

int main(int argc, char *argv[])
{
	/* 1. LOCAL VARIABLES */
	int retVal = 0;
	FILE* elfFile = NULL;
	size_t elfSize = 0;
	char* elfGuts = NULL;

	/* 2. INPUT VALIDATTION */
	if (argc != 2)
	{
		printf("Invalid number of arguments: %d\n", argc);
		return ERROR_BAD_ARGS;
	}

	/* READ ELF FILE */
	// OPEN FILE
	elfFile = fopen(argv[1], "rb");
	PERROR(errno);  // DEBUGGING

	if (elfFile)
	{
		// GET FILE SIZE
		elfSize = file_len(elfFile);

		// ALLOCATE BUFFER
		elfGuts = (char*)calloc(elfSize + 1, sizeof(char));
		PERROR(errno);  // DEBUGGING

		if (elfGuts)
		{
			// READ FILE
			if (fread(elfGuts, sizeof(char), elfSize, elfFile) != elfSize)
			{
				PERROR(errno);  // DEBUGGING
			}
			else
			{
				print_it(elfGuts, elfSize);  // DEBUGGING
			}			
		}
	}

	/* PARSE BUFFER */
	if (elfGuts)
	{

	}

	/* CLEAN UP */
	// CLOSE FILE
	if (elfFile)
	{
		retVal = fclose(elfFile);
		PERROR(errno);  // DEBUGGING
	}
	// FREE MEMORY
	if (elfGuts)
	{
		memset(elfGuts, 0, elfSize);
		PERROR(errno);  // DEBUGGING
		free(elfGuts);
		PERROR(errno);  // DEBUGGING
		elfGuts = NULL;
	}

	return retVal;
}


// Purpose:	Determine the exact length of a file
// Input:	Open FILE pointer
// Output:	Exact length of file in bytes
size_t file_len(FILE* openFile)
{
	size_t retVal = 0;
	char oneLetter = 'H';

	if (openFile)
	{
		while (1)
		{
			oneLetter = fgetc(openFile);
			PERROR(errno);  // DEBUGGING

			if (oneLetter != EOF)
			{
				retVal++;
			}
			else
			{
				break;
			}
		}

		rewind(openFile);
	}
	else
	{
		retVal = -1;
	}

	return retVal;
}


// Purpose:	Print a buffer, regardless of nul characters
// Input:	
//			buff - non-nul terminated char array
//			size - number of characters in buff
// Output:	Number of characters printed
size_t print_it(char* buff, size_t size)
{
	size_t retVal = 0;

	puts("\n");
	if (buff)
	{
		for (; retVal < size; retVal++)
		{
			putchar((*(buff + retVal)));
		}
	}
	puts("\n");

	return retVal;
}