#include "Elf_Details.h"
#include <stdio.h>
#include <string.h>


// Purpose:	Print human-readable details about an ELF file
// Input:
//			elven_file - A Elf_Details struct that contains data about an ELF file
//			sectionsToPrint - Bitwise AND the "PRINT_*" macros into this variable
//				to control what the function actually prints.
//			stream - A stream to send the information to (e.g., stdout, A file)
// Output:	None
// Note:	This function will print the relevant data from elven_file into stream
//				based on the flags found in sectionsToPrint
void print_elf_details(struct Elf_Details* elven_file, unsigned int sectionsToPrint, FILE* stream)
{
	/* LOCAL VARIABLES */
	

	/* INPUT VALIDATION */
	if (!stream)
	{
		fprintf(stderr, "ERROR: FILE* stream was NULL!\n");
		return;
	}
	else if (!elven_file)
	{
		fprintf(stream, "ERROR: struct Elf_Details* elven_file was NULL!\n");
		return;
	}
	else if ((sectionsToPrint >> 6) > 0)
	{
		fprintf(stream, "ERROR: Invalid flags found in sectionsToPrint!\n");
		return;
	}

	fprintf(stream, "\n\n");

	/* ELF HEADER */
	if (sectionsToPrint & PRINT_ELF_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	{
		print_fancy_header("ELF HEADER");

		fprintf(stream, "\n\n");
	}

	/* PROGRAM HEADER */
	if (sectionsToPrint & PRINT_ELF_PRGRM_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Implement later
		fprintf(stream, "\n\n");
	}

	/* SECTION HEADER */
	if (sectionsToPrint & PRINT_ELF_SECTN_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Implement later
		fprintf(stream, "\n\n");
	}

	/* PROGRAM DATA */
	if (sectionsToPrint & PRINT_ELF_PRGRM_DATA || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Implement later
		fprintf(stream, "\n\n");
	}

	/* SECTION DATA */
	if (sectionsToPrint & PRINT_ELF_SECTN_DATA || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Implement later
		fprintf(stream, "\n\n");
	}

	return;
}


// Purpose:	Prints an uppercase title surrounded by delimiters
// Input:
//			stream - Stream to print the header to
//			title - Title to print
//			delimiter - Single character to create the box
// Output:	None
// Note:	Automatically sizes the box
void print_fancy_header(FILE* stream, char* title, unsigned char delimiter)
{
	int headerWidth = 0;  // Used to dynamically determine the width of the header
	int i = 0;  // Iterating variable

	if (!stream || !title || !delimiter)
	{
		fprintf(stderr, "ERROR: NULL/nul parameter received!\n");
		return;
	}

	// ### title ###
	headerWidth = 3 + 1 + strlen(title) + 1 + 3;

	// First line
	for (i = 0; i < headerWidth; i++)
	{
		fputc(delimiter, stream);
	}
	fputc(0xA, stream);

	// Second line
	fprintf("%c%c%c %s %c%c%c\n", delimiter, delimiter, delimiter, title, delimiter, delimiter, delimiter);

	// Third line
	for (i = 0; i < headerWidth; i++)
	{
		fputc(delimiter, stream);
	}
	fputc(0xA, stream);

	return;
}
