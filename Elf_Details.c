#include "Elf_Details.h"
#include "Harklehash.h"
#include <assert.h>
#include <inttypes.h>	// Print uint64_t variables
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define SUPER_STR_ME(str) #str
// #define EXTRA_STR_ME(str) SUPER_STR_ME(str)
// #define STR_ME(str) EXTRA_STR_ME(str)

/********************************************************/
/********************************************************/
/******************* ELF HEADER START *******************/
/********************************************************/
/********************************************************/

// Purpose: Open and parse an ELF file.  Allocate, configure and return Elf_Details pointer.
// Input:	Filename, relative or absolute, to an ELF file
// Output:	A dynamically allocated Elf_Details struct that contains information about elvenFilename
// Note:	
//			It is caller's responsibility to free the return value from this function (and all char* within)
//			This function does all the prep work.  The actual parsing work is done by parse_elf()
struct Elf_Details* read_elf(char* elvenFilename)
{
	/* LOCAL VARIABLES */
	struct Elf_Details* retVal = NULL;	// Struct to be allocated, initialized and returned
	FILE* elfFile = NULL;				// File pointer of elvenFilename
	size_t elfSize = 0;					// Size of the file in bytes
	char* elfGuts = NULL;				// Holds contents of binary file
	char* tmpPtr = NULL;				// Holds return value from strstr()/strncpy()
	int tmpRetVal = 0;					// Holds return value from parse_elf()

	/* INPUT VALIDATION */
	if (!elvenFilename)
	{
		PERROR(errno);
		return retVal;
	}

	/* READ ELF FILE */
	// OPEN FILE
	elfFile = fopen(elvenFilename, "rb");

	if (elfFile)
	{
		// GET FILE SIZE
		elfSize = file_len(elfFile);
		PERROR(errno);  // DEBUGGING

		// ALLOCATE BUFFER
		elfGuts = (char*)gimme_mem(elfSize + 1, sizeof(char));
		PERROR(errno);  // DEBUGGING

		if (elfGuts)
		{
			// READ FILE
			if (fread(elfGuts, sizeof(char), elfSize, elfFile) != elfSize)
			{
				PERROR(errno);  // DEBUGGING
				take_mem_back((void**)&elfGuts, strlen(elfGuts), sizeof(char));
				// memset(elfGuts, 0, elfSize);
				// free(elfGuts);
				// elfGuts = NULL;
				fclose(elfFile);
				elfFile = NULL;
				return retVal;
			}
			else
			{
#ifdef DEBUGLEROAD
				print_it(elfGuts, elfSize);  // DEBUGGING
#endif // DEBUGLEROAD
			}			
		}
	}
	else
	{
		PERROR(errno);  // DEBUGGING
		return retVal;
	}

	/* ALLOCATE STRUCT MEMORY */
	retVal = (struct Elf_Details*)gimme_mem(1, sizeof(struct Elf_Details));
	if (!retVal)
	{
		PERROR(errno);
		take_mem_back((void**)&elfGuts, elfSize, sizeof(char));
		// memset(elfGuts, 0, elfSize);
		// free(elfGuts);
		// elfGuts = NULL;
		fclose(elfFile);
		elfFile = NULL;
		return retVal;
	}
	else  // Set struct bigEndian member to something other than 0
	{
		// We don't want the program mistakenly thinking the architecture is little Endian
		//	by default
		if (ZEROIZE_VALUE != TRUE && ZEROIZE_VALUE != FALSE)
		{
			retVal->bigEndian = ZEROIZE_VALUE;
		}
		else
		{
			retVal->bigEndian = -1;
		}
	}

	/* CLOSE ELF FILE */
	if (elfFile)
	{
		fclose(elfFile);
		elfFile = NULL;
	}

	/* PARSE ELF GUTS INTO STRUCT */
	// Allocate Filename
	retVal->fileName = gimme_mem(strlen(elvenFilename) + 1, sizeof(char));
	// Copy Filename
	if (retVal->fileName)
	{
		tmpPtr = strncpy(retVal->fileName, elvenFilename, strlen(elvenFilename));
		if(tmpPtr != retVal->fileName)
		{
			PERROR(errno);
			fprintf(stderr, "ERROR: strncpy of filename into Elf_Details struct failed!\n");
		}
		else
		{
#ifdef DEBUGLEROAD
			puts(retVal->fileName);  // DEBUGGING
#endif // DEBUGLEROAD
		}
	}
	// Initialize Remaining Struct Members
	tmpRetVal = parse_elf(retVal, elfGuts);
	if (tmpRetVal != ERROR_SUCCESS)
	{
		fprintf(stderr, "ERROR: parse_elf() encountered error number %d\n", tmpRetVal);
	}

	/* FINAL CLEAN UP */
	if (elfGuts)
	{
		take_mem_back((void**)&elfGuts, elfSize + 1, sizeof(char));
	}

	return retVal;
}


// Purpse:	Parse an ELF file contents into an Elf_Details struct
// Input:
//			elven_struct - Struct to store elven details
//			elven_contents - ELF file contents
// Output:	ERROR_* as specified in Elf_Details.h
int parse_elf(struct Elf_Details* elven_struct, char* elven_contents)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;	// parse_elf() return value
	char* tmpPtr = NULL;		// Holds return values from string functions
	int tmpInt = 0;				// Holds various temporary return values
	unsigned int tmpUint = 0;	// Holds calculated values from convert_char_to_int()
	uint32_t tmpUint32 = 0;		// Holds memory addresses on a 32-bit system
	uint64_t tmpUint64 = 0;		// Holds memory addresses on a 64-bit system
	int dataOffset = 0;			// Used to offset into elven_contents
	// char* tmpBuff = NULL;		// Temporary buffer used to assist in slicing up elven_contents
	struct HarkleDict* elfHdrClassDict = NULL;
	struct HarkleDict* elfHdrEndianDict = NULL;
	struct HarkleDict* elfHdrTargetOSDict = NULL;
	struct HarkleDict* elfHdrElfTypeDict = NULL;
	struct HarkleDict* elfHdrISADict = NULL;
	struct HarkleDict* elfHdrObjVerDict = NULL;
	struct HarkleDict* tmpNode = NULL;	// Holds return values from lookup_* functions

	/* INPUT VALIDATION */
	if (!elven_struct || !elven_contents)
	{
		retVal = ERROR_NULL_PTR;
		return retVal;
	}
	else if (strlen(elven_contents) == 0)
	{
		retVal = ERROR_ORC_FILE;
		return retVal;
	}

	/* PARSE ELF FILE CONTENTS */
	// 1. Find the beginning of the ELF Header (OFFSET: 0x00)
	tmpPtr = strstr(elven_contents, ELF_H_MAGIC_NUM);
	// printf("elven_contents:\t%p\nMagic Num:\t%p\n", elven_contents, tmpPtr);  // DEBUGGING
	if (tmpPtr != elven_contents)
	{
		PERROR(errno);
		// fprintf(stderr, "This is not an ELF formatted file.\nStart:\t%p\n%s:\t%p\n", elven_contents, ELF_H_MAGIC_NUM, tmpPtr);
		retVal = ERROR_ORC_FILE;
		return retVal;
	}
	else
	{
		elven_struct->magicNum = gimme_mem(strlen(ELF_H_MAGIC_NUM), sizeof(char));
		if (elven_struct->magicNum)
		{
			if (strncpy(elven_struct->magicNum, elven_contents, 4) != elven_struct->magicNum)
			{
				fprintf(stderr, "Error copying into Elf Struct magicNum!\n");
			}
		}
		else
		{
			fprintf(stderr, "Error allocating memory for Elf Struct magicNum!\n");
		}
	}

	/* PREPARE DYNAMICALLY ALLOCATED VARIABLES */
	// Peformed here to avoid memory leak if elven_contents turns out to be an ORC File
	// tmpBuff = gimme_mem(strlen(elven_contents) + 1, sizeof(char));
	elfHdrClassDict = init_elf_header_class_dict();
	elfHdrEndianDict = init_elf_header_endian_dict();
	elfHdrTargetOSDict = init_elf_header_targetOS_dict();
	elfHdrElfTypeDict = init_elf_header_elf_type_dict();
	elfHdrISADict = init_elf_header_isa_dict();
	elfHdrObjVerDict = init_elf_header_obj_version_dict();

	// 2. Begin initializing the struct

	// 2.1. Filename should already be initialized in calling function

	// 2.2. ELF Class (OFFSET: 0x04)
	dataOffset = 4;
	// fprintf(stdout, "ELF Class:\t%d\n", (*(elven_contents + dataOffset)));  // DEBUGGING
	// Unecessary?
	// strncpy(tmpBuff, elven_contents + dataOffset, 1);  // Copy one byte from the data offset to tmpBuff
	// fprintf(stdout, "tmpBuff now holds:\t%c (%d)\n", *tmpBuff, *tmpBuff);  // DEBUGGING
	// tmpInt = atoi(tmpBuff);  // THIS DOESN'T WORK!
	// tmpInt = (int)tmpBuff[0];  // Better way to do this?
	tmpInt = (int)(*(elven_contents + dataOffset));
	// fprintf(stdout, "tmpInt now holds:\t%d\n", tmpInt);  // DEBUGGING
	tmpNode = lookup_value(elfHdrClassDict, tmpInt);
	if (tmpNode)  // Found it
	{
		elven_struct->processorType = tmpInt;	// Set the processor type
		// fprintf(stdout, "tmpNode->name:\t%s\n", tmpNode->name);  // DEBUGGING
		elven_struct->elfClass = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
		if (elven_struct->elfClass)
		{
			if (strncpy(elven_struct->elfClass, tmpNode->name, strlen(tmpNode->name)) != elven_struct->elfClass)
			{
				fprintf(stderr, "ELF Class string '%s' not copied into ELF Struct!\n", tmpNode->name);
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "Successfully copied '%s' into ELF Struct!\n", elven_struct->elfClass);
#endif // DEBUGLEROAD
			}
		}
		else
		{
			fprintf(stderr, "Error allocating memory for Elf Struct elfClass!\n");
		}
	}
	else
	{
		elven_struct->processorType = ELF_H_CLASS_NONE;
		fprintf(stderr, "ELF Class %d not found in HarkleDict!\n", tmpInt);
	}
	// Zeroize/Free/NULLify elfHdrClassDict
	if (elfHdrClassDict)
	{
		tmpInt = destroy_a_list(&elfHdrClassDict);
	}

	// 2.3. Endianness (OFFSET: 0x05)
	dataOffset += 1;  // 5
	tmpInt = (int)(*(elven_contents + dataOffset));
	// fprintf(stdout, "tmpInt now holds:\t%d\n", tmpInt);  // DEBUGGING
	tmpNode = lookup_value(elfHdrEndianDict, tmpInt);
	if (tmpNode)  // Found it
	{
		// Set endianness bool (bigEndian)
		if (tmpInt == 2)
		{
			elven_struct->bigEndian = TRUE;
		}
		else
		{
			elven_struct->bigEndian = FALSE;
		}
		// fprintf(stdout, "tmpNode->name:\t%s\n", tmpNode->name);  // DEBUGGING
		elven_struct->endianness = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
		if (elven_struct->endianness)
		{
			if (strncpy(elven_struct->endianness, tmpNode->name, strlen(tmpNode->name)) != elven_struct->endianness)
			{
				fprintf(stderr, "ELF Endianness string '%s' not copied into ELF Struct!\n", tmpNode->name);
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "Successfully copied '%s' into ELF Struct!\n", elven_struct->endianness);
#endif // DEBUGLEROAD
			}
		}
		else
		{
			fprintf(stderr, "Error allocating memory for Elf Struct Endianness!\n");
		}
	}
	else
	{
		fprintf(stderr, "ELF Endianness %d not found in HarkleDict!\n", tmpInt);
	}
	// Zeroize/Free/NULLify elfHdrEndianDict
	if (elfHdrEndianDict)
	{
		tmpInt = destroy_a_list(&elfHdrEndianDict);
	}

	// 2.4. ELF Version (OFFSET: 0x06)
	dataOffset += 1;  // 6
	elven_struct->elfVersion = (int)(*(elven_contents + dataOffset));
	// fprintf(stdout, "elven_struct->elfVersion now holds:\t%d\n", elven_struct->elfVersion);  // DEBUGGING

	// 2.5. Target OS (OFFSET: 0x07)
	dataOffset += 1;  // 7
	tmpInt = (int)(*(elven_contents + dataOffset));
	// fprintf(stdout, "tmpInt now holds:\t%d\n", tmpInt);  // DEBUGGING
	tmpNode = lookup_value(elfHdrTargetOSDict, tmpInt);
	if (tmpNode)  // Found it
	{
		// fprintf(stdout, "tmpNode->name:\t%s\n", tmpNode->name);  // DEBUGGING
		elven_struct->targetOS = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
		if (elven_struct->targetOS)
		{
			if (strncpy(elven_struct->targetOS, tmpNode->name, strlen(tmpNode->name)) != elven_struct->targetOS)
			{
				fprintf(stderr, "ELF Target OS string '%s' not copied into ELF Struct!\n", tmpNode->name);
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "Successfully copied '%s' into ELF Struct!\n", elven_struct->targetOS);
#endif // DEBUGLEROAD
			}
		}
		else
		{
			fprintf(stderr, "Error allocating memory for Elf Struct Target OS ABI!\n");
		}
	}
	else
	{
		fprintf(stderr, "ELF Target OS %d not found in HarkleDict!\n", tmpInt);
	}
	// Zeroize/Free/NULLify elfHdrTargetOSDict
	if (elfHdrTargetOSDict)
	{
		tmpInt = destroy_a_list(&elfHdrTargetOSDict);
	}

	// 2.6. ABI Version (OFFSET: 0x08) //////////////////////////////////
	dataOffset += 1;  // 8
	// elven_struct->ABIversion = (int)(*(elven_contents + dataOffset));  // APPEARS TO BE NON-FUNCTIONAL
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 1, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->ABIversion = tmpUint;
	}
	// fprintf(stdout, "elven_struct->ABIversion now holds:\t%d\n", elven_struct->ABIversion);  // DEBUGGING

	// 2.6. Pad (OFFSET: 0x09)
	// char* pad;			// Unused portion
	// Not dynamically sized.  Statically sized.  Also, not performing a lookup.  Merely storing
	//	whatever was found in the Pad.
	dataOffset += 1;  // 9
	elven_struct->pad = gimme_mem(0x7 + 0x1, sizeof(char));
	if (elven_struct->pad)
	{
		if (memcpy(elven_struct->pad, elven_contents + dataOffset, 7) != elven_struct->pad)
		{
			fprintf(stderr, "ELF Pad not mem copied into ELF Struct!\n");
		}
		else
		{
#ifdef DEBUGLEROAD
			fprintf(stdout, "Successfully mem copied Pad into ELF Struct!\n");
#endif // DEBUGLEROAD
		}
	}
	else
	{
		fprintf(stderr, "Error allocating memory for Elf Struct Pad!\n");
	}

	// 2.7. Type (OFFSET: 0x10)
	dataOffset += 7;  // 16
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	// fprintf(stdout, "tmpInt now holds:\t%d\n", tmpInt);  // DEBUGGING
	// fprintf(stdout, "tmpUint now holds:\t%u\n", tmpUint);  // DEBUGGING
	if (tmpInt != ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to convert to an unsigned int.  Error Code:\t%d\n", tmpInt);
		tmpNode = NULL;
	}
	else
	{
		tmpNode = lookup_value(elfHdrElfTypeDict, (int)tmpUint);
	}
	
	if (tmpNode)  // Found it
	{
		// fprintf(stdout, "tmpNode->name:\t%s\n", tmpNode->name);  // DEBUGGING
		elven_struct->type = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
		if (elven_struct->type)
		{
			if (strncpy(elven_struct->type, tmpNode->name, strlen(tmpNode->name)) != elven_struct->type)
			{
				fprintf(stderr, "ELF Type string '%s' not copied into ELF Struct!\n", tmpNode->name);
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "Successfully copied '%s' into ELF Struct!\n", elven_struct->type);
#endif // DEBUGLEROAD
			}
		}
		else
		{
			fprintf(stderr, "Error allocating memory for Elf Struct Type!\n");
		}
	}
	else
	{
		fprintf(stderr, "ELF Type %d not found in HarkleDict!\n", (int)tmpUint);
	}
	// Zeroize/Free/NULLify elfHdrElfTypeDict
	if (elfHdrElfTypeDict)
	{
		tmpInt = destroy_a_list(&elfHdrElfTypeDict);
	}

	// 2.8. Instruction Set Architecture (ISA) (OFFSET: 0x12)
	dataOffset += 2;  // 18
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	// fprintf(stdout, "tmpInt now holds:\t%d\n", tmpInt);  // DEBUGGING
	// fprintf(stdout, "tmpUint now holds:\t%u\n", tmpUint);  // DEBUGGING
	if (tmpInt != ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to convert to an unsigned int.  Error Code:\t%d\n", tmpInt);
		tmpNode = NULL;
	}
	else
	{
		tmpNode = lookup_value(elfHdrISADict, (int)tmpUint);
	}
	
	if (tmpNode)  // Found it
	{
		// fprintf(stdout, "tmpNode->name:\t%s\n", tmpNode->name);  // DEBUGGING
		elven_struct->ISA = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
		if (elven_struct->ISA)
		{
			if (strncpy(elven_struct->ISA, tmpNode->name, strlen(tmpNode->name)) != elven_struct->ISA)
			{
				fprintf(stderr, "ELF ISA string '%s' not copied into ELF Struct!\n", tmpNode->name);
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "Successfully copied '%s' into ELF Struct!\n", elven_struct->ISA);
#endif // DEBUGLEROAD
			}
		}
		else
		{
			fprintf(stderr, "Error allocating memory for Elf Struct ISA!\n");
		}
	}
	else
	{
		fprintf(stderr, "ELF ISA %d not found in HarkleDict!\n", (int)tmpUint);
	}
	// Zeroize/Free/NULLify elfHdrISADict
	if (elfHdrISADict)
	{
		tmpInt = destroy_a_list(&elfHdrISADict);
	}

	// 2.9. Object File Version (OFFSET: 0x14)
	dataOffset += 2;  // 20
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 4, elven_struct->bigEndian, &tmpUint);
	// fprintf(stdout, "tmpInt now holds:\t%d\n", tmpInt);  // DEBUGGING
	// fprintf(stdout, "tmpUint now holds:\t%u\n", tmpUint);  // DEBUGGING
	if (tmpInt != ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to convert to an unsigned int.  Error Code:\t%d\n", tmpInt);
		tmpNode = NULL;
	}
	else
	{
		tmpNode = lookup_value(elfHdrObjVerDict, (int)tmpUint);
	}
	
	if (tmpNode)  // Found it
	{
		// fprintf(stdout, "tmpNode->name:\t%s\n", tmpNode->name);  // DEBUGGING
		elven_struct->objVersion = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
		if (elven_struct->objVersion)
		{
			if (strncpy(elven_struct->objVersion, tmpNode->name, strlen(tmpNode->name)) != elven_struct->objVersion)
			{
				fprintf(stderr, "ELF Object File Version string '%s' not copied into ELF Struct!\n", tmpNode->name);
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "Successfully copied '%s' into ELF Struct!\n", elven_struct->objVersion);
#endif // DEBUGLEROAD
			}
		}
		else
		{
			fprintf(stderr, "Error allocating memory for Elf Struct Object File Version!\n");
		}
	}
	else
	{
		fprintf(stderr, "ELF Object File Version %d not found in HarkleDict!\n", (int)tmpUint);
	}
	// Zeroize/Free/NULLify elfHdrObjVerDict
	if (elfHdrObjVerDict)
	{
		tmpInt = destroy_a_list(&elfHdrObjVerDict);
	}

	// 2.10 Entry Point (OFFSET: 0x18)
	dataOffset += 4;
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Entry Point section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	// 32-bit Processor
	if (elven_struct->processorType == ELF_H_CLASS_32)
	{		
		tmpInt = convert_char_to_uint64(elven_contents, dataOffset, 4, elven_struct->bigEndian, &tmpUint64);

		if (tmpInt != ERROR_SUCCESS)
		{
			fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
		}
		else
		{
			tmpInt = convert_uint64_to_uint32(tmpUint64, &tmpUint32);
			if (tmpInt != ERROR_SUCCESS)
			{
				fprintf(stderr, "Failed to convert uint64_t to a uint32_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
			}
			else
			{
				elven_struct->ePnt32 = tmpUint32;
			}
		}
	}
	// 64-bit Processor
	else if (elven_struct->processorType == ELF_H_CLASS_64)
	{
		tmpInt = convert_char_to_uint64(elven_contents, dataOffset, 8, elven_struct->bigEndian, &tmpUint64);

		if (tmpInt != ERROR_SUCCESS)
		{
			fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
		}
		else
		{
			elven_struct->ePnt64 = tmpUint64;
		}
	}
	// ??-bit Processor
	else
	{
		fprintf(stderr, "Struct Processor Type invalid so Entry Point not read!\n");
	}

	// 2.11. Program Header Table Offset
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Program Header Table Offset section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	// 32-bit Processor
	if (elven_struct->processorType == ELF_H_CLASS_32)
	{
		dataOffset += 4;
		tmpInt = convert_char_to_uint64(elven_contents, dataOffset, 4, elven_struct->bigEndian, &tmpUint64);

		if (tmpInt != ERROR_SUCCESS)
		{
			fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
		}
		else
		{
			tmpInt = convert_uint64_to_uint32(tmpUint64, &tmpUint32);
			if (tmpInt != ERROR_SUCCESS)
			{
				fprintf(stderr, "Failed to convert uint64_t to a uint32_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
			}
			else
			{
				// elven_struct->pHdr32 = dataOffset + tmpUint32;
				elven_struct->pHdr32 = tmpUint32;
			}
		}
	}
	// 64-bit Processor
	else if (elven_struct->processorType == ELF_H_CLASS_64)
	{
		dataOffset += 8;
		tmpInt = convert_char_to_uint64(elven_contents, dataOffset, 8, elven_struct->bigEndian, &tmpUint64);

		if (tmpInt != ERROR_SUCCESS)
		{
			fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
		}
		else
		{
			// elven_struct->pHdr64 = dataOffset + tmpUint64;
			elven_struct->pHdr64 = tmpUint64;
		}
	}
	// ??-bit Processor
	else
	{
		fprintf(stderr, "Struct Processor Type invalid so Program Header Offset not read!\n");
	}

	// 2.12. Section Header Table Offset
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Section Header Table Offset section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	// 32-bit Processor
	if (elven_struct->processorType == ELF_H_CLASS_32)
	{
		dataOffset += 4;
		tmpInt = convert_char_to_uint64(elven_contents, dataOffset, 4, elven_struct->bigEndian, &tmpUint64);

		if (tmpInt != ERROR_SUCCESS)
		{
			fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
		}
		else
		{
			tmpInt = convert_uint64_to_uint32(tmpUint64, &tmpUint32);
			if (tmpInt != ERROR_SUCCESS)
			{
				fprintf(stderr, "Failed to convert uint64_t to a uint32_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
			}
			else
			{
				elven_struct->sHdr32 = tmpUint32;
			}
		}
	}
	// 64-bit Processor
	else if (elven_struct->processorType == ELF_H_CLASS_64)
	{
		dataOffset += 8;
		tmpInt = convert_char_to_uint64(elven_contents, dataOffset, 8, elven_struct->bigEndian, &tmpUint64);

		if (tmpInt != ERROR_SUCCESS)
		{
			fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
		}
		else
		{
			elven_struct->sHdr64 = tmpUint64;
		}
	}
	// ??-bit Processor
	else
	{
		fprintf(stderr, "Struct Processor Type invalid so Section Header Offset not read!\n");
	}

	// 2.13. ELF Header Flags
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering ELF Header Flags section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	// 32-bit Processor
	if (elven_struct->processorType == ELF_H_CLASS_32)
	{
		dataOffset += 4;
	}
	// 64-bit Processor
	else if (elven_struct->processorType == ELF_H_CLASS_64)
	{
		dataOffset += 8;
	}
	// ??-bit Processor
	else
	{
		fprintf(stderr, "Struct Processor Type invalid so Flags not read!\n");
	}
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 4, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->flags = tmpUint;
	}

	// 2.14. ELF Header Size
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering ELF Header Size section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	dataOffset += 4;
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->elfHdrSize = tmpUint;
	}
	
	// 2.15. Program Header Size
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Program Header Size section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	dataOffset += 2;
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->prgmHdrSize = tmpUint;
	}

	// 2.16. Number of Program Header Entries
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Number of Program Header Entries section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	dataOffset += 2;
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->prgmHdrEntrNum = tmpUint;
	}

	// 2.17. Program Header Size
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Program Header Size section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	dataOffset += 2;
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->sectHdrSize = tmpUint;
	}

	// 2.18. Number of Section Header Entries
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Number of Section Header Entries section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	dataOffset += 2;
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->sectHdrEntrNum = tmpUint;
	}

	// 2.18. Section Header Index to Entry with Names
#ifdef DEBUGLEROAD
	fprintf(stdout, "Entering Section Header Index to Entry with Names section\n");  // DEBUGGING
#endif // DEBUGLEROAD
	dataOffset += 2;
	tmpInt = convert_char_to_int(elven_contents, dataOffset, 2, elven_struct->bigEndian, &tmpUint);
	if (tmpInt == ERROR_SUCCESS)
	{
		elven_struct->sectHdrSectNms = tmpUint;
	}

	/* CLEAN UP */
	// Zeroize/Free/NULLify tempBuff
	// if (tmpBuff)
	// {
	// 	take_mem_back((void**)&tmpBuff, strlen(elven_contents) + 1, sizeof(char));
	// }
#ifdef DEBUGLEROAD
	fprintf(stdout, "Exiting parse_elf()\n");  // DEBUGGING
#endif // DEBUGLEROAD

	return retVal;
}

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
	const char notConfigured[] = { "¡NOT CONFIGURED!"};	// Standard error output
	int i = 0;  										// Iterating variable

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
	else if (!elven_file->magicNum)
	{
		if (elven_file->fileName)
		{
			fprintf(stderr, "\n\n\tINVALID:  %s is not an ELF formatted file.\n\n\n", elven_file->fileName);
		}
		else
		{
			fprintf(stderr, "\n\n\tINVALID:  This is not an ELF formatted file.\n\n\n");
		}
		return;
	}
	else if (strncmp(elven_file->magicNum, ELF_H_MAGIC_NUM, 4))
	{
		if (elven_file->fileName)
		{
			fprintf(stderr, "\n\n\tINVALID:  %s is not an ELF formatted file.\n\n\n", elven_file->fileName);
		}
		else
		{
			fprintf(stderr, "\n\n\tINVALID:  This is not an ELF formatted file.\n\n\n");
		}
		return;
	}

	fprintf(stream, "\n\n");

	/* ELF HEADER */
	if (sectionsToPrint & PRINT_ELF_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Header
		print_fancy_header(stream, "ELF HEADER", HEADER_DELIM);

		// Filename
		if (elven_file->fileName)
		{
			fprintf(stream, "Filename:\t%s\n", elven_file->fileName);
		}
		else
		{
			fprintf(stream, "Filename:\t%s\n", notConfigured);	
		}

		// Class
		if (elven_file->elfClass)
		{
			fprintf(stream, "Class:\t\t%s\n", elven_file->elfClass);
		}
		else
		{
			fprintf(stream, "Class:\t\t%s\n", notConfigured);	
		}

		// Endianness
		if (elven_file->endianness)
		{
			fprintf(stream, "Endianness:\t%s\n", elven_file->endianness);
		}
		else if (elven_file->bigEndian == TRUE)
		{
			fprintf(stream, "Endianness:\t%s\n", "Big Endian");
		}
		else
		{
			fprintf(stream, "Endianness:\t%s\n", notConfigured);	
		}

		// ELF Version
		fprintf(stream, "ELF Version:\t%d\n", elven_file->elfVersion);

		// Target OS ABI
		if (elven_file->targetOS)
		{
			fprintf(stream, "Target OS ABI:\t%s\n", elven_file->targetOS);
		}
		else
		{
			fprintf(stream, "Target OS ABI:\t%s\n", notConfigured);	
		}

		// Version of the ABI
		fprintf(stream, "ABI Version:\t%d\n", elven_file->ABIversion);

		// Pad
		if (elven_file->pad)
		{
			fprintf(stream, "Pad:\t\t");
			for (i = 0; i < 7; i++)
			{
				fprintf(stream, "%c(%d) ", (*(elven_file->pad + i)), (*(elven_file->pad + i)));
			}
			fprintf(stream, "\n");
		}
		else
		{
			fprintf(stream, "Pad:\t%s\n", notConfigured);	
		}

		// Type of ELF File
		if (elven_file->type)
		{
			fprintf(stream, "ELF Type:\t%s\n", elven_file->type);
		}
		else
		{
			fprintf(stream, "ELF Type:\t%s\n", notConfigured);	
		}

		// Instruction Set Architecture (ISA)
		if (elven_file->ISA)
		{
			fprintf(stream, "Target ISA:\t%s\n", elven_file->ISA);
		}
		else
		{
			fprintf(stream, "Target ISA:\t%s\n", notConfigured);	
		}

		// Object File Version
		if (elven_file->objVersion)
		{
			fprintf(stream, "Object File:\t%s\n", elven_file->objVersion);
		}
		else
		{
			fprintf(stream, "Object File:\t%s\n", notConfigured);	
		}

		// Entry Point
		// 32-bit Processor
		if (elven_file->processorType == ELF_H_CLASS_32)
		{
			fprintf(stream, "Entry Point:\t0x%08" PRIx32 "\n", elven_file->ePnt32);
		}
		// 64-bit Processor
		else if (elven_file->processorType == ELF_H_CLASS_64)
		{
			fprintf(stream, "Entry Point:\t0x%016" PRIx64 "\n", elven_file->ePnt64);
		}
		// ??-bit Processor
		else
		{
			fprintf(stream, "Entry Point:\t%s\n", notConfigured);
		}

		// Program Header Offset
		// 32-bit Processor
		if (elven_file->processorType == ELF_H_CLASS_32)
		{
			fprintf(stream, "PH Offset:\t0x%" PRIx32 " (%" PRIu32 ")\n", elven_file->pHdr32, elven_file->pHdr32);
		}
		// 64-bit Processor
		else if (elven_file->processorType == ELF_H_CLASS_64)
		{
			fprintf(stream, "PH Offset:\t0x%" PRIx64 " (%" PRIu64 ")\n", elven_file->pHdr64, elven_file->pHdr64);
		}
		// ??-bit Processor
		else
		{
			fprintf(stream, "PH Offset:\t%s\n", notConfigured);
		}

		// Section Header Offset
		// 32-bit Processor
		if (elven_file->processorType == ELF_H_CLASS_32)
		{
			fprintf(stream, "SH Offset:\t0x%" PRIx32 "\n", elven_file->sHdr32);
		}
		// 64-bit Processor
		else if (elven_file->processorType == ELF_H_CLASS_64)
		{
			fprintf(stream, "SH Offset:\t0x%" PRIx64 "\n", elven_file->sHdr64);
		}
		// ??-bit Processor
		else
		{
			fprintf(stream, "SH Offset:\t%s\n", notConfigured);
		}

		// Flags
		if (elven_file->processorType == ELF_H_CLASS_32 || elven_file->processorType == ELF_H_CLASS_64)
		{
			fprintf(stream, "Flags:\t\t");
			// Binary printer
			// Printing flags so endianness shouldn't matter
			print_binary(stream, &(elven_file->flags), sizeof(elven_file->flags), TRUE);
			fprintf(stream, "\n");
		}
		// ??-bit Processor
		else
		{
			fprintf(stream, "Flags:\t\t%s\n", notConfigured);
		}

		// ELF Header Size
		fprintf(stream, "EHeader Size:\t%d\n", elven_file->elfHdrSize);
		
		// Program Header Size
		fprintf(stream, "PHeader Size:\t%d\n", elven_file->prgmHdrSize);

		// Number of Program Header Entries
		fprintf(stream, "# PH Entries:\t%d\n", elven_file->prgmHdrEntrNum);

		// Section Header Size
		fprintf(stream, "SHeader Size:\t%d\n", elven_file->sectHdrSize);

		// Number of Section Header Entries
		fprintf(stream, "# SH Entries:\t%d\n", elven_file->sectHdrEntrNum);

		// Section Header Index to the Entry with Names
		fprintf(stream, "SH Name Index:\t%d\n", elven_file->sectHdrSectNms);

		// Section delineation
		fprintf(stream, "\n\n");
	}

	// /* PROGRAM HEADER */
	// if (sectionsToPrint & PRINT_ELF_PRGRM_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	// {
	// 	// Header
	// 	print_fancy_header(stream, "PROGRAM HEADER", HEADER_DELIM);
	// 	// Implement later
	// 	fprintf(stream, "\n\n");
	// }

	// /* SECTION HEADER */
	// if (sectionsToPrint & PRINT_ELF_SECTN_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	// {
	// 	// Header
	// 	print_fancy_header(stream, "SECTION HEADER", HEADER_DELIM);
	// 	// Implement later
	// 	fprintf(stream, "\n\n");
	// }

	// /* PROGRAM DATA */
	// if (sectionsToPrint & PRINT_ELF_PRGRM_DATA || sectionsToPrint & PRINT_EVERYTHING)
	// {
	// 	// Header
	// 	print_fancy_header(stream, "PROGRAM DATA", HEADER_DELIM);
	// 	// Implement later
	// 	fprintf(stream, "\n\n");
	// }

	// /* SECTION DATA */
	// if (sectionsToPrint & PRINT_ELF_SECTN_DATA || sectionsToPrint & PRINT_EVERYTHING)
	// {
	// 	// Header
	// 	print_fancy_header(stream, "SECTION DATA", HEADER_DELIM);
	// 	// Implement later
	// 	fprintf(stream, "\n\n");
	// }

	return;
}


// Purpose:	Assist clean up efforts by zeroizing/free'ing an Elf_Details struct
// Input:	Pointer to an Elf_Details struct pointer
// Output:	ERROR_* as specified in Elf_Details.h
// Note:	This function will modify the original variable in the calling function
int kill_elf(struct Elf_Details** old_struct)
{
	int retVal = ERROR_SUCCESS;

	if (old_struct)
	{
		// fprintf(stdout, "old_struct:\t%p\n", old_struct);  // DEBUGGING
		if (*old_struct)
		{
			// fprintf(stdout, "*old_struct:\t%p\n", *old_struct);  // DEBUGGING
			/* ZEROIZE AND FREE (as appropriate) STRUCT MEMBERS */
			// char* fileName;		// Absolute or relative path
			if ((*old_struct)->fileName)
			{
				retVal += take_mem_back((void**)&((*old_struct)->fileName), strlen((*old_struct)->fileName), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->filename free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->filename.\n");
#endif // DEBUGLEROAD
				}
			}
			// 	char* magicNum;		// First four bytes of file
			if ((*old_struct)->magicNum)
			{
				retVal += take_mem_back((void**)&((*old_struct)->magicNum), strlen((*old_struct)->magicNum), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->magicNum free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->magicNum.\n");
#endif // DEBUGLEROAD
				}
			}
			// char* elfClass;		// 32 or 64 bit
			if ((*old_struct)->elfClass)
			{
				retVal += take_mem_back((void**)&((*old_struct)->elfClass), strlen((*old_struct)->elfClass), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->elfClass free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->elfClass.\n");
#endif // DEBUGLEROAD
				}
			}
			// int processorType;	// 32 or 64 bit
			(*old_struct)->processorType = ZEROIZE_VALUE;
			// char* endianness;	// Little or Big
			if ((*old_struct)->endianness)
			{
				retVal += take_mem_back((void**)&((*old_struct)->endianness), strlen((*old_struct)->endianness), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->endianness free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->endianness.\n");
#endif // DEBUGLEROAD
				}
			}
			// int bigEndian;		// If TRUE, bigEndian
			(*old_struct)->bigEndian = ZEROIZE_VALUE;
			// int elfVersion;		// ELF version
			(*old_struct)->elfVersion = ZEROIZE_VALUE;
			// char* targetOS;		// Target OS ABI
			if ((*old_struct)->targetOS)
			{
				retVal += take_mem_back((void**)&((*old_struct)->targetOS), strlen((*old_struct)->targetOS), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->targetOS free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->targetOS.\n");
#endif // DEBUGLEROAD
				}
			}
			// int ABIversion;		// Version of the ABI
			(*old_struct)->ABIversion = ZEROIZE_VALUE;
			// char* pad;			// Unused portion
			// NOTE: This char* member is statically sized based on ELF Header specifications
			if ((*old_struct)->pad)
			{
				retVal += take_mem_back((void**)&((*old_struct)->pad), 0x7, sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->pad free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->pad.\n");
#endif // DEBUGLEROAD
				}
			}
			// char* type;			// The type of ELF file
			if ((*old_struct)->type)
			{
				retVal += take_mem_back((void**)&((*old_struct)->type), strlen((*old_struct)->type), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->type free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->type.\n");
#endif // DEBUGLEROAD
				}
			}
			// char* ISA;			// Specifies target Instruction Set Architecture
			if ((*old_struct)->ISA)
			{
				retVal += take_mem_back((void**)&((*old_struct)->ISA), strlen((*old_struct)->ISA), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->ISA free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->ISA.\n");
#endif // DEBUGLEROAD
				}
			}
			// char* objVersion;	// Object File Version
			if ((*old_struct)->objVersion)
			{
				retVal += take_mem_back((void**)&((*old_struct)->objVersion), strlen((*old_struct)->objVersion), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->objVersion free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->objVersion.\n");
#endif // DEBUGLEROAD
				}
			}
			// uint32_t ePnt32;	// 32-bit memory address of the entry point from where the process starts executing
			(*old_struct)->ePnt32 = 0;
			(*old_struct)->ePnt32 |= ZEROIZE_VALUE;
			// uint64_t ePnt64;	// 64-bit memory address of the entry point from where the process starts executing
			(*old_struct)->ePnt64 = 0;
			(*old_struct)->ePnt64 |= ZEROIZE_VALUE;
			// uint32_t pHdr32;	// 32-bit address offset of the program header table
			(*old_struct)->pHdr32 = 0;
			(*old_struct)->pHdr32 |= ZEROIZE_VALUE;
			// uint32_t pHdr64;	// 64-bit address offset of the program header table
			(*old_struct)->pHdr64 = 0;
			(*old_struct)->pHdr64 |= ZEROIZE_VALUE;
			// uint32_t sHdr32;	// 32-bit address offset of the section header table
			(*old_struct)->sHdr32 = 0;
			(*old_struct)->sHdr32 |= ZEROIZE_VALUE;
			// uint64_t sHdr64;	// 64-bit address offset of the section header table
			(*old_struct)->sHdr64 = 0;
			(*old_struct)->sHdr64 |= ZEROIZE_VALUE;
			// unsigned int flags;	// Interpretation of this field depends on the target architecture
			(*old_struct)->flags = 0;
			(*old_struct)->flags |= ZEROIZE_VALUE;
			// int elfHdrSize;  // ELF Header Size
			(*old_struct)->elfHdrSize = 0;
			(*old_struct)->elfHdrSize |= ZEROIZE_VALUE;
			// int prgmHdrSize; // Contains the size of a program header table entry.
			(*old_struct)->prgmHdrSize = 0;
			(*old_struct)->prgmHdrSize |= ZEROIZE_VALUE;
			// int prgmHdrEntrNum;	// Number of entries in the program header table
			(*old_struct)->prgmHdrEntrNum = 0;
			(*old_struct)->prgmHdrEntrNum |= ZEROIZE_VALUE;
			// int sectHdrSize;	// Contains the size of a section header table entry.
			(*old_struct)->sectHdrSize = 0;
			(*old_struct)->sectHdrSize |= ZEROIZE_VALUE;
			// int sectHdrEntrNum;	// Number of entries in the section header table
			(*old_struct)->sectHdrEntrNum = 0;
			(*old_struct)->sectHdrEntrNum |= ZEROIZE_VALUE;
			// int sectHdrSectNms;	// Index of the section header table entry with section names
			(*old_struct)->sectHdrSectNms = 0;
			(*old_struct)->sectHdrSectNms |= ZEROIZE_VALUE;

			/* FREE THE STRUCT ITSELF */
			retVal += take_mem_back((void**)old_struct, 1, sizeof(struct Elf_Details));
			if (retVal)
			{
				PERROR(errno);
				fprintf(stderr, "take_mem_back() returned %d on struct free!\n", retVal);
				retVal = ERROR_SUCCESS;
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "take_mem_back() successfully freed struct.\n");
#endif // DEBUGLEROAD
			}
		}
		else
		{
			retVal = ERROR_NULL_PTR;
		}
	}
	else
	{
		retVal = ERROR_NULL_PTR;
	}

	return retVal;
}


// Purpose:	Build a HarkleDict of Elf Header Class definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_class_dict(void)
{
	/* LOCAL VARIABLES */
	struct HarkleDict* retVal = NULL;
	char* arrayOfNames[] = { "Invalid class", "32-bit format", "64-bit format" };
	// fprintf(stdout, "ELF_H_CLASS_32:\t%s\n", STR_ME(ELF_H_CLASS_32));  // DEBUGGING
	size_t numNames = sizeof(arrayOfNames)/sizeof(*arrayOfNames);
	int arrayOfValues[] = { ELF_H_CLASS_NONE, ELF_H_CLASS_32, ELF_H_CLASS_64 };
	size_t numValues = sizeof(arrayOfValues)/sizeof(*arrayOfValues);
	int i = 0;

	/* INPUT VALIDATION */
	// Verify the parallel arrays are the same length
	assert(numNames == numValues);

	for (i = 0; i < numNames; i++)
	{
		retVal = add_entry(retVal, (*(arrayOfNames + i)), (*(arrayOfValues + i)));
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				(*(arrayOfNames + i)), (*(arrayOfValues + i)));
			break;
		}
	}

	return retVal;
}


// Purpose:	Build a HarkleDict of Elf Header Data definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_endian_dict(void)
{
	/* LOCAL VARIABLES */
	struct HarkleDict* retVal = NULL;
	char* arrayOfNames[] = { "Invalid data encoding", "Little Endian", "Big Endian" };
	size_t numNames = sizeof(arrayOfNames)/sizeof(*arrayOfNames);
	int arrayOfValues[] = { ELF_H_DATA_NONE, ELF_H_DATA_LITTLE, ELF_H_DATA_BIG };
	size_t numValues = sizeof(arrayOfValues)/sizeof(*arrayOfValues);
	int i = 0;

	/* INPUT VALIDATION */
	// Verify the parallel arrays are the same length
	assert(numNames == numValues);

	for (i = 0; i < numNames; i++)
	{
		retVal = add_entry(retVal, (*(arrayOfNames + i)), (*(arrayOfValues + i)));
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				(*(arrayOfNames + i)), (*(arrayOfValues + i)));
			break;
		}
	}

	return retVal;
}


// Purpose:	Build a HarkleDict of Elf Header Target OS ABI definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_targetOS_dict(void)
{
	/* LOCAL VARIABLES */
	struct HarkleDict* retVal = NULL;
	char* arrayOfNames[] = { \
		"System V", "HP-UX", "NetBSD", \
		"Linux", "GNU Hurd", "Solaris", \
		"AIX", "IRIX", "FreeBSD", \
		"Tru64", "Novell Modesto", "OpenBSD", \
		"OpenVMS", "NonStop Kernel", "AROS", \
		"Fenix OS", "CloudABI", "Sortix", \
	};
	size_t numNames = sizeof(arrayOfNames)/sizeof(*arrayOfNames);
	int arrayOfValues[] = { \
		ELF_H_OSABI_SYSTEM_V, ELF_H_OSABI_HP_UX, ELF_H_OSABI_NETBSD,\
		ELF_H_OSABI_LINUX, ELF_H_OSABI_GNU_HURD, ELF_H_OSABI_SOLARIS,\
		ELF_H_OSABI_AIX, ELF_H_OSABI_IRIX, ELF_H_OSABI_FREE_BSD,\
		ELF_H_OSABI_TRU64, ELF_H_OSABI_NOVELL, ELF_H_OSABI_OPEN_BSD,\
		ELF_H_OSABI_OPEN_VMS, ELF_H_OSABI_NONSTOP_K, ELF_H_OSABI_AROS,\
		ELF_H_OSABI_FENIX_OS, ELF_H_OSABI_CLOUB_ABI, ELF_H_OSABI_SORTIX,\
	};
	size_t numValues = sizeof(arrayOfValues)/sizeof(*arrayOfValues);
	int i = 0;

	/* INPUT VALIDATION */
	// Verify the parallel arrays are the same length
	assert(numNames == numValues);

	for (i = 0; i < numNames; i++)
	{
		retVal = add_entry(retVal, (*(arrayOfNames + i)), (*(arrayOfValues + i)));
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				(*(arrayOfNames + i)), (*(arrayOfValues + i)));
			break;
		}
	}

	return retVal;
}


// Purpose:	Build a HarkleDict of Elf Header Type definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_elf_type_dict(void)
{
	/* LOCAL VARIABLES */
	struct HarkleDict* retVal = NULL;
	char* arrayOfNames[] = { \
		"No file type", "Relocatable file", \
		"Executable file", "Shared object file", \
		"Core file", \
	};
	size_t numNames = sizeof(arrayOfNames)/sizeof(*arrayOfNames);
	int arrayOfValues[] = { \
		ELF_H_TYPE_NONE, ELF_H_TYPE_RELOCATABLE, \
		ELF_H_TYPE_EXECUTABLE, ELF_H_TYPE_SHARED, \
		ELF_H_TYPE_CORE, \
	};
	size_t numValues = sizeof(arrayOfValues)/sizeof(*arrayOfValues);
	int i = 0;

	/* INPUT VALIDATION */
	// Verify the parallel arrays are the same length
	assert(numNames == numValues);

	for (i = 0; i < numNames; i++)
	{
		retVal = add_entry(retVal, (*(arrayOfNames + i)), (*(arrayOfValues + i)));
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				(*(arrayOfNames + i)), (*(arrayOfValues + i)));
			break;
		}
	}

	for (i = ELF_H_TYPE_LO_OS; i <= ELF_H_TYPE_HI_OS; i++)
	{
		retVal = add_entry(retVal, "Operating system-specific", i);
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				"Operating system-specific", i);
			break;
		}
	}

	for (i = ELF_H_TYPE_LO_PROC; i <= ELF_H_TYPE_HI_PROC; i++)
	{
		retVal = add_entry(retVal, "Processor-specific", i);
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				"Processor-specific", i);
			break;
		}
	}

	return retVal;
}


// Purpose:	Build a HarkleDict of Elf Header ISA definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_isa_dict(void)
{
	/* LOCAL VARIABLES */
	struct HarkleDict* retVal = NULL;
	// The following arrays may not be in numerical order but they are still
	//	parallel.
	// FUN FACT: The arrays were originally created with an old list of ISAs and later updated.
	char* arrayOfNames[] = { \
		"No machine", "AT&T WE 32100", "SPARC", \
		"Intel 80386", "Motorola 68000", "Motorola 88000", \
		"Intel 80860", "MIPS I Architecture", "IBM System/370 Processor", \
		"MIPS RS3000 Little-endian", "Hewlett-Packard PA-RISC", "Fujitsu VPP500", \
		"Enhanced instruction set SPARC", "Intel 80960", "PowerPC", \
		"64-bit PowerPC", "NEC V800", "Fujitsu FR20", \
		"TRW RH-32", "Motorola RCE", "Advanced RISC Machines ARM", \
		"Digital Alpha", "Hitachi SH", "SPARC Version 9", \
		"Siemens Tricore embedded processor", "Argonaut RISC Core, Argonaut Technologies Inc.", "Hitachi H8/300", \
		"Hitachi H8/300H", "Hitachi H8S", "Hitachi H8/500", \
		"Intel IA-64 processor architecture", "Stanford MIPS-X", "Motorola ColdFire", \
		"Motorola M68HC12", "Fujitsu MMA Multimedia Accelerator", "Siemens PCP", \
		"Sony nCPU embedded RISC processor", "Denso NDR1 microprocessor", "Motorola Star*Core processor", \
		"Toyota ME16 processor", "STMicroelectronics ST100 processor", "Advanced Logic Corp. TinyJ embedded processor family", \
		"Siemens FX66 microcontroller", "STMicroelectronics ST9+ 8/16 bit microcontroller", "STMicroelectronics ST7 8-bit microcontroller", \
		"Motorola MC68HC16 Microcontroller", "Motorola MC68HC11 Microcontroller", "Motorola MC68HC08 Microcontroller", \
		"Motorola MC68HC05 Microcontroller", "Silicon Graphics SVx", "STMicroelectronics ST19 8-bit microcontroller", \
		"Digital VAX", "Axis Communications 32-bit embedded processor", "Infineon Technologies 32-bit embedded processor", \
		"Element 14 64-bit DSP Processor", "LSI Logic 16-bit DSP Processor", "Donald Knuth's educational 64-bit processor", \
		"Harvard University machine-independent object files", "SiTera Prism", "Intel MCU", \
		"IBM System/390 Processor", "IBM SPU/SPC", "AMD x86-64 architecture", \
		"Sony DSP Processor", "Digital Equipment Corp. PDP-10", "Digital Equipment Corp. PDP-11", \
	};
	size_t numNames = sizeof(arrayOfNames)/sizeof(*arrayOfNames);
	int arrayOfValues[] = { \
		ELF_H_ISA_NONE, ELF_H_ISA_M32, ELF_H_ISA_SPARC, \
		ELF_H_ISA_386, ELF_H_ISA_68K, ELF_H_ISA_88K, \
		ELF_H_ISA_860, ELF_H_ISA_MIPS, ELF_H_ISA_S370, \
		ELF_H_ISA_MIPS_RS3_LE, ELF_H_ISA_PARISC, ELF_H_ISA_VPP500, \
		ELF_H_ISA_SPARC32PLUS, ELF_H_ISA_960, ELF_H_ISA_PPC, \
		ELF_H_ISA_PPC64, ELF_H_ISA_V800, ELF_H_ISA_FR20, \
		ELF_H_ISA_RH32, ELF_H_ISA_RCE, ELF_H_ISA_ARM, \
		ELF_H_ISA_ALPHA, ELF_H_ISA_SH, ELF_H_ISA_SPARCV9, \
		ELF_H_ISA_TRICORE, ELF_H_ISA_ARC, ELF_H_ISA_H8_300, \
		ELF_H_ISA_H8_300H, ELF_H_ISA_H8S, ELF_H_ISA_H8_500, \
		ELF_H_ISA_IA_64, ELF_H_ISA_MIPS_X, ELF_H_ISA_COLDFIRE, \
		ELF_H_ISA_68HC12, ELF_H_ISA_MMA, ELF_H_ISA_PCP, \
		ELF_H_ISA_NCPU, ELF_H_ISA_NDR1, ELF_H_ISA_STARCORE, \
		ELF_H_ISA_ME16, ELF_H_ISA_ST100, ELF_H_ISA_TINYJ, \
		ELF_H_ISA_FX66, ELF_H_ISA_ST9PLUS, ELF_H_ISA_ST7, \
		ELF_H_ISA_68HC16, ELF_H_ISA_68HC11, ELF_H_ISA_68HC08, \
		ELF_H_ISA_68HC05, ELF_H_ISA_SVX, ELF_H_ISA_ST19, \
		ELF_H_ISA_VAX, ELF_H_ISA_CRIS, ELF_H_ISA_JAVELIN, \
		ELF_H_ISA_FIREPATH, ELF_H_ISA_ZSP, ELF_H_ISA_MMIX, \
		ELF_H_ISA_HUANY, ELF_H_ISA_PRISM, ELF_H_ISA_IAMCU, \
		ELF_H_ISA_S390, ELF_H_ISA_SPU, ELF_H_ISA_X86_64, \
		ELF_H_ISA_PDSP, ELF_H_ISA_PDP10, ELF_H_ISA_PDP11, \
	};
	size_t numValues = sizeof(arrayOfValues)/sizeof(*arrayOfValues);
	int i = 0;

	/* INPUT VALIDATION */
	// Verify the parallel arrays are the same length
	assert(numNames == numValues);

	for (i = 0; i < numNames; i++)
	{
		retVal = add_entry(retVal, (*(arrayOfNames + i)), (*(arrayOfValues + i)));
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				(*(arrayOfNames + i)), (*(arrayOfValues + i)));
			break;
		}
	}

	// RESERVED ENTRIES
	// RESERVED 	11-14 	Reserved for future use
	for (i = 11; i <= 14; i++)
	{
		retVal = add_entry(retVal, "Reserved for future use", i);
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				"Reserved for future use", i);
			break;
		}
	}
	// RESERVED 	16 	Reserved for future use
	i = 16;  
	retVal = add_entry(retVal, "Reserved for future use", i);
	if (!retVal)
	{
		fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
			"Reserved for future use", i);
	}
	// RESERVED 	24-35 	Reserved for future use
	for (i = 24; i <= 35; i++)
	{
		retVal = add_entry(retVal, "Reserved for future use", i);
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				"Reserved for future use", i);
			break;
		}
	}

	return retVal;
}


// Purpose:	Build a HarkleDict of Elf Header Object File Version definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_obj_version_dict(void)
{
	/* LOCAL VARIABLES */
	struct HarkleDict* retVal = NULL;
	char* arrayOfNames[] = { "Invalid version", "Current version" };
	size_t numNames = sizeof(arrayOfNames)/sizeof(*arrayOfNames);
	int arrayOfValues[] = { ELF_H_OBJ_V_NONE, ELF_H_OBJ_V_CURRENT };
	size_t numValues = sizeof(arrayOfValues)/sizeof(*arrayOfValues);
	int i = 0;

	/* INPUT VALIDATION */
	// Verify the parallel arrays are the same length
	assert(numNames == numValues);

	for (i = 0; i < numNames; i++)
	{
		retVal = add_entry(retVal, (*(arrayOfNames + i)), (*(arrayOfValues + i)));
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				(*(arrayOfNames + i)), (*(arrayOfValues + i)));
			break;
		}
	}

	return retVal;
}


/********************************************************/
/********************************************************/
/******************* ELF HEADER STOP ********************/
/********************************************************/
/********************************************************/


/********************************************************/
/********************************************************/
/***************** PROGRAM HEADER START *****************/
/********************************************************/
/********************************************************/


// Purpose: Open and parse an ELF file.  Allocate, configure and return Prgrm_Hdr_Details pointer.
// Input:	
//			[in] elvenFilename - Filename, relative or absolute, to an ELF file
//			[in] elven_file - ELF Header struct previously read from program_contents
// Output:	A dynamically allocated Prgrm_Hdr_Details struct that contains information about elvenFilename
// Note:	It is caller's responsibility to free the return value from this function by calling
//				kill_elf()
struct Prgrm_Hdr_Details* read_program_header(char* elvenFilename, struct Elf_Details* elven_file)
{
    /* LOCAL VARIABLES */
    struct Prgrm_Hdr_Details* retVal = NULL;
	FILE* elfFile = NULL;				// File pointer of elvenFilename
	size_t elfSize = 0;					// Size of the file in bytes
	char* elfGuts = NULL;				// Holds contents of binary file
	char* tmpPtr = NULL;				// Holds return value from strstr()/strncpy()
	int tmpRetVal = 0;					// Holds return value from parse_program_header()... and others
	
    /* INPUT VALIDATION */
	if (!elvenFilename)
    {
		return retVal;	
    }
	else if (!elven_file)
	{
		return retVal;	
	}
	
	/* READ ELF FILE */
	// OPEN FILE
	elfFile = fopen(elvenFilename, "rb");

	if (elfFile)
	{
		// GET FILE SIZE
		elfSize = file_len(elfFile);
		PERROR(errno);  // DEBUGGING

		// ALLOCATE BUFFER
		elfGuts = (char*)gimme_mem(elfSize + 1, sizeof(char));
		PERROR(errno);  // DEBUGGING

		if (elfGuts)
		{
			// READ FILE
			if (fread(elfGuts, sizeof(char), elfSize, elfFile) != elfSize)
			{
				PERROR(errno);  // DEBUGGING
				take_mem_back((void**)&elfGuts, strlen(elfGuts), sizeof(char));
				fclose(elfFile);
				elfFile = NULL;
				return retVal;
			}
			else
			{
#ifdef DEBUGLEROAD
				print_it(elfGuts, elfSize);  // DEBUGGING
#endif // DEBUGLEROAD
			}			
		}
	}
	else
	{
		PERROR(errno);  // DEBUGGING
		return retVal;
	}

	/* ALLOCATE STRUCT MEMORY */
	retVal = (struct Prgrm_Hdr_Details*)gimme_mem(1, sizeof(struct Prgrm_Hdr_Details));
	if (!retVal)
	{
		PERROR(errno);
		take_mem_back((void**)&elfGuts, elfSize, sizeof(char));
		fclose(elfFile);
		elfFile = NULL;
		return retVal;
	}

	/* CLOSE ELF FILE */
	if (elfFile)
	{
		fclose(elfFile);
		elfFile = NULL;
	}

	/* COPY OVERLAP STRUCT MEMBERS FROM ELF_DETAILS INTO PRGRM_HDR_DETAILS */
	// Allocate Filename
	retVal->fileName = gimme_mem(strlen(elvenFilename) + 1, sizeof(char));
	// Copy Filename
	if (retVal->fileName)
	{
		tmpPtr = strncpy(retVal->fileName, elvenFilename, strlen(elvenFilename));
		if(tmpPtr != retVal->fileName)
		{
			PERROR(errno);
			fprintf(stderr, "ERROR: strncpy of filename into Prgrm_Hdr_Details struct failed!\n");
		}
		else
		{
#ifdef DEBUGLEROAD
			puts(retVal->fileName);  // DEBUGGING
#endif // DEBUGLEROAD
		}
	}

	// Allocate Elf Class
	if (elven_file->elfClass)
	{
		retVal->elfClass = gimme_mem(strlen(elven_file->elfClass) + 1, sizeof(char));
	}
	// Copy elfClass
	if (retVal->elfClass)
	{
		// Copy Elf Class
		tmpPtr = strncpy(retVal->elfClass, elven_file->elfClass, strlen(elven_file->elfClass));
		if(tmpPtr != retVal->elfClass)
		{
			PERROR(errno);
			fprintf(stderr, "ERROR: strncpy of elfClass into Prgrm_Hdr_Details struct failed!\n");
		}
		else
		{
#ifdef DEBUGLEROAD
			puts(retVal->elfClass);  // DEBUGGING
#endif // DEBUGLEROAD
		}
	}
	else
	{
		PERROR(errno);
		fprintf(stderr, "ERROR: Allocation of memory for Prgrm_Hdr_Details->elfClass failed!\n");
	}

	// Copy ELF Header processorType into the Program Header processorType
	retVal->processorType = elven_file->processorType;

	// Allocate Endianness
	if (elven_file->endianness)
	{
		retVal->endianness = gimme_mem(strlen(elven_file->endianness) + 1, sizeof(char));
	}
	// Copy endianness
	if (retVal->endianness)
	{
		// Copy Elf Class
		tmpPtr = strncpy(retVal->endianness, elven_file->endianness, strlen(elven_file->endianness));
		if(tmpPtr != retVal->endianness)
		{
			PERROR(errno);
			fprintf(stderr, "ERROR: strncpy of endianness into Prgrm_Hdr_Details struct failed!\n");
		}
		else
		{
#ifdef DEBUGLEROAD
			puts(retVal->endianness);  // DEBUGGING
#endif // DEBUGLEROAD
		}
	}
	else
	{
		PERROR(errno);
		fprintf(stderr, "ERROR: Allocation of memory for Prgrm_Hdr_Details->endianness failed!\n");
	}
#ifdef DEBUGLEROAD
	puts("Entering copy section");  // DEBUGGING
#endif // DEBUGLEROAD
	// Copy ELF Header bigEndian into the Program Header bigEndian
	retVal->bigEndian = elven_file->bigEndian;
	// Copy ELF Header pHdr32 into the Program Header pHdr32
	retVal->pHdr32 = elven_file->pHdr32;
	// Copy ELF Header pHdr64 into the Program Header pHdr64
	retVal->pHdr64 = elven_file->pHdr64;
	// Copy ELF Header prgmHdrSize into the Program Header prgmHdrSize
	retVal->prgmHdrSize = elven_file->prgmHdrSize;
	// Copy ELF Header prgmHdrEntrNum into the Program Header prgmHdrEntrNum
	retVal->prgmHdrEntrNum = elven_file->prgmHdrEntrNum;

	/* ALLOCATE PROGRAM HEADER SEGMENT ARRAY */
#ifdef DEBUGLEROAD
	puts("Entering allocate_segment_array()");  // DEBUGGING
#endif // DEBUGLEROAD
	tmpRetVal = allocate_segment_array(retVal);
	if (tmpRetVal != ERROR_SUCCESS)
	{
		fprintf(stderr, "ERROR: allocate_segment_array() returned error number %d\n", tmpRetVal);
	}

	/* PARSE PROGRAM HEADER INTO STRUCT */
	// Initialize Remaining Struct Members
#ifdef DEBUGLEROAD
	puts("Entering parse_program_header()");  // DEBUGGING
#endif // DEBUGLEROAD
	tmpRetVal = parse_program_header(retVal, elfGuts, elven_file);
	if (tmpRetVal != ERROR_SUCCESS)
	{
		fprintf(stderr, "ERROR: parse_program_header() returned error number %d\n", tmpRetVal);
	}

	/* FINAL CLEAN UP */
	if (elfGuts)
	{
		take_mem_back((void**)&elfGuts, elfSize + 1, sizeof(char));
	}

	return retVal;
}


// Purpse:	Parse an ELF file contents into an Elf_Details struct
// Input:
//			program_struct - Struct to store elven details regarding the program header
//			program_contents - ELF file contents
//			elven_file - ELF Header struct previously read from program_contents
// Output:	ERROR_* as specified in Elf_Details.h
int parse_program_header(struct Prgrm_Hdr_Details* program_struct, char* program_contents, struct Elf_Details* elven_file)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;	// parse_elf() return value
	char* tmpPtr = NULL;		// Holds return values from string functions
	int tmpInt = 0;				// Holds various temporary return values
	int i = 0;					// Iterating variable
	unsigned int tmpUint = 0;	// Holds calculated values from convert_char_to_int()
	uint32_t tmpUint32 = 0;		// Holds memory addresses on a 32-bit system
	uint64_t tmpUint64 = 0;		// Holds memory addresses on a 64-bit system
	int dataOffset = 0;			// Used to offset into elven_contents
	// char* tmpBuff = NULL;		// Temporary buffer used to assist in slicing up elven_contents
	struct HarkleDict* prgrmHdrTypeDict = NULL;
	struct HarkleDict* tmpNode = NULL;	// Holds return values from lookup_* functions
	int segmentCount = 0;		// Keeps track of how many program header segments have been read
	struct Prgrm_Hdr_Segment_32* segment32_ptr = NULL;
	struct Prgrm_Hdr_Segment_64* segment64_ptr = NULL;
	struct Prgrm_Hdr_Segment_32** segmentArray32_ptr = NULL;
	struct Prgrm_Hdr_Segment_64** segmentArray64_ptr = NULL;

	/* INPUT VALIDATION */
	// Check for NULL pointers
	if (!program_struct || !program_contents || !elven_file)
	{
		retVal = ERROR_NULL_PTR;
		return retVal;
	}
	// Verify file has been read
	else if (strlen(program_contents) == 0)
	{
		retVal = ERROR_ORC_FILE;
		return retVal;
	}
	// Verify processory type (e.g., 32-bit, 64-bit) has been configured already
	else if (program_struct->processorType != ELF_H_CLASS_32 && program_struct->processorType != ELF_H_CLASS_64)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	// Verify Endianness has been configured already
	else if (program_struct->bigEndian != TRUE && program_struct->bigEndian != FALSE)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	// Verify Program Header Offsets (and other necessary struct members)
    // IMPLEMENT THIS LATER
#ifdef DEBUGLEROAD
    puts("parse_program_header() copying Processor Type");  // DEBUGGING
#endif // DEBUGLEROAD
	if (program_struct->processorType == ELF_H_CLASS_32)
	{
		dataOffset = program_struct->pHdr32;
	}
	else if (program_struct->processorType == ELF_H_CLASS_64)
	{
		dataOffset = program_struct->pHdr64;
	}
	else
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	//// Verify program_struct->pHdr32 and program_struct->pHdr64 are non-0 as appropriate

	/* PARSE PROGRAM HEADER CONTENTS */
#ifdef DEBUGLEROAD
	puts("parse_program_header() made it past input validation");  // DEBUGGING
#endif // DEBUGLEROAD
	// 2. Begin initializing the struct
	// 2.1. fileName should already be initialized in calling function
	// 2.2. elfClass should already be initialized in calling function
	// 2.3. processorType should already be initialized in calling function
	// 2.4. endianness should already be initialized in calling function
	// 2.5. bigEndian should already be initialized in calling function
	// 2.6. pHdr32 should already be initialized by the calling function
	// 2.7. pHdr64 should already be initialized by the calling function
	// 2.8. prgmHdrSize
	// 2.9. prgmHdrEntrNum
	// 2.10. Program Header Segments
	// Prepare Harkledict of Program Header Types
	prgrmHdrTypeDict = init_program_header_type_dict();
#ifdef DEBUGLEROAD
	puts("parse_program_header() made init_program_header_type_dict()");  // DEBUGGING
#endif // DEBUGLEROAD

	// Iterate through the array of segment structs
	while (segmentCount < program_struct->prgmHdrEntrNum)
	{
		// 2.10.A. 32 bit
		if (program_struct->processorType == ELF_H_CLASS_32)
		{
			segmentArray32_ptr = (struct Prgrm_Hdr_Segment_32**)program_struct->segmentArray;
			if (segmentArray32_ptr)
			{
				for (i = 0; i < program_struct->prgmHdrEntrNum; i++)
				{
					segment32_ptr = (struct Prgrm_Hdr_Segment_32*)(*(segmentArray32_ptr + i));
					if (segment32_ptr)
					{
						// 2.10.A.1. uint32_t p_type;  // Segment type as number
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->p_type), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->p_type = tmpUint;
							// 2.10.A.2. char* prgmHdrType;  // Identifies the type of the segment
							if (prgrmHdrTypeDict)
							{
								tmpNode = lookup_value(prgrmHdrTypeDict, (int)tmpUint);
							}
							else
							{
								fprintf(stderr, "init_program_header_type_dict() failed to build a HarkleDict.\n");
								tmpNode = NULL;
							}

							// Allocate and copy human readable string into the struct
							if (tmpNode)  // Found it
							{
								segment32_ptr->prgmHdrType = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
								if (segment32_ptr->prgmHdrType)
								{
									if (strncpy(segment32_ptr->prgmHdrType, tmpNode->name, strlen(tmpNode->name)) != segment32_ptr->prgmHdrType)
									{
										fprintf(stderr, "Program Header Type string '%s' not copied into Program Header Struct!\n", tmpNode->name);
									}
									else
									{
#ifdef DEBUGLEROAD
										fprintf(stdout, "Successfully copied '%s' into Program Header Struct!\n", segment32_ptr->prgmHdrType);
#endif // DEBUGLEROAD
									}
								}
								else
								{
									fprintf(stderr, "Error allocating memory for Program Header Struct Type!\n");
								}
							}
							else
							{
								fprintf(stderr, "Program Header Type %d not found in HarkleDict!\n", (int)tmpUint);
							}
						}
						else 
						{
							fprintf(stderr, "Failed to convert to an unsigned int.  Error Code:\t%d\n", tmpInt);
							tmpNode = NULL;
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->p_type);
						
						// 2.10.A.3. uint32_t segOffset;  // Offset of the segment's first byte in the file image
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->segOffset), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->segOffset = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->segOffset);

						// 2.10.A.4. uint32_t segVirtualAddr;  // Virtual address of the segment in memory
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->segVirtualAddr), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->segVirtualAddr = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->segVirtualAddr);

						// 2.10.A.5. uint32_t segPhysicalAddr;  // Physical address of the segment in memory
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->segPhysicalAddr), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->segPhysicalAddr = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->segPhysicalAddr);

						// 2.10.A.6. uint32_t segFileSize;  // Size in bytes of the segment in the file image
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->segFileSize), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->segFileSize = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->segFileSize);

						// 2.10.A.7. uint32_t segMemSize;  // Size in bytes of the segment in memory
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->segMemSize), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->segMemSize = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->segMemSize);

						// 2.10.A.8. uint32_t flags;  // Segment flags
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->flags), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->flags = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->flags);

						// 2.10.A.9. uint32_t alignment;  // Alignment
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment32_ptr->alignment), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment32_ptr->alignment = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment32_ptr->alignment);
					}
					else
					{
						fprintf(stderr, "The segment array of the Program Header struct contains a NULL Pointer.\n");
					}
				}
			}
			else
			{
				fprintf(stderr, "Program Header struct does not have a segment array.\n");
			}
		}
		// 2.10.B. 64 bit
		else if (program_struct->processorType == ELF_H_CLASS_64)
		{
			segmentArray64_ptr = (struct Prgrm_Hdr_Segment_64**)program_struct->segmentArray;
			if (segmentArray64_ptr)
			{
				for (i = 0; i < program_struct->prgmHdrEntrNum; i++)
				{
					segment64_ptr = (struct Prgrm_Hdr_Segment_64*)(*(segmentArray64_ptr + i));
					if (segment64_ptr)
					{
						// 2.10.A.1. uint32_t p_type;  // Segment type as number
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->p_type), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->p_type = tmpUint;
							// 2.10.A.2. char* prgmHdrType;  // Identifies the type of the segment
							if (prgrmHdrTypeDict)
							{
								tmpNode = lookup_value(prgrmHdrTypeDict, (int)tmpUint);
							}
							else
							{
								fprintf(stderr, "init_program_header_type_dict() failed to build a HarkleDict.\n");
								tmpNode = NULL;
							}

							// Allocate and copy human readable string into the struct
							if (tmpNode)  // Found it
							{
								segment64_ptr->prgmHdrType = gimme_mem(strlen(tmpNode->name) + 1, sizeof(char));
								if (segment64_ptr->prgmHdrType)
								{
									if (strncpy(segment64_ptr->prgmHdrType, tmpNode->name, strlen(tmpNode->name)) != segment64_ptr->prgmHdrType)
									{
										fprintf(stderr, "Program Header Type string '%s' not copied into Program Header Struct!\n", tmpNode->name);
									}
									else
									{
#ifdef DEBUGLEROAD
										fprintf(stdout, "Successfully copied '%s' into Program Header Struct!\n", segment64_ptr->prgmHdrType);
#endif // DEBUGLEROAD
									}
								}
								else
								{
									fprintf(stderr, "Error allocating memory for Program Header Struct Type!\n");
								}
							}
							else
							{
								fprintf(stderr, "Program Header Type %d not found in HarkleDict!\n", (int)tmpUint);
							}
						}
						else 
						{
							fprintf(stderr, "Failed to convert to an unsigned int.  Error Code:\t%d\n", tmpInt);
							tmpNode = NULL;
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->p_type);
						
						// 2.10.A.3. uint32_t segOffset;  // Offset of the segment's first byte in the file image
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->segOffset), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->segOffset = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->segOffset);

						// 2.10.A.4. uint32_t segVirtualAddr;  // Virtual address of the segment in memory
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->segVirtualAddr), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->segVirtualAddr = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->segVirtualAddr);

						// 2.10.A.5. uint32_t segPhysicalAddr;  // Physical address of the segment in memory
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->segPhysicalAddr), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->segPhysicalAddr = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->segPhysicalAddr);

						// 2.10.A.6. uint32_t segFileSize;  // Size in bytes of the segment in the file image
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->segFileSize), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->segFileSize = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->segFileSize);

						// 2.10.A.7. uint32_t segMemSize;  // Size in bytes of the segment in memory
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->segMemSize), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->segMemSize = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->segMemSize);

						// 2.10.A.8. uint32_t flags;  // Segment flags
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->flags), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->flags = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->flags);
						
						// 2.10.A.9. uint32_t alignment;  // Alignment
						tmpInt = convert_char_to_int(program_contents, dataOffset, sizeof(segment64_ptr->alignment), program_struct->bigEndian, &tmpUint);
						if (tmpInt == ERROR_SUCCESS)
						{
							segment64_ptr->alignment = tmpUint;
						}
						else
						{
							fprintf(stderr, "Failed to convert char to an uint64_t.  Error Code:\t%d\n", tmpInt);  // DEBUGGING
						}
						// Advance to next struct member
						dataOffset += sizeof(segment64_ptr->alignment);
					}
					else
					{
						fprintf(stderr, "The segment array of the Program Header struct contains a NULL Pointer.\n");
					}
				}
			}
			else
			{
				fprintf(stderr, "Program Header struct does not have a segment array.\n");
			}
		}
		// How did we get here?!
		else
		{
			fprintf(stderr, "Program Header struct hasn't been configured with a processor type.\n");
		}

		// 2.10.C. Finished reading a program header segment
		segmentCount++;
	}

	// Zeroize/Free/NULLify prgrmHdrTypeDict
	if (prgrmHdrTypeDict)
	{
		tmpInt = destroy_a_list(&prgrmHdrTypeDict);
	}

	/* CLEAN UP */
	// Nothing to clean up... yet

	return retVal;
}


// Purpose:	Print human-readable details about an ELF file's Program Header
// Input:
//			program_struct - A Prgrm_Hdr_Details struct that contains data about an ELF file
//			sectionsToPrint - Bitwise AND the "PRINT_*" macros into this variable
//				to control what the function actually prints.
//			stream - A stream to send the information to (e.g., stdout, A file)
// Output:	None
// Note:	This function will print the relevant data from program_struct into stream
//				based on the flags found in sectionsToPrint
void print_program_header(struct Prgrm_Hdr_Details* program_struct, unsigned int sectionsToPrint, FILE* stream)
{
	/* LOCAL VARIABLES */
	const char notConfigured[] = { "¡NOT CONFIGURED!"};	// Standard error output
	int i = 0;  										// Iterating variable

	/* INPUT VALIDATION */
	if (!stream)
	{
		fprintf(stderr, "ERROR: FILE* stream was NULL!\n");
		return;
	}
	else if (!program_struct)
	{
		fprintf(stream, "ERROR: struct Prgrm_Hdr_Details* program_struct was NULL!\n");
		return;
	}
	else if ((sectionsToPrint >> 6) > 0)
	{
		fprintf(stream, "ERROR: Invalid flags found in sectionsToPrint!\n");
		return;
	}

	/* PROGRAM HEADER */
	if (sectionsToPrint & PRINT_ELF_PRGRM_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Header
		print_fancy_header(stream, "PROGRAM HEADER", HEADER_DELIM);
		// Implement later
		// fprintf(stream, "\n\n");

		// Filename
		if (program_struct->fileName)
		{
			fprintf(stream, "Filename:\t%s\n", program_struct->fileName);
		}
		else
		{
			fprintf(stream, "Filename:\t%s\n", notConfigured);	
		}

		// Class
		if (program_struct->elfClass)
		{
			fprintf(stream, "Class:\t\t%s\n", program_struct->elfClass);
		}
		else
		{
			fprintf(stream, "Class:\t\t%s\n", notConfigured);	
		}

		// Endianness
		if (program_struct->endianness)
		{
			fprintf(stream, "Endianness:\t%s\n", program_struct->endianness);
		}
		else if (program_struct->bigEndian == TRUE)
		{
			fprintf(stream, "Endianness:\t%s\n", "Big Endian");
		}
		else
		{
			fprintf(stream, "Endianness:\t%s\n", notConfigured);	
		}

		// Program Header Offset
		// 32-bit Processor
		if (program_struct->processorType == ELF_H_CLASS_32)
		{
			fprintf(stream, "PH Offset:\t0x%" PRIx32 "\n", program_struct->pHdr32);
		}
		// 64-bit Processor
		else if (program_struct->processorType == ELF_H_CLASS_64)
		{
			fprintf(stream, "PH Offset:\t0x%" PRIx64 "\n", program_struct->pHdr64);
		}
		// ??-bit Processor
		else
		{
			fprintf(stream, "PH Offset:\t%s\n", notConfigured);
		}

		// Program Header Size
		fprintf(stream, "PHeader Size:\t%d\n", program_struct->prgmHdrSize);

		// Number of Program Header Entries
		fprintf(stream, "# PH Entries:\t%d\n", program_struct->prgmHdrEntrNum);

		// Print the Program Header Segments
		print_program_header_segments(program_struct, stream);

		// Section delineation
		fprintf(stream, "\n\n");
	}

	return;
}


// Purpose:	Assist clean up efforts by zeroizing/free'ing an Prgrm_Hdr_Details struct
// Input:	Pointer to an Prgrm_Hdr_Details struct pointer
// Output:	ERROR_* as specified in Elf_Details.h
// Note:	This function will modify the original variable in the calling function
int kill_program_header(struct Prgrm_Hdr_Details** old_struct)
{
	int retVal = ERROR_SUCCESS;
	int i = 0;  // Iterating variable

	if (old_struct)
	{
		if (*old_struct)
		{
			/* ZEROIZE AND FREE (as appropriate) STRUCT MEMBERS */
			// void* segmentArray;  // Array of struct* (Prgrm_Hdr_Segment_32 or Prgrm_Hdr_Segment_64)
			if ((*old_struct)->segmentArray)
			{
				for (i = 0; i < (*old_struct)->prgmHdrEntrNum; i++)
				{
					retVal += take_mem_back(&(*(((*old_struct)->segmentArray) + i)), 1, sizeof(void*));
					if (retVal)
					{
						PERROR(errno);
						fprintf(stderr, "take_mem_back() returned %d on struct->segmentStruct free!\n", retVal);
						retVal = ERROR_SUCCESS;
					}
				}
				retVal += take_mem_back(&((*old_struct)->segmentArray), 1, sizeof(void*));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->segmentArray free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->segmentArray.\n");
#endif // DEBUGLEROAD
				}
			}
			// char* fileName;		// Absolute or relative path
			if ((*old_struct)->fileName)
			{
				retVal += take_mem_back((void**)&((*old_struct)->fileName), strlen((*old_struct)->fileName), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->filename free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->filename.\n");
#endif // DEBUGLEROAD
				}
			}
			// char* elfClass;		// 32 or 64 bit
			if ((*old_struct)->elfClass)
			{
				retVal += take_mem_back((void**)&((*old_struct)->elfClass), strlen((*old_struct)->elfClass), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->elfClass free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->elfClass.\n");
#endif // DEBUGLEROAD
				}
			}
			// int processorType;	// 32 or 64 bit
			(*old_struct)->processorType = 0;
			(*old_struct)->processorType |= ZEROIZE_VALUE;
#ifdef DEBUGLEROAD
			fprintf(stdout, "take_mem_back() successfully zeroized struct->processorType.\n");
#endif // DEBUGLEROAD
			// char* endianness;	// Little or Big
#ifdef DEBUGLEROAD
			fprintf(stdout, "take_mem_back() attempting to free() struct->endianness:\t%s\n", (*old_struct)->endianness);
#endif // DEBUGLEROAD
			if ((*old_struct)->endianness)
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "Attempting to free struct->endianness:\t%s\n", (*old_struct)->endianness);  // DEBUGGING
#endif // DEBUGLEROAD
				retVal += take_mem_back((void**)&((*old_struct)->endianness), strlen((*old_struct)->endianness), sizeof(char));
				if (retVal)
				{
					PERROR(errno);
					fprintf(stderr, "take_mem_back() returned %d on struct->endianness free!\n", retVal);
					retVal = ERROR_SUCCESS;
				}
				else
				{
#ifdef DEBUGLEROAD
					fprintf(stdout, "take_mem_back() successfully freed struct->endianness.\n");
#endif // DEBUGLEROAD
				}
			}
			// int bigEndian;		// If TRUE, bigEndian
			(*old_struct)->bigEndian = 0;
			(*old_struct)->bigEndian |= ZEROIZE_VALUE;
			// uint32_t pHdr32;	// 32-bit address offset of the program header table
			(*old_struct)->pHdr32 = 0;
			(*old_struct)->pHdr32 |= ZEROIZE_VALUE;
			// uint32_t pHdr64;	// 64-bit address offset of the program header table
			(*old_struct)->pHdr64 = 0;
			(*old_struct)->pHdr64 |= ZEROIZE_VALUE;
			// int prgmHdrSize; // Contains the size of a program header table entry.
			(*old_struct)->prgmHdrSize = 0;
			(*old_struct)->prgmHdrSize |= ZEROIZE_VALUE;
			// int prgmHdrEntrNum;	// Number of entries in the program header table
			(*old_struct)->prgmHdrEntrNum = 0;
			(*old_struct)->prgmHdrEntrNum |= ZEROIZE_VALUE;
			
			/* FREE THE STRUCT ITSELF */
#ifdef DEBUGLEROAD
			fprintf(stdout, "Attempting to free the struct\n");
#endif // DEBUGLEROAD
			retVal += take_mem_back((void**)old_struct, 1, sizeof(struct Prgrm_Hdr_Details));
			if (retVal)
			{
				PERROR(errno);
				fprintf(stderr, "take_mem_back() returned %d on struct free!\n", retVal);
				retVal = ERROR_SUCCESS;
			}
			else
			{
#ifdef DEBUGLEROAD
				fprintf(stdout, "take_mem_back() successfully freed struct.\n");
#endif // DEBUGLEROAD
			}
		}
		else
		{
			retVal = ERROR_NULL_PTR;
		}
	}
	else
	{
		retVal = ERROR_NULL_PTR;
	}

	return retVal;
}


// Purpose:	Allocate memory for an array of Program Header Segment struct pointers and assign the array pointer
// Input:	program_struct - A Prgrm_Hdr_Details struct pointer that contains data about an ELF file
// Output:	ERROR_* as specified in Elf_Details.h
// Note:
//			program_struct->segmentArray will receive a pointer to an array of struct pointers
//			The struct type segmentArray will be determined by program_struct->processorType
//			The segmentArray will be of "program_struct->prgmHdrEntrNum" length
//			Be sure to properly type cast the void* based on the processor type
int allocate_segment_array(struct Prgrm_Hdr_Details* program_struct)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;
	int i = 0;  // Incrementing variable

	/* INPUT VALIDATION */
	if (!program_struct)
	{
		retVal = ERROR_NULL_PTR;
	}
	else if (program_struct->processorType != ELF_H_CLASS_32 && program_struct->processorType != ELF_H_CLASS_64)
	{
		retVal = ERROR_BAD_ARG;  // Invalid processor type
	}
	else if (program_struct->prgmHdrEntrNum < 0)
	{
		retVal = ERROR_BAD_ARG;  // Invalid processor type
	}
	else if (program_struct->prgmHdrEntrNum > 0)
	{
		/* ALLOCATE MEMORY */
		if (program_struct->processorType == ELF_H_CLASS_32)
		{
			program_struct->segmentArray = (struct Prgrm_Hdr_Segment_32**)gimme_mem((size_t)program_struct->prgmHdrEntrNum, sizeof(struct Prgrm_Hdr_Segment_32*));
			if (program_struct->segmentArray)
			{
				for (i = 0; i < program_struct->prgmHdrEntrNum; i++)
				{
					// 105: error: invalid use of void expression
					(*(((struct Prgrm_Hdr_Segment_32*)program_struct->segmentArray) + i)) = (struct Prgrm_Hdr_Segment_32*)gimme_mem(1, sizeof(struct Prgrm_Hdr_Segment_32));
				}
			}
			else
			{
				retVal = ERROR_NULL_PTR;
			}
		}
		else if (program_struct->processorType == ELF_H_CLASS_64)
		{
			program_struct->segmentArray = (struct Prgrm_Hdr_Segment_64**)gimme_mem((size_t)program_struct->prgmHdrEntrNum, sizeof(struct Prgrm_Hdr_Segment_64*));
			if (program_struct->segmentArray)
			{
				for (i = 0; i < program_struct->prgmHdrEntrNum; i++)
				{
					// 105: error: invalid use of void expression
					(*(((struct Prgrm_Hdr_Segment_64*)program_struct->segmentArray) + i)) = (struct Prgrm_Hdr_Segment_64*)gimme_mem(1, sizeof(struct Prgrm_Hdr_Segment_64));
				}
			}
			else
			{
				retVal = ERROR_NULL_PTR;
			}
		}
		
	}

	return retVal;
}


// Purpose:	Print human-readable details about an ELF file's Program Header Segments
// Input:
//			program_struct - A Prgrm_Hdr_Details struct that contains data about an ELF file
//			stream - A stream to send the information to (e.g., stdout, A file)
// Output:	None
// Note:	This function will print the relevant data from program_struct->segmentArray into stream
void print_program_header_segments(struct Prgrm_Hdr_Details* program_struct, FILE* stream)
{
	/* LOCAL VARIABLES */
	struct Prgrm_Hdr_Segment_32* segment32_ptr = NULL;
	struct Prgrm_Hdr_Segment_64* segment64_ptr = NULL;
	struct Prgrm_Hdr_Segment_32** segmentArray32_ptr = NULL;
	struct Prgrm_Hdr_Segment_64** segmentArray64_ptr = NULL;
	int segmentNum = 0;
	int retVal = ERROR_SUCCESS;

	/* INPUT VALIDATION */
	if (!program_struct || !stream)
	{
		retVal = ERROR_NULL_PTR;
	}
	else if (program_struct->processorType != ELF_H_CLASS_32 && program_struct->processorType != ELF_H_CLASS_64)
	{
		retVal = ERROR_BAD_ARG;  // Invalid processor type
	}
	else if (program_struct->prgmHdrEntrNum < 0)
	{
		retVal = ERROR_BAD_ARG;  // Invalid processor type
	}
	else if (program_struct->prgmHdrEntrNum > 0)
	{
		// 32-bit Program Header Segments
		if (program_struct->processorType == ELF_H_CLASS_32)
		{
			segmentArray32_ptr = (struct Prgrm_Hdr_Segment_32**)program_struct->segmentArray;
			if (segmentArray32_ptr)
			{
				for (segmentNum = 0; segmentNum < program_struct->prgmHdrEntrNum; segmentNum++)
				{
					segment32_ptr = (struct Prgrm_Hdr_Segment_32*)(*(segmentArray32_ptr + segmentNum));
					if (segment32_ptr)
					{
						fprintf(stream, "\nSegment #%d\n", segmentNum + 1);
						fprintf(stream, "\tType:\t\t%s\n", segment32_ptr->prgmHdrType);
						fprintf(stream, "\tOffset:\t\t0x%" PRIx32 " (%" PRIu32 ")\n", segment32_ptr->segOffset);
						fprintf(stream, "\tVirtual Addr:\t0x016%" PRIx32 "\n", segment32_ptr->segVirtualAddr);
						fprintf(stream, "\tPhysical Addr:\t0x016%" PRIx32 "\n", segment32_ptr->segPhysicalAddr);
						fprintf(stream, "\tFile Size:\t%" PRIu32 "\n", segment32_ptr->segFileSize);
						fprintf(stream, "\tMem Size:\t%" PRIu32 "\n", segment32_ptr->segMemSize);
						/* PRINT THE FLAGS */
						fprintf(stream, "\tFlags:\t\t");
						// Binary printer
						// Printing flags so endianness shouldn't matter
						print_binary(stream, &(segment32_ptr->flags), sizeof(segment32_ptr->flags), TRUE);
						fprintf(stream, "\n");
						// If at least one known flag is set, tab over
						if (!(segment32_ptr->flags && (ELF_H_64_FLAG_R || ELF_H_64_FLAG_W || ELF_H_64_FLAG_X)))
						{
							fprintf(stream, "\t\t\t");
							// Read
							if ((segment32_ptr->flags && ELF_H_64_FLAG_R) == ELF_H_64_FLAG_R)
							{
								fprintf(stream, "Read ");
							}
							// Write
							if ((segment32_ptr->flags && ELF_H_64_FLAG_W) == ELF_H_64_FLAG_W)
							{
								fprintf(stream, "Write ");
							}
							// Execute
							if ((segment32_ptr->flags && ELF_H_64_FLAG_W) == ELF_H_64_FLAG_W)
							{
								fprintf(stream, "Execute ");
							}
							fprintf(stream, "\n");
						}
						/* DONE PRINTING FLAGS */
						fprintf(stream, "\tAlignment:\t%" PRIu32 "\n", segment32_ptr->alignment);
					}
				}
			}			
		}
		else if (program_struct->processorType == ELF_H_CLASS_64)
		{
			segmentArray64_ptr = (struct Prgrm_Hdr_Segment_64**)program_struct->segmentArray;
			if (segmentArray64_ptr)
			{
				for (segmentNum = 0; segmentNum < program_struct->prgmHdrEntrNum; segmentNum++)
				{
					segment64_ptr = (struct Prgrm_Hdr_Segment_64*)(*(segmentArray64_ptr + segmentNum));
					if (segment64_ptr)
					{
						fprintf(stream, "\nSegment #%d\n", segmentNum + 1);
						fprintf(stream, "\tType:\t\t%s\n", segment64_ptr->prgmHdrType);
						/* PRINT THE FLAGS */
						fprintf(stream, "\tFlags:\t\t");
						// Binary printer
						// Printing flags so endianness shouldn't matter
						print_binary(stream, &(segment64_ptr->flags), sizeof(segment64_ptr->flags), TRUE);
						fprintf(stream, "\n");
						// If at least one known flag is set, tab over
						if (!(segment64_ptr->flags && (ELF_H_64_FLAG_R || ELF_H_64_FLAG_W || ELF_H_64_FLAG_X)))
						{
							fprintf(stream, "\t\t\t");
							// Read
							if ((segment64_ptr->flags && ELF_H_64_FLAG_R) == ELF_H_64_FLAG_R)
							{
								fprintf(stream, "Read ");
							}
							// Write
							if ((segment64_ptr->flags && ELF_H_64_FLAG_W) == ELF_H_64_FLAG_W)
							{
								fprintf(stream, "Write ");
							}
							// Execute
							if ((segment64_ptr->flags && ELF_H_64_FLAG_W) == ELF_H_64_FLAG_W)
							{
								fprintf(stream, "Execute ");
							}
							fprintf(stream, "\n");
						}
						/* DONE PRINTING FLAGS */
						fprintf(stream, "\tOffset:\t\t0x%" PRIx64 " (%" PRIu64 ")\n", segment64_ptr->segOffset);
						fprintf(stream, "\tVirtual Addr:\t0x032%" PRIx64 "\n", segment64_ptr->segVirtualAddr);
						fprintf(stream, "\tPhysical Addr:\t0x032%" PRIx64 "\n", segment64_ptr->segPhysicalAddr);
						fprintf(stream, "\tFile Size:\t%" PRIu64 "\n", segment64_ptr->segFileSize);
						fprintf(stream, "\tMem Size:\t%" PRIu64 "\n", segment64_ptr->segMemSize);
						fprintf(stream, "\tAlignment:\t%" PRIu64 "\n", segment64_ptr->alignment);
					}
				}
			}	
		}
		else
		{
			// How did we get here?
			fprintf(stream, "print_program_header_segments: Unable to determine valid processor type.\n");
		}
	}

	return;
}


// Purpose:	Build a HarkleDict of Program Header Type definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_program_header_type_dict(void)
{
#ifdef DEBUGLEROAD
	puts("Entering init_program_header_type_dict()");  // DEBUGGING
#endif // DEBUGLEROAD
	/* LOCAL VARIABLES */
	struct HarkleDict* retVal = NULL;
	// The following arrays may not be in numerical order but they are still
	//	parallel.
	// FUN FACT: The arrays were originally created with an old list of ISAs and later updated.
	char* arrayOfNames[] = { \
		"ELF_H_PT_NULL - Unused", \
		"ELF_H_PT_LOAD - Loadable segment, described by p_filesz and p_memsz", \
		"ELF_H_PT_DYNAMIC - Specifies dynamic linking information", \
		"ELF_H_PT_INTERP - Location and size of interpreter's null-terminated path name", \
		"ELF_H_PT_NOTE - Location and size of auxiliary information", \
		"ELF_H_PT_SHLIB - Reserved", \
		"ELF_H_PT_PHDR - Location and size of the program header table", \
		"ELF_H_PT_TLS - Thread-Local Storage template", \
	};
	size_t numNames = sizeof(arrayOfNames)/sizeof(*arrayOfNames);
	int arrayOfValues[] = { \
		ELF_H_PT_NULL, \
		ELF_H_PT_LOAD, \
		ELF_H_PT_DYNAMIC, \
		ELF_H_PT_INTERP, \
		ELF_H_PT_NOTE, \
		ELF_H_PT_SHLIB, \
		ELF_H_PT_PHDR, \
		ELF_H_PT_TLS, \
	};
	size_t numValues = sizeof(arrayOfValues)/sizeof(*arrayOfValues);
	int i = 0;

	/* INPUT VALIDATION */
	// Verify the parallel arrays are the same length
	assert(numNames == numValues);

	for (i = 0; i < numNames; i++)
	{
		retVal = add_entry(retVal, (*(arrayOfNames + i)), (*(arrayOfValues + i)));
		if (!retVal)
		{
			fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
				(*(arrayOfNames + i)), (*(arrayOfValues + i)));
			break;
		}
	}

#ifdef DEBUGLEROAD
	puts("Starting ranged entries");  // DEBUGGING
#endif // DEBUGLEROAD
	// RANGED ENTRIES
	// NOTE:  These ranged entries are commented about because the creation of the Harkledict
	//			entries is taking too long.
	// for (i = ELF_H_PT_LOOS; i <= ELF_H_PT_HIOS; i++)
	// {
	// 	fprintf(stdout, "Harkledict adding OS-specific #%d\n", i);
	// 	retVal = add_entry(retVal, "Reserved for OS-specific semantics", i);
	// 	if (!retVal)
	// 	{
	// 		fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
	// 			"Reserved for OS-specific semantics", i);
	// 		break;
	// 	}
	// }
	// for (i = ELF_H_PT_LOPROC; i <= ELF_H_PT_HIPROC; i++)
	// {
	// 	fprintf(stdout, "Harkledict adding processor-specific #%d\n", i);
	// 	retVal = add_entry(retVal, "Reserved for processor-specific semantics", i);
	// 	if (!retVal)
	// 	{
	// 		fprintf(stderr, "Harkledict add_entry() returned NULL for:\n\tName:\t%s\n\tValue:\t%d\n", \
	// 			"Reserved for processor-specific semantics", i);
	// 		break;
	// 	}
	// }

#ifdef DEBUGLEROAD
	puts("Exiting init_program_header_type_dict()");  // DEBUGGING
#endif // DEBUGLEROAD
	return retVal;
}


/********************************************************/
/********************************************************/
/***************** PROGRAM HEADER STOP ******************/
/********************************************************/
/********************************************************/


/********************************************************/
/********************************************************/
/***************** HELPER FUNCTION START ****************/
/********************************************************/
/********************************************************/


// Purpose:	Wrapper for individual header parse function calls
// Input:
// 			[in] elvenFilename - Filename, relative or absolute, to an ELF file
//			[out] ELFstruct - Pointer to an Elf_Details struct pointer
//			[out] PHstruct - Pointer to an Prgrm_Hdr_Details struct pointer
//			program_contents - ELF file contents
// Output:	ERROR_* as specified in Elf_Details.h
// Note:
//			This function calls parse_elf() and parse_program_header()
//			ELFstruct may not be NULL but *ELFstruct is expected to be NULL and will be overwritten
//			PHstruct may not be NULL but *PHstruct is expected to be NULL and will be overwritten
//			This function will return ERROR_BAD_ARG if either *ELFstruct or *PHstruct is not NULL
int read_elf_file(char* elvenFilename, struct Elf_Details** ELFstruct, struct Prgrm_Hdr_Details** PHstruct)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;

	/* INPUT VALIDATION */
	if (!elvenFilename || !ELFstruct || !PHstruct)  // || !SHstruct
	{
		retVal = ERROR_NULL_PTR;
	}
	else if (*ELFstruct || *PHstruct)  // || *SHstruct
	{
		retVal = ERROR_BAD_ARG;
	}
	else
	{
		*ELFstruct = read_elf(elvenFilename);

		if (*ELFstruct)
		{
#ifdef DEBUGLEROAD
			fprintf(stdout, "Successfully read ELF header.\n");  // DEBUGGING
#endif // DEBUGLEROAD
			*PHstruct = read_program_header(elvenFilename, *ELFstruct);

			if (*PHstruct)
			{
				// *SHstruct = read_section_header(elvenFilename, *ELFstruct);
#ifdef DEBUGLEROAD
				fprintf(stdout, "Successfully read program header.\n");  // DEBUGGING
#endif // DEBUGLEROAD
			}
			else
			{
				PERROR(errno);
				retVal = ERROR_NULL_PTR;
			}
		}
		else
		{
			PERROR(errno);
			retVal = ERROR_NULL_PTR;
		}
	}

	return retVal;
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
	fprintf(stream, "%c%c%c %s %c%c%c\n", delimiter, delimiter, delimiter, title, delimiter, delimiter, delimiter);

	// Third line
	for (i = 0; i < headerWidth; i++)
	{
		fputc(delimiter, stream);
	}
	fputc(0xA, stream);

	return;
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


// Purpose:	Wrap calloc
// Input:
//			numElem - function allocates memory for an array of numElem elements
//			sizeElem - size of each numElem
// Output:	Pointer to dynamically allocated array
// Note:	
//			Cast the return value to the type you want
//			It is the responsibility of the calling function to free the mem returned
void* gimme_mem(size_t numElem, size_t sizeElem)
{
	void* retVal = NULL;
	int numRetries = 0;

	for (; numRetries <= MAX_RETRIES; numRetries++)
	{
		retVal = (void*)calloc(numElem, sizeElem);
		if (retVal)
		{
			break;
		}
	}

	if (!retVal)
	{
		PERROR(errno);
	}

	return retVal;
}


// Purpose:	Automate zeroizing, free'ing, and NULL'ing of dynamically allocated memory
// Input:	
//			buff - Pointer to a buffer pointer
//			numElem - The number of things in *buff
//			sizeElem - The size of each thing in *buff
// Output:	ERROR_* as specified in Elf_Details.h
// Note:	Modifies the pointer to *buf by making it NULL
int take_mem_back(void** buff, size_t numElem, size_t sizeElem)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;

	/* INPUT VALIDATION */
	if (!buff)
	{
		retVal = ERROR_NULL_PTR;
	}
	else if (!(*buff))
	{
		retVal = ERROR_NULL_PTR;
	}
	else if (numElem < 1 || sizeElem < 1)
	{
		retVal = ERROR_BAD_ARG;
	}
	/* FREE MEMORY */
	else
	{
		// Zeroize the memory
		memset(*buff, ZEROIZE_CHAR, numElem * sizeElem);
		PERROR(errno);

		// Free the memory
		free(*buff);
		PERROR(errno);

		// NULL the pointer variable
		*buff = NULL;
	}

	return retVal;
}


// Purpose:	Convert consecutive characters into a single int IAW the specified endianness
// Input:
//			buffToConvert - Pointer to the buffer that holds the bytes in question
//			dataOffset - Starting location in buffToConvert
//			numBytesToConvert - Number of bytes to translate starting at buffToConvert[dataOffset]
//			bigEndian - If True, bigEndian byte ordering
//			translation [out] - Pointer to memory space to hold the translated value
// Output:	ERROR_* as specified in Elf_Details.h on success and failure
// Note:	Behavior is as follows:
//			If bigEndian == TRUE:
//				Addr + 0x0:	0xFE
//				Addr + 0x1:	0xFF
//				Returns:	0xFEFF == 65279
//			If bigEndian == FALSE:
//				Addr + 0x0:	0xFE
//				Addr + 0x1:	0xFF
//				Returns:	0xFFFE == 65534
//			Also, translation will always be zeroized if input validation is passed
int convert_char_to_int(char* buffToConvert, int dataOffset, \
	                    int numBytesToConvert, int bigEndian, \
	                    unsigned int* translation)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;	// Function return value
	unsigned int value = 0;		// Holds the current translated value prior to return
	int i = 0;					// Iterating variable

	/* INPUT VALIDATION */
	if (!buffToConvert || !translation)
	{
		retVal = ERROR_NULL_PTR;
		return retVal;
	}
	else if (dataOffset < 0)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	else if (numBytesToConvert < 1)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	else if (bigEndian != TRUE && bigEndian != FALSE)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	else if (numBytesToConvert > sizeof(value))
	{
		retVal = ERROR_OVERFLOW;
		return retVal;
	}
	else
	{
		// Zeroize translation
		*translation = value;
	}

	if (bigEndian == TRUE)
	{
		for (i = dataOffset; i < (dataOffset + numBytesToConvert); i++)
		{
			value |= (unsigned int)(*(buffToConvert + i));
			// printf("Value at %p:\t%d(0x%X)\n", buffToConvert + i, value, value);  // DEBUGGING
			if ((i + 1) < (dataOffset + numBytesToConvert))
			{
				value <<= 8;
				// printf("Value bit shift:\t\t%d(0x%X)\n", value, value);  // DEBUGGING
			}
		}
		// We started at the top and now we're here
	}
	else if (bigEndian == FALSE)
	{
		// printf("buffToConvert[BUFF_SIZE - 2] == %d(0x%X)\n", (*(buffToConvert + 1024 - 2)), (*(buffToConvert + 1024 - 2)));  // DEBUGGING
		// printf("buffToConvert[BUFF_SIZE - 1] == %d(0x%X)\n", (*(buffToConvert + 1024 - 1)), (*(buffToConvert + 1024 - 1)));  // DEBUGGING

		for (i = (dataOffset + numBytesToConvert - 1); i >= dataOffset ; i--)
		{
			value |= (unsigned int)(*(buffToConvert + i));
			// printf("Value at %p:\t%d(0x%X)\n", buffToConvert + i, value, value);  // DEBUGGING
			if (i > dataOffset)
			{
				value <<= 8;
				// printf("Value bit shift:\t\t%d(0x%X)\n", value, value);  // DEBUGGING
			}
		}
		// We started at the bottom and now we're here
	}
	else
	{
		// How did we get here?!
		retVal = ERROR_BAD_ARG;
	}

	// Done
	if (retVal == ERROR_SUCCESS)
	{
		*translation = value;
	}
	return retVal;
}


// Purpose:	Convert consecutive characters into a single uint64_t IAW the specified endianness
// Input:
//			buffToConvert - Pointer to the buffer that holds the bytes in question
//			dataOffset - Starting location in buffToConvert
//			numBytesToConvert - Number of bytes to translate starting at buffToConvert[dataOffset]
//			bigEndian - If True, bigEndian byte ordering
//			translation [out] - Pointer to memory space to hold the translated value
// Output:	ERROR_* as specified in Elf_Details.h on success and failure
// Note:	Behavior is as follows:
//			If bigEndian == TRUE:
//				Addr + 0x0:	0xFE
//				Addr + 0x1:	0xFF
//				Returns:	0xFEFF == 65279
//			If bigEndian == FALSE:
//				Addr + 0x0:	0xFE
//				Addr + 0x1:	0xFF
//				Returns:	0xFFFE == 65534
//			Also, translation will always be zeroized if input validation is passed
//			Finally, this function is a 64-bit wrapper to convert_char_to_int()
int convert_char_to_uint64(char* buffToConvert, int dataOffset, \
	                       int numBytesToConvert, int bigEndian, \
	                       uint64_t* translation)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;	// Function return value
	uint64_t value = 0;			// Holds the current translated value prior to return
	int i = 0;					// Iterating variable
	unsigned int tmpValue = 0;	// Holds the value from successive calls to ccti()

	/* INPUT VALIDATION */
	if (!buffToConvert || !translation)
	{
		retVal = ERROR_NULL_PTR;
		return retVal;
	}
	else if (dataOffset < 0)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	else if (numBytesToConvert < 1)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	else if (bigEndian != TRUE && bigEndian != FALSE)
	{
		retVal = ERROR_BAD_ARG;
		return retVal;
	}
	else if (numBytesToConvert > sizeof(value))
	{
		retVal = ERROR_OVERFLOW;
		return retVal;
	}
	else
	{
		// Zeroize translation
		*translation = value;
	}

	/* CALL convert_char_to_int() */
	if (bigEndian == TRUE)
	{
		for (i = dataOffset; i < (dataOffset + numBytesToConvert); i++)
		{
			retVal += convert_char_to_int(buffToConvert, i, 1, bigEndian, &tmpValue);
			if (retVal)
			{
				tmpValue = 0;
				*translation = 0;
				return retVal;
			}
			else
			{
				value |= tmpValue;
				tmpValue = 0;
				if ((i + 1) < (dataOffset + numBytesToConvert))
				{
					value <<= 8;
				}
			}
		}		
		// We started at the top and now we're here
	}
	else if (bigEndian == FALSE)
	{
		for (i = (dataOffset + numBytesToConvert - 1); i >= dataOffset; i--)
		{
			retVal += convert_char_to_int(buffToConvert, i, 1, bigEndian, &tmpValue);
			if (retVal)
			{
				tmpValue = 0;
				*translation = 0;
				return retVal;
			}
			else
			{
				value |= tmpValue;
				tmpValue = 0;
				if (i > dataOffset)
				{
					value <<= 8;
				}
			}
		}
		// We started at the bottom and now we're here
	}
	else
	{
		// How did we get here?!
		retVal = ERROR_BAD_ARG;
	}
	
	// Done
	if (retVal == ERROR_SUCCESS)
	{
		*translation = value;
	}
	return retVal;
}


// Purpose:	Safely convert a uint64_t to a uint32_t
// Input:
//			inVal - Input a uint64_t value
//			outVal - Pointer to a uint32_t that will receive converted value
// Ouput:	ERROR_* as specified in Elf_Details.h on success and failure
int convert_uint64_to_uint32(uint64_t inVal, uint32_t* outVal)
{
	/* LOCAL VARIABLES */
	int retVal = ERROR_SUCCESS;	// Function's return value
	uint32_t newVal = 0;		// Value to calculate from the input uint64_t
	// uint32_t tmpVal = 0;		// Used to hold bits of the uint64_t
	int i = 0;					// Iterating variable

	/* INPUT VALIDATION */
	if (!outVal)
	{
		retVal = ERROR_NULL_PTR;
	}
	else if (inVal > 0xFFFFFFFF)
	{
		retVal = ERROR_OVERFLOW;
	}
	else
	{
		*outVal = newVal;
		newVal = inVal & 0x00000000FFFFFFFF;
	}

	/* DONE */
	if (retVal == ERROR_SUCCESS)
	{
		*outVal = newVal;
	}

	return retVal;
}


// Purpose:	Print a certain number of bytes in binary
// Input:	
//			stream - The stream to print output
//			valueToPrint - A variable to print data from
//			numBytesToPrint - The number of bytes to print
//			bigEndian - If TRUE, bigEndian byte ordering
// Output:	None
// Note:	
//			Endianness does not appear to matter since these are flags
//			Function will print one space on invalid input
void print_binary(FILE* stream, void* valueToPrint, size_t numBytesToPrint, int bigEndian)
{
	/* LOCAL VARIABLES */
	unsigned char mask = 0;			// Bit mask used to print bits
	unsigned char printThis = 0;	// Value to print
	int i = 0;					// Iterating variable
	int j = 0;					// Iterating variable
	// int numOfPasses = 0;	// Number of passes for the mask

	/* INPUT VALIDATION */
	if (!stream)
	{
		return;
	}
	else if (!valueToPrint)
	{
		fprintf(stream, " ");
		return;
	}
	else if (numBytesToPrint < 1)
	{
		fprintf(stream, " ");
		return;
	}
	else if (bigEndian != TRUE && bigEndian != FALSE)
	{
		fprintf(stream, " ");
		return;
	}

	/* START PRINTING */
	if (bigEndian == TRUE)
	{
		for (i = 0; i < numBytesToPrint; i++)
		{
			printThis = (unsigned char)(*(unsigned char*)(valueToPrint + i));

			for (j = 7; j >= 0; j--)
			{
				if (j == 3)
				{
					fprintf(stream, " ");
				}

				mask = 1 << j;
				// printf("i == %d\tj== %d\tMask:\t0x%02X\tCurrent Value:\t0x%02X\n", i, j, mask, printThis);  // DEBUGGING
				if (printThis & mask)
				{
					fprintf(stream, "1");
				}
				else
				{
					fprintf(stream, "0");
				}
			}
			fprintf(stream, " ");
		}
	}
	else if (bigEndian == FALSE)
	{
		for (i = (numBytesToPrint - 1); i >= 0; i--)
		{
			printThis = (unsigned char)(*(unsigned char*)(valueToPrint + i));

			for (j = 7; j >= 0; j--)
			{
				if (j == 3)
				{
					fprintf(stream, " ");
				}

				mask = 1 << j;
				// printf("i == %d\tj== %d\tMask:\t0x%02X\tCurrent Value:\t0x%02X\n", i, j, mask, printThis);  // DEBUGGING
				if (printThis & mask)
				{
					fprintf(stream, "1");
				}
				else
				{
					fprintf(stream, "0");
				}
			}
			fprintf(stream, " ");
		}
	}
	else
	{
		fprintf(stream, " ");
	}

	return;
}


/********************************************************/
/********************************************************/
/***************** HELPER FUNCTION STOP *****************/
/********************************************************/
/********************************************************/
