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
};

int main(void)
{
	/* LOCAL VARIABLES */
	char buff[BUFF_SIZE + 1] = { 0 };			// Reusable buffer
	// Buffer that contains UINT_MAX, UINT_MAX + 1, and UINT_MIN for all endianness
	char uintMax[] = { "FFFFFFFFFFFFFFFFFF0000000000001000000000000FFFFFFFFFFFFFFFF" };	
	unsigned int i = 0;							// Iterating variable
	struct cctiTestGroup** tstGrpArr = NULL;	// Array of test group pointers
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
	struct cctiTestGroup NormalUnitTests = { "Normal Unit Tests", &Normal1 };

	// ERROR
	//// Error1 - NULL buff
	struct cctiTest Error1 = { "Error1", NULL, 1, 3, TRUE, DEFAULT_INT, ERROR_NULL_PTR, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Error2 - negative dataOffset
	struct cctiTest Error2 = { "Error2", buff, -455, 3, FALSE, DEFAULT_INT, ERROR_BAD_ARG, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Error3 - negative numBytesToConvert
	struct cctiTest Error3 = { "Error3", buff, 126, -455, TRUE, DEFAULT_INT, ERROR_BAD_ARG, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Error4 - zero numBytesToConvert
	struct cctiTest Error4 = { "Error4", buff, 126, 0, FALSE, DEFAULT_INT, ERROR_BAD_ARG, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Error5 - invalid bigEndian
	struct cctiTest Error5a = { "Error5a", buff, 1, 3, TRUE + 1, DEFAULT_INT, ERROR_BAD_ARG, DEFAULT_UINT, DEFAULT_UINT, NULL };
	struct cctiTest Error5b = { "Error5b", buff, 1, 3, FALSE - 1, DEFAULT_INT, ERROR_BAD_ARG, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Error6 - Value too large for an unsigned int
	struct cctiTest Error6 = { "Error6", buff, 120, 10, TRUE, DEFAULT_INT, ERROR_OVERFLOW, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Link Tests
	Error1.nextTest = &Error2;
	Error2.nextTest = &Error3;
	Error3.nextTest = &Error4;
	Error4.nextTest = &Error5a;
	Error5a.nextTest = &Error5b;
	Error5b.nextTest = &Error6;
	//// Create Test Group
	struct cctiTestGroup ErrorUnitTests = { "Error Unit Tests", &Error1 };

	// BOUNDARY
	//// Boundary1 - Normal input, start of buff (pass)
	struct cctiTest Boundary1 = { "Boundary1", buff, 0, 5, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x0001020304, NULL };
	//// Boundary2 - Normal input, end of buff (pass)
	struct cctiTest Boundary2 = { "Boundary2", buff, BUFF_SIZE, 2, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x7F7E, NULL };
	//// Boundary3 - Normal input, one char to convert (pass)
	struct cctiTest Boundary3a = { "Boundary3a", buff, 13, 1, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x0D, NULL };
	struct cctiTest Boundary3b = { "Boundary3b", buff, 13, 1, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x0D, NULL };
	//// Boundary4 - Value equates to UINT_MAX (pass)
	//// Boundary5 - Value equates to UINT_MAX + 1 (fail)
	//// Boundary6 - Value equates to UINT_MIN (pass)
	//// Boundary7 - numBytesToConvert == 8 + 1 (fail)
	//// Boundary8 - numBytesToConvert == 8 (pass)

	// SPECIAL
	//// Special1 - Start of buff, numBytes > 1, bigEndian == FALSE (fail)
	//// Special2 - End of buff, numBytes > 1, bigEndian == TRUE (fail)

	// Run this test separate in order to avoid redefining the struct
	// Pass NULL as the unsigned int* in the function call
	//// Error6 - NULL translation
	struct cctiTest Separate1 = { "Separate1", buff, 1, 3, FALSE, DEFAULT_INT, ERROR_NULL_PTR, DEFAULT_UINT, DEFAULT_UINT, NULL };

	// ARRAY OF TEST GROUPS
	struct cctiTestGroup* arrayOfTests[] = { &NormalUnitTests, &ErrorUnitTests, NULL };

	/* RUN THE TESTS */
	tstGrpArr = arrayOfTests;
	currTstGrp = *tstGrpArr;

	while (currTstGrp)
	{
		printf("Running '%s'...\n", currTstGrp->testGroupName);
		currTst = currTstGrp->headNode;

		while(currTst)
		{
			// Header
			printf("\tTest %s:\n", currTst->testName);
			// Function call
			currTst->actualResult = convert_char_to_int(currTst->inputBuffer, currTst->dataOffset, \
				                                        currTst->numBytes, currTst->bigEndian, \
				                                        &(currTst->actualVal));

			// Test return value
			printf("\t\tReturn:\t\t");
			numTests++;
			if (currTst->actualResult == currTst->expectedResult)
			{
				printf("Pass\n");
				numPass++;
			}
			else
			{
				printf("FAIL\n");
				printf("\t\t\tExpected:\t%d\n", currTst->expectedResult);
				printf("\t\t\tReceived:\t%d\n", currTst->actualResult);
			}

			// Test calculated value
			printf("\t\tConversion:\t");
			numTests++;
			if (currTst->actualVal == currTst->expectedVal)
			{
				printf("Pass\n");
				numPass++;
			}
			else
			{
				printf("FAIL\n");
				printf("\t\t\tExpected:\t%d\n", currTst->expectedVal);
				printf("\t\t\tReceived:\t%d\n", currTst->actualVal);
			}

			// Next test
			currTst = currTst->nextTest;
		}

		// Next test group
		tstGrpArr++;
		currTstGrp = *tstGrpArr;
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
