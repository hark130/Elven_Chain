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

	/* FINAL CLEAN UP */
	if (elfGuts)
	{
		take_mem_back((void**)&elfGuts, elfSize + 1, sizeof(char));
		// memset(elfGuts, 0, elfSize);
		// free(elfGuts);
		// elfGuts = NULL;
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

	// 2.6. ABI Version (OFFSET: 0x08)
	dataOffset += 1;  // 8
	elven_struct->ABIversion = (int)(*(elven_contents + dataOffset));
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

	// 2.9. Object File Version
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

	// 2.10 Entry Point
	dataOffset += 4;
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

	// 2.12. Section Header Table Offset
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

	/* CLEAN UP */
	// Zeroize/Free/NULLify tempBuff
	// if (tmpBuff)
	// {
	// 	take_mem_back((void**)&tmpBuff, strlen(elven_contents) + 1, sizeof(char));
	// }

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
			fprintf(stream, "PH Offset:\t0x%" PRIx32 "\n", elven_file->pHdr32);
		}
		// 64-bit Processor
		else if (elven_file->processorType == ELF_H_CLASS_64)
		{
			fprintf(stream, "PH Offset:\t0x%" PRIx64 "\n", elven_file->pHdr64);
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
			// Implement binary printer
			fprintf(stream, "\n");
		}
		// ??-bit Processor
		else
		{
			fprintf(stream, "Flags:\t\t%s\n", notConfigured);
		}

		// Section delineation
		fprintf(stream, "\n\n");
	}

	/* PROGRAM HEADER */
	if (sectionsToPrint & PRINT_ELF_PRGRM_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Header
		print_fancy_header(stream, "PROGRAM HEADER", HEADER_DELIM);
		// Implement later
		fprintf(stream, "\n\n");
	}

	/* SECTION HEADER */
	if (sectionsToPrint & PRINT_ELF_SECTN_HEADER || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Header
		print_fancy_header(stream, "SECTION HEADER", HEADER_DELIM);
		// Implement later
		fprintf(stream, "\n\n");
	}

	/* PROGRAM DATA */
	if (sectionsToPrint & PRINT_ELF_PRGRM_DATA || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Header
		print_fancy_header(stream, "PROGRAM DATA", HEADER_DELIM);
		// Implement later
		fprintf(stream, "\n\n");
	}

	/* SECTION DATA */
	if (sectionsToPrint & PRINT_ELF_SECTN_DATA || sectionsToPrint & PRINT_EVERYTHING)
	{
		// Header
		print_fancy_header(stream, "SECTION DATA", HEADER_DELIM);
		// Implement later
		fprintf(stream, "\n\n");
	}

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
				printf("i == %d\tj== %d\tMask:\t0x%02X\tCurrent Value:\t0x%02X\n", i, j, mask, printThis);  // DEBUGGING
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
