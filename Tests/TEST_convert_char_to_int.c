#include "../Elf_Details.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

#define TEST_NAME_SIZE	
#define BUFF_SIZE 		1024
#define DEFAULT_INT		((int)1337)
#define DEFAULT_UINT	((unsigned int)31337)

struct cctiTest
{
	char* testName;
	char* inputBuffer;
	int dataOffset;
	int numBytes;
	int bigEndian;
	int actualResult;
	int expectedResult;
	unsigned int actualVal;
	unsigned int expectedVal;	// NOTE: UINT_MAX == 4294967295 == 0xFFFFFFFF
	struct cctiTest* nextTest;
};

struct cctiTestGroup
{
	char* testGroupName;
	struct cctiTest* headNode;
}

int main(void)
{
	/* LOCAL VARIABLES */
	char buff[BUFF_SIZE + 1] = { 0 };			// Reusable buffer
	unsigned int i = 0;							// Iterating variable
	struct cctiTestGroup* currTstGrp = NULL;	// Current test group pointer
	struct cctiTest* currTst = NULL;			// Current test
	int numTests = 0;							// Total number of tests
	int numPass = 0;							// Number of tests that passed

	/* SETUP BUFF */
	for (i = 0; i < BUFF_SIZE; i++)
	{
		(*(buff + i)) = i % (CHAR_MAX + 1);
		// printf("%c(%d)", (*(buff + i)), (*(buff + i)));  // DEBUGGING
	}
	putchar('\n');
	// print_it(buff, BUFF_SIZE);  // DEBUGGING

	/* SETUP UNIT TEST GROUPS */
	// NORMAL
	//// Normal1 - bigEndian sequential
	struct cctiTest Normal1 = { "Normal1", buff, 1, 3, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x010203, NULL };
	//// Normal2 - littleEndian sequential
	struct cctiTest Normal2 = { "Normal2", buff, 1, 3, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x030201, NULL };
	//// Normal3 - bigEndian cross barrier (126, 127, 0, 1...)
	struct cctiTest Normal3 = { "Normal3", buff, 126, 4, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x7E7F0001, NULL };
	//// Normal4 - littleEndian cross barrier (126, 127, 0, 1...)
	struct cctiTest Normal4 = { "Normal4", buff, 126, 4, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x01007F7E, NULL };
	//// Link Tests
	Normal1.nextTest = &Normal2;
	Normal2.nextTest = &Normal3;
	Normal3.nextTest = &Normal4;
	//// Create Test Group
	struct cctiTestGroup NormalUnitTests = { "Normal Unit Tests", &cctiTest };

	// ERROR
	//// Error1 - NULL buff
	//// Error2 - negative dataOffset
	//// Error3 - negative numBytesToConvert
	//// Error4 - zero numBytesToConvert
	//// Error5 - invalid bigEndian
	//// Error6 - NULL translation
	//// Error7 - Value too large for an unsigned int

	// BOUNDARY
	//// Boundary1 - Normal input, start of buff (pass)
	//// Boundary2 - Normal input, end of buff (pass)
	//// Boundary3 - Normal input, one char to convert (pass)
	//// Boundary4 - Value equates to UINT_MAX (pass)
	//// Boundary5 - Value equates to UINT_MAX + 1 (fail)
	//// Boundary6 - Value equates to UINT_MIN (pass)
	//// Boundary7 - numBytesToConvert == 8 + 1 (fail)
	//// Boundary8 - numBytesToConvert == 8 (pass)

	// SPECIAL
	//// Verify translation value is not zeroized on Error input
	////// Special1 - NULL buff
	////// Special2 - NULL translation
	////// Special3 - negative dataOffset
	////// Special4 - negative numBytesToConvert
	////// Special5 - zero numBytesToConvert
	////// Special6 - invalid bigEndian
	////// Special7 - Value too large for an unsigned int
	////// Special8 - zero numBytesToConvert
	////// Special9 - Value equates to UINT_MAX + 1

	// ARRAY OF TEST GROUPS
	struct cctiTestGroup* arrayOfTests[] = { &NormalUnitTests, NULL };

	/* RUN THE TESTS */
	currTstGrp = *arrayOfTests;

	while (currTstGrp)
	{
		printf("Running '%s'...\n", currTstGrp->testGroupName);
		currTst = currTstGrp->headNode;

		while(currTst)
		{
			// Header
			printf("Test %s:\n", currTst->testName);
			// Function call
			currTst->actualResult = convert_char_to_int(currTst->inputBuffer, currTst->dataOffset, \
				                                        currTst->numBytes, currTst->bigEndian, \
				                                        &(currTst->actualVal));

			// Test return value
			printf("\tReturn:\t");
			numTests++;
			if (currTst->actualResult == currTst->expectedResult)
			{
				printf("Pass\n");
				numPass++;
			}
			else
			{
				printf("FAIL\n");
				print("\t\tExpected:\t%d\n", currTst->expectedResult);
				print("\t\tReceived:\t%d\n", currTst->actualResult);
			}

			// Test calculated value
			printf("\tConversion:\t")
			numTests++;
			if (currTst->actualVal == currTst->expectedVal)
			{
				printf("Pass\n");
				numPass++;
			}
			else
			{
				printf("FAIL\n");
				print("\t\tExpected:\t%d\n", currTst->expectedVal);
				print("\t\tReceived:\t%d\n", currTst->actualVal);
			}

			// Next test
			currTst = currTst->nextTest;
		}

		// Next test group
		currTstGrp = *(arrayOfTests++);
	}

	return 0;
}
/*
#define ERROR_SUCCESS	((int)0)	// EUREKA!
#define ERROR_NULL_PTR	((int)-1)	// NULL pointer
#define ERROR_BAD_ARG	((int)-2)	// Bad arguments
#define ERROR_ORC_FILE	((int)-3)	// Indicates this is not an ELF file
#define ERROR_OVERFLOW	((int)-4)	// The given data type will overflow
*/
// int convert_char_to_int(char* buffToConvert, int dataOffset, \
// 	                    int numBytesToConvert, int bigEndian, \
// 	                    unsigned int* translation);
// Purpose:	Convert consecutive characters into a single int IAW the specified endianess
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
// int convert_char_to_int(char* buffToConvert, int dataOffset, \
// 	                    int numBytesToConvert, int bigEndian, \
// 	                    unsigned int* translation);