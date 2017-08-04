#include "../Elf_Details.h"
#include <inttypes.h>	// Print uint64_t variables
#include <stdio.h>		// I/O

#define DEFAULT_INT		((int)1337)
#define DEFAULT_UINT32	((uint32_t)31337)
#define UINT32_MAXIMUM	0xFFFFFFFF
#define DEFAULT_UINT64	((uint64_t)0x0000000007041776)
#define UINT64_MAXIMUM	((uint64_t)0xFFFFFFFFFFFFFFFF)


typedef struct cu64tu32Test
{
	char* testName;
	int actualResult;
	int expectedResult;
	uint64_t inputVal;
	uint32_t outputVal;
	uint32_t expectedVal;
	struct cu64tu32Test* nextTest;
} unitTest;


typedef struct cctiTestGroup
{
	char* testGroupName;
	unitTest* headNode;
} unitTestGroup;


int main(void)
{
	/* LOCAL VARIABLES */
	unitTestGroup** tstGrpArr = NULL;	// Array of test group pointers
	unitTestGroup* currTstGrp = NULL;	// Current test group pointer
	unitTest* currTst = NULL;			// Current test
	int numTests = 0;					// Total number of tests
	int numPass = 0;					// Total number of tests that passed

	/* SETUP UNIT TEST GROUPS */
	// NORMAL
	unitTest Normal1 = { "Normal1", DEFAULT_INT, ERROR_SUCCESS, 0x0000000001020304, DEFAULT_UINT32, 0x01020304, NULL };
	unitTest Normal2 = { "Normal2", DEFAULT_INT, ERROR_SUCCESS, 0x000000000B10BBED, DEFAULT_UINT32, 0x0B10BBED, NULL };
	unitTest Normal3 = { "Normal3", DEFAULT_INT, ERROR_SUCCESS, 0x000000000EA7BEEF, DEFAULT_UINT32, 0x0EA7BEEF, NULL };
	unitTest Normal4 = { "Normal4", DEFAULT_INT, ERROR_SUCCESS, 0x0000000008ADC0DE, DEFAULT_UINT32, 0x08ADC0DE, NULL };
	//// Link Tests
	Normal1.nextTest = &Normal2;
	Normal2.nextTest = &Normal3;
	Normal3.nextTest = &Normal4;
	//// Create Test Group
	unitTestGroup NormalUnitTests = { "Normal Unit Tests", &Normal1 };

	// ERROR
	//// Error1 - inVal == UINT64_MAXIMUM
	unitTest Error1 = { "Error1", DEFAULT_INT, ERROR_OVERFLOW, UINT64_MAXIMUM, DEFAULT_UINT32, DEFAULT_UINT32, NULL };
	//// Link Tests
	// Error1.nextTest = &Error2;
	//// Create Test Group
	unitTestGroup ErrorUnitTests = { "Error Unit Tests", &Error1 };

	// BOUNDARY
	//// Boundary1 - inVal == UINT32_MAXIMUM + 1 (fail)
	unitTest Boundary1 = { "Boundary1", DEFAULT_INT, ERROR_OVERFLOW, UINT32_MAXIMUM + 1, DEFAULT_UINT32, DEFAULT_UINT32, NULL };
	//// Boundary2 - inVal == UINT32_MAXIMUM (pass)
	unitTest Boundary2 = { "Boundary2", DEFAULT_INT, ERROR_SUCCESS, UINT32_MAXIMUM, DEFAULT_UINT32, UINT32_MAXIMUM, NULL };
	//// Boundary3 - inVal == 0 (pass)
	unitTest Boundary3 = { "Boundary3", DEFAULT_INT, ERROR_SUCCESS, 0x0000000000000000, DEFAULT_UINT32, 0x00000000, NULL };
	//// Link Tests
	Boundary1.nextTest = &Boundary2;
	Boundary2.nextTest = &Boundary3;
	//// Create Test Group
	unitTestGroup BoundaryUnitTests = { "Boundary Unit Tests", &Boundary1 };

	// SPECIAL
	// Run these tests separate in order to avoid redefining the struct
	// Pass NULL as the uint32_t* in the function call
	//// Special1 - Pass NULL as outVal;
	unitTest Special1 = { "Special1", DEFAULT_INT, ERROR_NULL_PTR, 0x0000000008ADC0DE, DEFAULT_UINT32, DEFAULT_UINT32, NULL };
	//// Link Tests
	// Special1.nextTest = &Special2;
	//// Create Test Group
	unitTestGroup SpecialUnitTests = { "Special Unit Tests", &Special1 };

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
			if (currTst == &Special1)
			{
				currTst->actualResult = convert_uint64_to_uint32(currTst->inputVal, NULL);
			}
			else
			{
				currTst->actualResult = convert_uint64_to_uint32(currTst->inputVal, &(currTst->outputVal));
			}

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
			if (currTst->outputVal == currTst->expectedVal)
			{
				printf("Pass\n");
				numPass++;
			}
			else
			{
				printf("FAIL\n");
				printf("\t\t\tExpected:\t%" PRIu32 "\n", currTst->expectedVal);
				printf("\t\t\tReceived:\t%" PRIu32 "\n", currTst->outputVal);
			}

			// Next test
			currTst = currTst->nextTest;
		}

		// Next test group
		tstGrpArr++;
		currTstGrp = *tstGrpArr;
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
