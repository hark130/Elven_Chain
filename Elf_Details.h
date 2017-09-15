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

/********************************/
/***** PROGRAM HEADER START *****/
/********************************/
// http://www.sco.com/developers/gabi/latest/ch5.pheader.html
#define ELF_H_PT_NULL 		0			// Unused
#define ELF_H_PT_LOAD 		1			// Loadable segment, described by p_filesz and p_memsz
#define ELF_H_PT_DYNAMIC 	2			// Specifies dynamic linking information
#define ELF_H_PT_INTERP 	3			// Location and size of interpreter's null-terminated path name 
#define ELF_H_PT_NOTE 		4			// Location and size of auxiliary information
#define ELF_H_PT_SHLIB 		5			// Reserved
#define ELF_H_PT_PHDR 		6			// Location and size of the program header table
#define ELF_H_PT_TLS 		7			// Thread-Local Storage template
#define ELF_H_PT_LOOS 		0x60000000	// Reserved for OS-specific semantics
#define ELF_H_PT_HIOS 		0x6fffffff	// Reserved for OS-specific semantics
#define ELF_H_PT_LOPROC 	0x70000000	// Reserved for processor-specific semantics
#define ELF_H_PT_HIPROC 	0x7fffffff	// Reserved for processor-specific semantics

// 64-bit Flags
// http://www.sco.com/developers/gabi/latest/ch5.pheader.html#p_flags
#define ELF_H_64_FLAG_X		0x1 		// Segment Flag - Execute 
#define ELF_H_64_FLAG_W		0x2 		// Segment Flag - Write 
#define ELF_H_64_FLAG_R		0x4 		// Segment Flag - Read 
/********************************/
/***** PROGRAM HEADER STOP ******/
/********************************/

/********************************/
/***** SECTION HEADER START *****/
/********************************/
#define ELF_H_SHT_NULL 			0			// Inactive
#define ELF_H_SHT_PROGBITS 		1			// Holds information defined by the program
#define ELF_H_SHT_SYMTAB 		2			// Provides a symbol table for link editing
#define ELF_H_SHT_STRTAB 		3			// String table
#define ELF_H_SHT_RELA 			4			// Relocation entries with explicit addends
#define ELF_H_SHT_HASH 			5			// Holds a symbol hash table
#define ELF_H_SHT_DYNAMIC 		6			// Holds information for dynamic linking
#define ELF_H_SHT_NOTE 			7			// Holds information that marks the file in some way
#define ELF_H_SHT_NOBITS 		8			// Occupies no space but resembles SHT_PROGBITS
#define ELF_H_SHT_REL 			9			// Relocation entries without explicit addends
#define ELF_H_SHT_SHLIB 		10			// Reserved but has unspecified semantics
#define ELF_H_SHT_DYNSYM 		11			// Minimal set of dynamic linking symbols
#define ELF_H_SHT_INIT_ARRAY 	14			// Array of pointers to initilization functions
#define ELF_H_SHT_FINI_ARRAY 	15			// Array of pointers to termination functions
#define ELF_H_SHT_PREINIT_ARRAY 16			// Array of pointers to pre-initilization functions
#define ELF_H_SHT_GROUP 		17			// Defines a section group
#define ELF_H_SHT_SYMTAB_SHNDX 	18			// Associated with a symbol table section
#define ELF_H_SHT_LOOS 			0x60000000	// Reserved for OS-specific semantics
#define ELF_H_SHT_HIOS 			0x6fffffff	// Reserved for OS-specific semantics
#define ELF_H_SHT_LOPROC 		0x70000000	// Reserved for processor-specific semantics
#define ELF_H_SHT_HIPROC 		0x7fffffff	// Reserved for processor-specific semantics
#define ELF_H_SHT_LOUSER 		0x80000000	// Lower bound for range of indexes reserved for application programs
#define ELF_H_SHT_HIUSER 		0xffffffff	// Upper bound for range of indexes reserved for application programs
/********************************/
/***** SECTION HEADER STOP ******/
/********************************/

/* sectionsToPrint Flags for print_elf_details() */
#define PRINT_EVERYTHING		((unsigned int)1)			// Print everything
#define PRINT_ELF_HEADER		(((unsigned int)1) << 1)	// Print the ELF header
#define PRINT_ELF_PRGRM_HEADER	(((unsigned int)1) << 2)	// Print the Program header
#define PRINT_ELF_SECTN_HEADER	(((unsigned int)1) << 3)	// Print the Section header
#define PRINT_ELF_PRGRM_DATA	(((unsigned int)1) << 4)	// Print the Program header data
#define PRINT_ELF_SECTN_DATA	(((unsigned int)1) << 5)	// Print the Section header data 

struct Elf_Details
{
	char* fileName;				// Absolute or relative path
	char* magicNum;				// First four bytes of file
	char* elfClass;				// 32 or 64 bit
	int processorType;			// 32 or 64 bit
	char* endianness;			// Little or Big
	int bigEndian;				// If TRUE, bigEndian
	int elfVersion;				// ELF version
	char* targetOS;				// Target OS ABI
	int ABIversion;				// Version of the ABI
	char* pad;					// Unused portion
	char* type;					// The type of ELF file
	char* ISA;					// Specifies target Instruction Set Architecture
	char* objVersion;			// Object File Version
	uint32_t ePnt32;			// 32-bit memory address of the entry point from where the process starts executing
	uint64_t ePnt64;			// 64-bit memory address of the entry point from where the process starts executing
	uint32_t pHdr32;			// 32-bit address offset of the program header table
	uint64_t pHdr64;			// 64-bit address offset of the program header table
	uint32_t sHdr32;			// 32-bit address offset of the section header table
	uint64_t sHdr64;			// 64-bit address offset of the section header table
	unsigned int flags;			// Interpretation of this field depends on the target architecture
	int elfHdrSize;				// ELF Header Size
	int prgmHdrSize;			// Contains the size of a program header table entry
	int prgmHdrEntrNum;			// Number of entries in the program header table
	int sectHdrSize;			// Contains the size of a section header table entry
	int sectHdrEntrNum;			// Number of entries in the section header table
	int sectHdrSectNms;			// Index of the section header table entry with section names
};
// All char* members should be dynamically allocated and later free()'d

struct Prgrm_Hdr_Details
{
	char* fileName;				// Absolute or relative path
	char* elfClass;				// 32 or 64 bit
	int processorType;			// 32 or 64 bit
	char* endianness;			// Little or Big
	int bigEndian;				// If TRUE, bigEndian
	uint32_t pHdr32;			// 32-bit address offset of the program header table
	uint64_t pHdr64;			// 64-bit address offset of the program header table
	int prgmHdrSize;			// Contains the size of a program header table entry
	int prgmHdrEntrNum;			// Number of entries in the program header table
	// char* prgmHdrType;			// Identifies the type of the segment
	// uint32_t flags64bit;		// 64 bit flag segment
	// uint32_t seg32off;			// 32-bit offset of the segment's first byte in the file image
	// uint64_t seg64off;			// 64-bit offset of the segment's first byte in the file image
	// uint32_t seg32virAddr;		// 32-bit Virtual address of the segment in memory
	// uint64_t seg64virAddr;		// 64-bit Virtual address of the segment in memory
	// uint32_t seg32physAddr;		// 32-bit Physical address of the segment in memory
	// uint64_t seg64physAddr;		// 64-bit Physical address of the segment in memory
	// uint64_t segFileSize;		// Size in bytes of the segment in the file image
	// uint64_t segMemSize;		// Size in bytes of the segment in memory
	// uint32_t flags32bit;		// 32 bit flag segment
	// uint32_t align32bit;		// 32 bit alignment
	// uint64_t align64bit;		// 64 bit alignment
	void* segmentArray;			// Array of struct* (Prgrm_Hdr_Segment_32 or Prgrm_Hdr_Segment_64)
};
// All char* members should be dynamically allocated and later free()'d

struct Prgrm_Hdr_Segment_32
{
	uint32_t p_type;			// Segment type as number
	char* prgmHdrType;			// Identifies the type of the segment
	uint32_t flags;				// Segment flags
	uint32_t segOffset;			// Offset of the segment's first byte in the file image
	uint32_t segVirtualAddr;	// Virtual address of the segment in memory
	uint32_t segPhysicalAddr;	// Physical address of the segment in memory
	uint32_t segFileSize;		// Size in bytes of the segment in the file image
	uint32_t segMemSize;		// Size in bytes of the segment in memory
	uint32_t alignment;			// Alignment
};

struct Prgrm_Hdr_Segment_64
{
	uint32_t p_type;			// Segment type as number
	char* prgmHdrType;			// Identifies the type of the segment
	uint32_t flags;				// Segment flags
	uint64_t segOffset;			// Offset of the segment's first byte in the file image
	uint64_t segVirtualAddr;	// Virtual address of the segment in memory
	uint64_t segPhysicalAddr;	// Physical address of the segment in memory
	uint64_t segFileSize;		// Size in bytes of the segment in the file image
	uint64_t segMemSize;		// Size in bytes of the segment in memory
	uint64_t alignment;			// Alignment
};

/*************************/
/* ELF HEADER PROTOTYPES */
/*************************/
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

/*****************************/
/* PROGRAM HEADER PROTOTYPES */
/*****************************/
// Purpose: Open and parse an ELF file.  Allocate, configure and return Prgrm_Hdr_Details pointer.
// Input:	
//			[in] elvenFilename - Filename, relative or absolute, to an ELF file
//			[in] elven_file - ELF Header struct previously read from program_contents
// Output:	A dynamically allocated Prgrm_Hdr_Details struct that contains information about elvenFilename
// Note:	It is caller's responsibility to free the return value from this function by calling
//				kill_elf()
struct Prgrm_Hdr_Details* read_program_header(char* elvenFilename, struct Elf_Details* elven_file);

// Purpse:	Parse an ELF file contents into an Elf_Details struct
// Input:
//			program_struct - Struct to store elven details regarding the program header
//			program_contents - ELF file contents
//			elven_file - ELF Header struct previously read from program_contents
// Output:	ERROR_* as specified in Elf_Details.h
int parse_program_header(struct Prgrm_Hdr_Details* program_struct, char* program_contents, struct Elf_Details* elven_file);

// Purpose:	Print human-readable details about an ELF file's Program Header
// Input:
//			program_struct - A Prgrm_Hdr_Details struct that contains data about an ELF file
//			sectionsToPrint - Bitwise AND the "PRINT_*" macros into this variable
//				to control what the function actually prints.
//			stream - A stream to send the information to (e.g., stdout, A file)
// Output:	None
// Note:	This function will print the relevant data from program_struct into stream
//				based on the flags found in sectionsToPrint
void print_program_header(struct Prgrm_Hdr_Details* program_struct, unsigned int sectionsToPrint, FILE* stream);

// Purpose:	Assist clean up efforts by zeroizing/free'ing an Prgrm_Hdr_Details struct
// Input:	Pointer to an Prgrm_Hdr_Details struct pointer
// Output:	ERROR_* as specified in Elf_Details.h
// Note:	This function will modify the original variable in the calling function
int kill_program_header(struct Prgrm_Hdr_Details** old_struct);

// Purpose:	Allocate memory for an array of Program Header Segment struct pointers and assign the array pointer
// Input:	program_struct - A Prgrm_Hdr_Details struct pointer that contains data about an ELF file
// Output:	ERROR_* as specified in Elf_Details.h
// Note:
//			program_struct->segmentArray will receive a pointer to an array of struct pointers
//			The struct type segmentArray will be determined by program_struct->processorType
//			Be sure to properly type cast the void* based on the processor type
int allocate_segment_array(struct Prgrm_Hdr_Details* program_struct);

// Purpose:	Print human-readable details about an ELF file's Program Header Segments
// Input:
//			program_struct - A Prgrm_Hdr_Details struct that contains data about an ELF file
//			stream - A stream to send the information to (e.g., stdout, A file)
// Output:	None
// Note:	This function will print the relevant data from program_struct->segmentArray into stream
void print_program_header_segments(struct Prgrm_Hdr_Details* program_struct, FILE* stream)

// Purpose:	Build a HarkleDict of Program Header Type definitions
// Input:	None
// Output:	Pointer to the head node of a linked list of HarkleDicts
// Note:	Caller is responsible for utilizing destroy_a_list() to free this linked list
struct HarkleDict* init_program_header_type_dict(void);

/******************************/
/* HELPER FUNCTION PROTOTYPES */
/******************************/
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
//			Prgrm_Hdr_Details may not be NULL but *ELFstruct is expected to be NULL and will be overwritten
int read_elf_file(char* elvenFilename, struct Elf_Details** ELFstruct, struct Prgrm_Hdr_Details** PHstruct);

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



#endif // __ELF_DETAILS_H__
