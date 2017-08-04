#include "../Elf_Details.h"
// #include <assert.h>
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
	// Buffer that contains UINT_MAX, UINT_MAX + 1, and UINT_MIN for all endianness...es(sp?)
	// "We've always been each other's greatest nemesises... uh, nemesee... What's the plural on that?""
	// -Captain Amazing (Mystery Men)
	char uintMax[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, \
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
					   0x01, \
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
					   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
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

	// DEBUGGING PRINT STATEMENTS
	// DISCLAIMER: No Debuggers or IDEs were harmed during the debugging of this source file
	// printf("Buff[BUFF_SIZE - 2] == %d(0x%X)\n", (*(buff + BUFF_SIZE - 2)), (*(buff + BUFF_SIZE - 2)));  // DEBUGGING
	// printf("Buff[BUFF_SIZE - 1] == %d(0x%X)\n", (*(buff + BUFF_SIZE - 1)), (*(buff + BUFF_SIZE - 1)));  // DEBUGGING
	// printf("uintMax[4] == %d(0x%X)\n", (*(uintMax + 4)), (*(uintMax + 4)));  // DEBUGGING
	// printf("uintMax[5] == %d(0x%X)\n", (*(uintMax + 5)), (*(uintMax + 5)));  // DEBUGGING
	// printf("uintMax[24] == %c(0x%02X)\n", (*(uintMax + 24)), (*(uintMax + 24)));  // DEBUGGING
	// printf("uintMax[25] == %c(0x%02X)\n", (*(uintMax + 25)), (*(uintMax + 25)));  // DEBUGGING
	// printf("uintMax[26] == %c(0x%02X)\n", (*(uintMax + 26)), (*(uintMax + 26)));  // DEBUGGING
	// printf("uintMax[27] == %c(0x%02X)\n", (*(uintMax + 27)), (*(uintMax + 27)));  // DEBUGGING
	// printf("uintMax[28] == %c(0x%02X)\n", (*(uintMax + 28)), (*(uintMax + 28)));  // DEBUGGING

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
	struct cctiTest Boundary1 = { "Boundary1", buff, 0, 4, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x00010203, NULL };
	//// Boundary2 - Normal input, end of buff (pass)
	struct cctiTest Boundary2 = { "Boundary2", buff, BUFF_SIZE - 2, 2, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x7F7E, NULL };
	//// Boundary3 - Normal input, one char to convert (pass)
	struct cctiTest Boundary3a = { "Boundary3a", buff, 13, 1, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x0D, NULL };
	struct cctiTest Boundary3b = { "Boundary3b", buff, 13, 1, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x0D, NULL };
	//// Boundary4 - Value equates to UINT_MAX (pass)
	struct cctiTest Boundary4a = { "Boundary4a", uintMax, 4, 4, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0xFFFFFFFF, NULL };
	struct cctiTest Boundary4b = { "Boundary4b", uintMax, 6, 4, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0xFFFFFFFF, NULL };
	//// Boundary5 - Value equates to UINT_MAX + 1 (fail)
	struct cctiTest Boundary5a = { "Boundary5a", uintMax, 9, 5, TRUE, DEFAULT_INT, ERROR_OVERFLOW, DEFAULT_UINT, DEFAULT_UINT, NULL };
	struct cctiTest Boundary5b = { "Boundary5b", uintMax, 19, 5, FALSE, DEFAULT_INT, ERROR_OVERFLOW, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Boundary6 - Value equates to UINT_MIN (pass)
	struct cctiTest Boundary6a = { "Boundary6a", uintMax, 19, 4, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x00000000, NULL };
	struct cctiTest Boundary6b = { "Boundary6b", uintMax, 24, 4, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x00000000, NULL };
	//// Boundary7 - numBytesToConvert == 4 + 1 (fail)
	struct cctiTest Boundary7a = { "Boundary7a", buff, 1, 5, TRUE, DEFAULT_INT, ERROR_OVERFLOW, DEFAULT_UINT, DEFAULT_UINT, NULL };
	struct cctiTest Boundary7b = { "Boundary7b", buff, 5, 5, FALSE, DEFAULT_INT, ERROR_OVERFLOW, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Boundary8 - numBytesToConvert == 4 (pass)
	struct cctiTest Boundary8a = { "Boundary8a", buff, 10, 4, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x0A0B0C0D, NULL };
	struct cctiTest Boundary8b = { "Boundary8b", buff, 17, 4, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x14131211, NULL };
	//// Link Tests
	Boundary1.nextTest = &Boundary2;
	Boundary2.nextTest = &Boundary3a;
	Boundary3a.nextTest = &Boundary3b;
	Boundary3b.nextTest = &Boundary4a;
	Boundary4a.nextTest = &Boundary4b;
	Boundary4b.nextTest = &Boundary5a;
	Boundary5a.nextTest = &Boundary5b;
	Boundary5b.nextTest = &Boundary6a;
	Boundary6a.nextTest = &Boundary6b;
	Boundary6b.nextTest = &Boundary7a;
	Boundary7a.nextTest = &Boundary7b;
	Boundary7b.nextTest = &Boundary8a;
	Boundary8a.nextTest = &Boundary8b;
	//// Create Test Group
	struct cctiTestGroup BoundaryUnitTests = { "Boundary Unit Tests", &Boundary1 };

	// SPECIAL
	//// Special1 - Start of buff, numBytes > 1, bigEndian == FALSE (pass)
	struct cctiTest Special1 = { "Special1", buff, 0, 2, FALSE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x0100, NULL };
	//// Special2 - End of buff, numBytes > 1, bigEndian == TRUE (pass)... Reads the nul terminator
	struct cctiTest Special2 = { "Special2", buff, BUFF_SIZE - 1, 2, TRUE, DEFAULT_INT, ERROR_SUCCESS, DEFAULT_UINT, 0x7F00, NULL };
	//// Special3 - numBytesToConvert > 8 but value < UINT_MAX (fail)
	struct cctiTest Special3a = { "Special3a", buff, 0, 5, TRUE, DEFAULT_INT, ERROR_OVERFLOW, DEFAULT_UINT, DEFAULT_UINT, NULL };
	struct cctiTest Special3b = { "Special3b", buff, 128, 5, FALSE, DEFAULT_INT, ERROR_OVERFLOW, DEFAULT_UINT, DEFAULT_UINT, NULL };
	//// Link Tests
	Special1.nextTest = &Special2;
	Special2.nextTest = &Special3a;
	Special3a.nextTest = &Special3b;
	//// Create Test Group
	struct cctiTestGroup SpecialUnitTests = { "Special Unit Tests", &Special1 };

	// Run this test separate in order to avoid redefining the struct
	// Pass NULL as the unsigned int* in the function call
	//// Error6 - NULL translation
	struct cctiTest Separate1 = { "Separate1", buff, 1, 3, FALSE, DEFAULT_INT, ERROR_NULL_PTR, DEFAULT_UINT, DEFAULT_UINT, NULL };

	// ARRAY OF TEST GROUPS
	struct cctiTestGroup* arrayOfTests[] = { &NormalUnitTests, &ErrorUnitTests, \
		                                     &BoundaryUnitTests, &SpecialUnitTests, \
		                                     NULL };

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
				printf("\t\t\tExpected:\t%u\n", currTst->expectedVal);
				printf("\t\t\tReceived:\t%u\n", currTst->actualVal);
			}

			// Next test
			currTst = currTst->nextTest;
		}

		// Next test group
		tstGrpArr++;
		currTstGrp = *tstGrpArr;
	}

	// SEPARATE TEST
	currTst = &Separate1;
	printf("Running '%s'...\n", "Separate Unit Tests");
	while(currTst)
	{
		// Header
		printf("\tTest %s:\n", currTst->testName);
		// Function call
		currTst->actualResult = convert_char_to_int(currTst->inputBuffer, currTst->dataOffset, \
			                                        currTst->numBytes, currTst->bigEndian, \
			                                        NULL);

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
			printf("\t\t\tExpected:\t%u\n", currTst->expectedVal);
			printf("\t\t\tReceived:\t%u\n", currTst->actualVal);
		}

		// Next test
		currTst = currTst->nextTest;
	}

	/* PRINT TEST RESULTS */
	putchar('\n');
	print_fancy_header(stdout, "    UNIT TEST RESULTS    ", HEADER_DELIM);
	printf("Total Pass:\t\t%d\n", numPass);
	printf("Total Tests:\t\t%d\n", numTests);
	if ((100 * numPass) % numTests)
	{
		printf("Percent Tests Passed:\t%.1f%%\n\n", (float)numPass / numTests * 100);
	}
	else
	{
		printf("Percent Tests Passed:\t%.0f%%\n\n", (float)numPass / numTests * 100);
	}

	return 0;
}
