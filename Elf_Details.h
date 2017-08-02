// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format

struct Elf_Details
{
	char* fileName;		// Absolute or relative path
	char* class;		// 32 or 64 bit
	char* endianess;	// Little or Big
	int version;		// ELF version
	char* targetOS;		// Target OS ABI
	int ABIversion;		// Version of the ABI
	int type;			// The type of ELF file
}

/****************************/
/***** ELF HEADER START *****/
/****************************/
// Magic Number 0x00 - 0x03
#define ELF_H_MAGIC_NUM		"\x7f\x45\x4c\x46"	// 7F E L F
// Class 0x04
#define ELF_H_CLASS_32		"\x1"				// 1
#define ELF_H_CLASS_64		"\x2"				// 2
// Endianess 0x05
#define ELF_H_DATA_LITTLE	"\x1"				// 1
#define ELF_H_DATA_BIG		"\x2"				// 2
// Version 0x06
#define ELF_H_VERSION		"\x1"				// 1
// Target Operating System ABI 0x07
#define ELF_H_OSABI_SYSTEM_V	"\x00"			// 0
#define ELF_H_OSABI_HP_UX		"\x01"			// 1
#define ELF_H_OSABI_NETBSD  	"\x02"			// 2
#define ELF_H_OSABI_LINUX   	"\x03"			// 3
#define ELF_H_OSABI_GNU_HURD	"\x04"			// 4
#define ELF_H_OSABI_SOLARIS 	"\x06"			// 6
#define ELF_H_OSABI_AIX     	"\x07"			// 7
#define ELF_H_OSABI_IRIX    	"\x08"			// 8
#define ELF_H_OSABI_FREE_BSD	"\x09"			// 9
#define ELF_H_OSABI_TRU64   	"\x0A"			// 10
#define ELF_H_OSABI_NOVELL  	"\x0B"			// 11
#define ELF_H_OSABI_OPEN_BSD	"\x0C"			// 12
#define ELF_H_OSABI_OPEN_VMS	"\x0D"			// 13
#define ELF_H_OSABI_NONSTOP_K	"\x0E"			// 14
#define ELF_H_OSABI_AROS    	"\x0F"			// 15
#define ELF_H_OSABI_FENIX_OS	"\x10"			// 16
#define ELF_H_OSABI_CLOUB_ABI	"\x11"			// 17
#define ELF_H_OSABI_SORTIX  	"\x53"			// 83
// ABI Version 0x08
// 		No assumed values here.  int-->str or int-->int?
// Pad 0x09 - 0x0F
// 		Currently unused
// Type 0x10 - 0x11
#define ELF_H_TYPE_RELOCATABLE	"\x1"			// 1
#define ELF_H_TYPE_EXECUTABLE	"\x2"			// 2
#define ELF_H_TYPE_SHARED		"\x3"			// 3
#define ELF_H_TYPE_CORE			"\x4"			// 4
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

// Purpose:	Prints an uppercase title surrounded by delimiters
// Input:
//			stream - Stream to print the header to
//			title - Title to print
//			delimiter - Single character to create the box
// Output:	None
// Note:	Automatically sizes the box
void print_fancy_header(FILE* stream, char* title, unsigned char delimiter);