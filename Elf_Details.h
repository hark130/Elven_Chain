// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// Additional macros gleaned from:
//	http://www.sco.com/developers/gabi/latest/ch4.eheader.html
#ifndef __ELF_DETAILS_H__
#define __ELF_DETAILS_H__

#include "Harklehash.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#define ERROR_SUCCESS	((int)0)	// EUREKA!
#define ERROR_NULL_PTR	((int)-1)	// NULL pointer
#define ERROR_BAD_ARG	((int)-2)	// Bad arguments
#define ERROR_ORC_FILE	((int)-3)	// Indicates this is not an ELF file
#define ERROR_OVERFLOW	((int)-4)	// The given data type will overflow
#define MAX_RETRIES		((int)10)	// Number of times to retry a function call before giving up
#define ZEROIZE_VALUE	((int)42)	// Value used to 'clear' int values
#define ZEROIZE_CHAR	((char)'H') // Character used to memset free()'d memory
#define HEADER_DELIM	((char)'#')	// Character used to print fance output headers
// #define DEBUGLEROAD					// No IDEs were harmed during the coding of this project

#ifndef TRUE
#define TRUE ((int)1)
#endif // TRUE

#ifndef FALSE
#define FALSE ((int)0)
#endif // FALSE

#ifndef DEBUGLEROAD
#ifndef PERROR
#define PERROR(errnum) \
do { if (errnum) { printf("Error Number:\t%d\nError Description:\t%s\n", errnum, strerror(errnum)); } } while(0);
#endif // PERROR
#else
#ifndef PERROR
#define PERROR(errnum) ;
#endif // PERROR
#endif // DEBUGLEROAD

// #define SUPER_STR_ME(str) #str
// #define EXTRA_STR_ME(str) SUPER_STR_ME(str)
// #define STR_ME(str) SUPER_STR_ME(#str)
#define STR_ME(str) #str

/****************************/
/***** ELF HEADER START *****/
/****************************/
// Magic Number 0x00 - 0x03
#define ELF_H_MAGIC_NUM		"\x7f\x45\x4c\x46"	// 7F E L F
// Class 0x04
#define ELF_H_CLASS_NONE		0x00 			// 0
#define ELF_H_CLASS_32			0x01			// 1
#define ELF_H_CLASS_64			0x02			// 2
// Endianness 0x05
#define ELF_H_DATA_NONE			0x00 			// 0
#define ELF_H_DATA_LITTLE		0x01			// 1
#define ELF_H_DATA_BIG			0x02			// 2
// Version 0x06
#define ELF_H_VERSION			0x01			// 1
// Target Operating System ABI 0x07
#define ELF_H_OSABI_SYSTEM_V	0x00			// 0
#define ELF_H_OSABI_HP_UX		0x01			// 1
#define ELF_H_OSABI_NETBSD  	0x02			// 2
#define ELF_H_OSABI_LINUX   	0x03			// 3
#define ELF_H_OSABI_GNU_HURD	0x04			// 4
#define ELF_H_OSABI_SOLARIS 	0x06			// 6
#define ELF_H_OSABI_AIX     	0x07			// 7
#define ELF_H_OSABI_IRIX    	0x08			// 8
#define ELF_H_OSABI_FREE_BSD	0x09			// 9
#define ELF_H_OSABI_TRU64   	0x0A			// 10
#define ELF_H_OSABI_NOVELL  	0x0B			// 11
#define ELF_H_OSABI_OPEN_BSD	0x0C			// 12
#define ELF_H_OSABI_OPEN_VMS	0x0D			// 13
#define ELF_H_OSABI_NONSTOP_K	0x0E			// 14
#define ELF_H_OSABI_AROS    	0x0F			// 15
#define ELF_H_OSABI_FENIX_OS	0x10			// 16
#define ELF_H_OSABI_CLOUB_ABI	0x11			// 17
#define ELF_H_OSABI_SORTIX  	0x53			// 83
// ABI Version 0x08
// 		No assumed values here.  int-->str or int-->int?
// Pad 0x09 - 0x0F
// 		Currently unused
// Type 0x10 - 0x11
#define ELF_H_TYPE_NONE			0x00			// 0
#define ELF_H_TYPE_RELOCATABLE	0x01			// 1
#define ELF_H_TYPE_EXECUTABLE	0x02			// 2
#define ELF_H_TYPE_SHARED		0x03			// 3
#define ELF_H_TYPE_CORE			0x04			// 4
#define ELF_H_TYPE_LO_OS		0xFE00			// 65024
#define ELF_H_TYPE_HI_OS		0xFEFF			// 65279
#define ELF_H_TYPE_LO_PROC		0xFF00			// 65280
#define ELF_H_TYPE_HI_PROC		0xFFFF			// 65535
// Instruction Set Architecture 0x12 - 0x13
#define ELF_H_ISA_NONE 			0 				// No machine
#define ELF_H_ISA_M32 			1 				// AT&T WE 32100
#define ELF_H_ISA_SPARC 		2 				// SPARC
#define ELF_H_ISA_386 			3 				// Intel 80386
#define ELF_H_ISA_68K 			4 				// Motorola 68000
#define ELF_H_ISA_88K 			5 				// Motorola 88000
#define ELF_H_ISA_IAMCU 	 	6 				// Intel MCU
#define ELF_H_ISA_860 			7 				// Intel 80860
#define ELF_H_ISA_MIPS 			8 				// MIPS I Architecture
#define ELF_H_ISA_S370 			9 				// IBM System/370 Processor
#define ELF_H_ISA_MIPS_RS3_LE 	10 				// MIPS RS3000 Little-endian
// RESERVED 	11-14 	Reserved for future use
#define ELF_H_ISA_PARISC 		15 				// Hewlett-Packard PA-RISC
// RESERVED 	16 	Reserved for future use
#define ELF_H_ISA_VPP500 		17 				// Fujitsu VPP500
#define ELF_H_ISA_SPARC32PLUS 	18 				// Enhanced instruction set SPARC
#define ELF_H_ISA_960 			19 				// Intel 80960
#define ELF_H_ISA_PPC 			20 				// PowerPC
#define ELF_H_ISA_PPC64 		21 				// 64-bit PowerPC
#define ELF_H_ISA_S390			22				// IBM System/390 Processor
#define ELF_H_ISA_SPU			23				// IBM SPU/SPC
// RESERVED 	24-35 	Reserved for future use
#define ELF_H_ISA_V800 			36 				// NEC V800
#define ELF_H_ISA_FR20 			37 				// Fujitsu FR20
#define ELF_H_ISA_RH32 			38 				// TRW RH-32
#define ELF_H_ISA_RCE 			39 				// Motorola RCE
#define ELF_H_ISA_ARM 			40 				// Advanced RISC Machines ARM
#define ELF_H_ISA_ALPHA 		41 				// Digital Alpha
#define ELF_H_ISA_SH 			42 				// Hitachi SH
#define ELF_H_ISA_SPARCV9 		43 				// SPARC Version 9
#define ELF_H_ISA_TRICORE 		44 				// Siemens Tricore embedded processor
#define ELF_H_ISA_ARC 			45 				// Argonaut RISC Core, Argonaut Technologies Inc.
#define ELF_H_ISA_H8_300 		46 				// Hitachi H8/300
#define ELF_H_ISA_H8_300H 		47 				// Hitachi H8/300H
#define ELF_H_ISA_H8S 			48 				// Hitachi H8S
#define ELF_H_ISA_H8_500 		49 				// Hitachi H8/500
#define ELF_H_ISA_IA_64 		50 				// Intel IA-64 processor architecture
#define ELF_H_ISA_MIPS_X 		51 				// Stanford MIPS-X
#define ELF_H_ISA_COLDFIRE 		52 				// Motorola ColdFire
#define ELF_H_ISA_68HC12 		53 				// Motorola M68HC12
#define ELF_H_ISA_MMA 			54 				// Fujitsu MMA Multimedia Accelerator
#define ELF_H_ISA_PCP 			55 				// Siemens PCP
#define ELF_H_ISA_NCPU 			56 				// Sony nCPU embedded RISC processor
#define ELF_H_ISA_NDR1 			57 				// Denso NDR1 microprocessor
#define ELF_H_ISA_STARCORE 		58 				// Motorola Star*Core processor
#define ELF_H_ISA_ME16 			59 				// Toyota ME16 processor
#define ELF_H_ISA_ST100 		60 				// STMicroelectronics ST100 processor
#define ELF_H_ISA_TINYJ 		61 				// Advanced Logic Corp. TinyJ embedded processor family
#define ELF_H_ISA_X86_64 		62 				// AMD x86-64 architecture
#define ELF_H_ISA_PDSP 			63 				// Sony DSP Processor
#define ELF_H_ISA_PDP10 		64 				// Digital Equipment Corp. PDP-10
#define ELF_H_ISA_PDP11 		65 				// Digital Equipment Corp. PDP-11
#define ELF_H_ISA_FX66 			66 				// Siemens FX66 microcontroller
#define ELF_H_ISA_ST9PLUS 		67 				// STMicroelectronics ST9+ 8/16 bit microcontroller
#define ELF_H_ISA_ST7 			68 				// STMicroelectronics ST7 8-bit microcontroller
#define ELF_H_ISA_68HC16 		69 				// Motorola MC68HC16 Microcontroller
#define ELF_H_ISA_68HC11 		70 				// Motorola MC68HC11 Microcontroller
#define ELF_H_ISA_68HC08 		71 				// Motorola MC68HC08 Microcontroller
#define ELF_H_ISA_68HC05 		72 				// Motorola MC68HC05 Microcontroller
#define ELF_H_ISA_SVX 			73 				// Silicon Graphics SVx
#define ELF_H_ISA_ST19 			74 				// STMicroelectronics ST19 8-bit microcontroller
#define ELF_H_ISA_VAX 			75 				// Digital VAX
#define ELF_H_ISA_CRIS 			76 				// Axis Communications 32-bit embedded processor
#define ELF_H_ISA_JAVELIN 		77 				// Infineon Technologies 32-bit embedded processor
#define ELF_H_ISA_FIREPATH 		78 				// Element 14 64-bit DSP Processor
#define ELF_H_ISA_ZSP 			79 				// LSI Logic 16-bit DSP Processor
#define ELF_H_ISA_MMIX 			80 				// Donald Knuth's educational 64-bit processor
#define ELF_H_ISA_HUANY 		81 				// Harvard University machine-independent object files
#define ELF_H_ISA_PRISM 		82 				// SiTera Prism
// Object File Version 0x14 - 0x17
#define ELF_H_OBJ_V_NONE		0				// Invalid version
#define ELF_H_OBJ_V_CURRENT		1				// Current version
/****************************/
/***** ELF HEADER STOP ******/
/****************************/

/* sectionsToPrint Flags for print_elf_details() */
#define PRINT_EVERYTHING		((unsigned int)1)			// Print everything
#define PRINT_ELF_HEADER		(((unsigned int)1) << 1)	// Print the ELF header
#define PRINT_ELF_PRGRM_HEADER	(((unsigned int)1) << 2)	// Print the Program header
#define PRINT_ELF_SECTN_HEADER	(((unsigned int)1) << 3)	// Print the Section header
#define PRINT_ELF_PRGRM_DATA	(((unsigned int)1) << 4)	// Print the Program header data
#define PRINT_ELF_SECTN_DATA	(((unsigned int)1) << 5)	// Print the Section header data 

struct Elf_Details
{
	char* fileName;		// Absolute or relative path
	char* magicNum;		// First four bytes of file
	char* elfClass;		// 32 or 64 bit
	int processorType;	// 32 or 64 bit
	char* endianness;	// Little or Big
	int bigEndian;		// If TRUE, bigEndian
	int elfVersion;		// ELF version
	char* targetOS;		// Target OS ABI
	int ABIversion;		// Version of the ABI
	char* pad;			// Unused portion
	char* type;			// The type of ELF file
	char* ISA;			// Specifies target Instruction Set Architecture
	char* objVersion;	// Object File Version
	uint32_t ePnt32;	// 32-bit memory address of the entry point from where the process starts executing
	uint64_t ePnt64;	// 64-bit memory address of the entry point from where the process starts executing
	uint32_t pHdr32;	// 32-bit address offset of the program header table
	uint64_t pHdr64;	// 64-bit address offset of the program header table
	uint32_t sHdr32;	// 32-bit address offset of the section header table
	uint64_t sHdr64;	// 64-bit address offset of the section header table
	unsigned int flags;	// Interpretation of this field depends on the target architecture
	int elfHdrSize;		// ELF Header Size
	int prgmHdrSize;	// Contains the size of a program header table entry.
	int prgmHdrEntrNum;	// Number of entries in the program header table
	int sectHdrSize;	// Contains the size of a section header table entry.
};
// All char* members should be dynamically allocated and later free()'d


// Purpose: Open and parse an ELF file.  Allocate, configure and return Elf_Details pointer.
// Input:	Filename, relative or absolute, to an ELF file
// Output:	A dynamically allocated Elf_Details struct that contains information about elvenFilename
// Note:	It is caller's responsibility to free the return value from this function by calling
//				kill_elf()
struct Elf_Details* read_elf(char* elvenFilename);

// Purpse:	Parse an ELF file contents into an Elf_Details struct
// Input:
//			elven_struct - Struct to store elven details
//			elven_contents - ELF file contents
// Output:	ERROR_* as specified in Elf_Details.h
int parse_elf(struct Elf_Details* elven_struct, char* elven_contents);

// Purpose:	Print human-readable details about an ELF file
// Input:
//			elven_file - A Elf_Details struct that contains data about an ELF file
//			sectionsToPrint - Bitwise AND the "PRINT_*" macros into this variable
//				to control what the function actually prints.
//			stream - A stream to send the information to (e.g., stdout, A file)
// Output:	None
// Note:	This function will print the relevant data from elven_file into stream
//				based on the flags found in sectionsToPrint
void print_elf_details(struct Elf_Details* elven_file, unsigned int sectionsToPrint, FILE* stream);

// Purpose:	Assist clean up efforts by zeroizing/free'ing an Elf_Details struct
// Input:	Pointer to an Elf_Details struct pointer
// Output:	ERROR_* as specified in Elf_Details.h
// Note:	This function will modify the original variable in the calling function
int kill_elf(struct Elf_Details** old_struct);

// Purpose:	Prints an uppercase title surrounded by delimiters
// Input:
//			stream - Stream to print the header to
//			title - Title to print
//			delimiter - Single character to create the box
// Output:	None
// Note:	Automatically sizes the box
void print_fancy_header(FILE* stream, char* title, unsigned char delimiter);

// Purpose:	Determine the exact length of a file
// Input:	Open FILE pointer
// Output:	Exact length of file in bytes
size_t file_len(FILE* openFile);

// Purpose:	Print a buffer, regardless of nul characters
// Input:	
//			buff - non-nul terminated char array
//			size - number of characters in buff
// Output:	Number of characters printed
size_t print_it(char* buff, size_t size);

// Purpose:	Wrap calloc
// Input:
//			numElem - function allocates memory for an array of numElem elements
//			sizeElem - size of each numElem
// Output:	Pointer to dynamically allocated array
// Note:	
//			Cast the return value to the type you want
//			It is the responsibility of the calling function to free the mem returned
void* gimme_mem(size_t numElem, size_t sizeElem);

// Purpose:	Automate zeroizing, free'ing, and NULL'ing of dynamically allocated memory
// Input:	
//			buff - Pointer to a buffer pointer
//			numElem - The number of things in *buff
//			sizeElem - The size of each thing in *buff
// Output:	ERROR_* as specified in Elf_Details.h
// Note:	Modifies the pointer to *buf by making it NULL
int take_mem_back(void** buff, size_t numElem, size_t sizeElem);

// Purpose:	Convert consecutive characters into a single int IAW the specified endianness
// Input:
//			buffToConvert - Pointer to the buffer that holds the bytes in question
//			dataOffset - Starting location in buffToConvert
//			numBytesToConvert - Number of bytes to translate starting at buffToConvert[dataOffset]
//			bigEndian - If True, bigEndian byte ordering
//			translation [out] - Pointer to memory space to hold the translated value
// Output:	The translation of the raw values found in the first numBytesToConvert bytes in
//				buffToConvert on success
//			ERROR_* as specified in Elf_Details.h on failure
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
	                    unsigned int* translation);

// Purpose:	Convert consecutive characters into a single uint64_t IAW the specified endianness
// Input:
//			buffToConvert - Pointer to the buffer that holds the bytes in question
//			dataOffset - Starting location in buffToConvert
//			numBytesToConvert - Number of bytes to translate starting at buffToConvert[dataOffset]
//			bigEndian - If True, bigEndian byte ordering
//			translation [out] - Pointer to memory space to hold the translated value
// Output:	The translation of the raw values found in the first numBytesToConvert bytes in
//				buffToConvert on success
//			ERROR_* as specified in Elf_Details.h on failure
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
	                       uint64_t* translation);

// Purpose:	Safely convert a uint64_t to a uint32_t
// Input:
//			inVal - Input a uint64_t value
//			outVal - Pointer to a uint32_t that will receive converted value
// Ouput:	ERROR_* as specified in Elf_Details.h on success and failure
int convert_uint64_to_uint32(uint64_t inVal, uint32_t* outVal);

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
void print_binary(FILE* stream, void* valueToPrint, size_t numBytesToPrint, int bigEndian);

// Purpose:	Build a HarkleDict of Elf Header Class definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_class_dict(void);

// Purpose:	Build a HarkleDict of Elf Header Data definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_endian_dict(void);

// Purpose:	Build a HarkleDict of Elf Header Target OS ABI definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_targetOS_dict(void);

// Purpose:	Build a HarkleDict of Elf Header Type definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_elf_type_dict(void);

// Purpose:	Build a HarkleDict of Elf Header ISA definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_isa_dict(void);

// Purpose:	Build a HarkleDict of Elf Header Object File Version definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_elf_header_obj_version_dict(void);

#endif // __ELF_DETAILS_H__
