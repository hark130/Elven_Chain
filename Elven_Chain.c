#include "Elf_Details.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NULL
#define NULL ((void*)0)
#endif // NULL


size_t file_len(FILE* openFile);
size_t print_it(char* buff, size_t size);


int main(int argc, char *argv[])
{
	/* 1. LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;
	// FILE* elfFile = NULL;
	// size_t elfSize = 0;
	// char* elfGuts = NULL;
	// char* tmpPtr = NULL;
	struct Elf_Details* elvenCharSheet = NULL;
	struct Prgrm_Hdr_Details* programHeaderDetails = NULL;

	/* 2. INPUT VALIDATTION */
	if (argc != 2)
	{
		printf("Invalid number of arguments: %d\n", argc);
		return ERROR_BAD_ARG;
	}
	else if (argv[1] == NULL)
	{
		return ERROR_NULL_PTR;
	}
	else if (strlen(argv[1]) == 0)
	{
		return ERROR_BAD_ARG;
	}

	/* 3. READ ELF FILE */
	retVal = read_elf_file(argv[1], &elvenCharSheet, &programHeaderDetails);
	if (retVal != ERROR_SUCCESS)
	{
		PERROR(errno);
		return retVal;
	}
	else
	{
		// puts("ELVEN CHAIN: Completed read_elf_file()");  // DEBUGGING
	}

	/* 4. PRINT ELF HEADER DETAILS */
	// puts("ELVEN CHAIN: Made it to print_elf_details()");  // DEBUGGING
	print_elf_details(elvenCharSheet, PRINT_EVERYTHING, stdout);

	/* 5. PRINT PROGRAM HEADER DETAILS */
	// puts("ELVEN CHAIN: Made it to print_program_header()");  // DEBUGGING
	print_program_header(programHeaderDetails, PRINT_EVERYTHING, stdout);  // sectionsToPrint not yet implemented

	/* 6. CLEAN UP */
	// FREE Elf_Details STRUCT
	// puts("ELVEN CHAIN: Made it to kill_elf()");  // DEBUGGING
	retVal = kill_elf(&elvenCharSheet);
	// FREE Prgrm_Hdr_Details STRUCT
	// puts("ELVEN CHAIN: Made it to kill_program_header()");  // DEBUGGING
	retVal += kill_program_header(&programHeaderDetails);

	return retVal;
}
