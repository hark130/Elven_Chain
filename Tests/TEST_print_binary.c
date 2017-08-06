#include "../Elf_Details.h"
#include <inttypes.h>	// Print uint64_t variables
#include <limits.h>
#include <stdio.h>		// I/O
#include <string.h>		// strncmp

#define BUFF_SIZE (((8 + 2) * 8) - 1)


typedef struct printBinaryTest
{
	char* testName;
	char* fileName;
	FILE* filePtr;
	void* inputPtr;
	size_t numBytes;
	char actualStr[BUFF_SIZE + 1];
	char* expectedStr;
	struct printBinaryTest* nextTest;
} unitTest;


typedef struct pbTestGroup
{
	char* testGroupName;
	unitTest* headNode;
} unitTestGroup;


int main(void)
{
	/* LOCAL VARIABLES */
	unitTestGroup** tstGrpArr = NULL;		// Array of test group pointers
	unitTestGroup* currTstGrp = NULL;		// Current test group pointer
	unitTest* currTst = NULL;				// Current test
	int numTests = 0;						// Total number of tests
	int numPass = 0;						// Total number of tests that passed
	size_t tmpRet = 0;						// Holds fread return value
	size_t tmpInt = 0;						// Number of bytes to read
	// Input Variables
	char inChar1 = CHAR_MIN;				// 1000 0000
	int inInt1 = INT_MIN;					// 1000 0000 0000 0000 0000 0000 0000 0000 
	char inChar2 = CHAR_MAX;				// 0111 1111
	int inInt2 = INT_MAX;					// 0111 1111 1111 1111 1111 1111 1111 1111 
	uint8_t inUint8 = 0xAA;					// 1010 1010
	uint16_t inUint16 = 0x5555;				// 0101 0101 0101 0101
	uint32_t inUint32 = 0xF0F0F0F0;			// 1111 0000 1111 0000 1111 0000 1111 0000 
	uint64_t inUint64 = 0xFF00FF00FF00FF00;	// 1111 1111 0000 0000 1111 1111 0000 0000 1111 1111 0000 0000 1111 1111 0000 0000
	unsigned char inUchar = 0xF0;			// 1111 0000
	unsigned int inUint = UINT_MAX;			// 1111 1111 1111 1111 1111 1111 1111 1111 


	/* UNIT TESTS */
	// Normal
	//// Normal1 - char
	unitTest Normal1a = { "Normal1a", "./Test_pb_Normal1a.tst", NULL, &inChar1, sizeof(inChar1), "h", "1000 0000", NULL };
	unitTest Normal1b = { "Normal1b", "./Test_pb_Normal1b.tst", NULL, &inChar2, sizeof(inChar2), "h", "0111 1111", NULL };
	//// Normal2 - int
	unitTest Normal2a = { "Normal2a", "./Test_pb_Normal2a.tst", NULL, &inInt1, sizeof(inInt1), "h", "1000 0000 0000 0000 0000 0000 0000 0000", NULL };
	unitTest Normal2b = { "Normal2b", "./Test_pb_Normal2b.tst", NULL, &inInt2, sizeof(inInt2), "h", "0111 1111 1111 1111 1111 1111 1111 1111", NULL };
	//// Normal3 - uint8_t
	unitTest Normal3 = { "Normal3", "./Test_pb_Normal3.tst", NULL, &inUint8, sizeof(inUint8), "h", "1010 1010", NULL };
	//// Normal4 - uint16_t
	unitTest Normal4 = { "Normal4", "./Test_pb_Normal4.tst", NULL, &inUint16, sizeof(inUint16), "h", "0101 0101 0101 0101", NULL };
	//// Normal5 - uint32_t
	unitTest Normal5 = { "Normal5", "./Test_pb_Normal5.tst", NULL, &inUint32, sizeof(inUint32), "h", "1111 0000 1111 0000 1111 0000 1111 0000", NULL };
	//// Normal6 - uint64_t
	unitTest Normal6 = { "Normal6", "./Test_pb_Normal6.tst", NULL, &inUint64, sizeof(inUint64), "h", "1111 1111 0000 0000 1111 1111 0000 0000 1111 1111 0000 0000 1111 1111 0000 0000", NULL };
	//// Normal7 - unsigned char
	unitTest Normal7 = { "Normal7", "./Test_pb_Normal7.tst", NULL, &inUchar, sizeof(inUchar), "h", "1111 0000", NULL };
	//// Normal8 - unsigned int
	unitTest Normal8 = { "Normal8", "./Test_pb_Normal8.tst", NULL, &inUint, sizeof(inUint), "h", "1111 1111 1111 1111 1111 1111 1111 1111", NULL };
	//// Link Tests
	Normal1a.nextTest = &Normal1b;
	Normal1b.nextTest = &Normal2a;
	Normal2a.nextTest = &Normal2b;
	Normal2b.nextTest = &Normal3;
	Normal3.nextTest = &Normal4;
	Normal4.nextTest = &Normal5;
	Normal5.nextTest = &Normal6;
	Normal6.nextTest = &Normal7;
	Normal7.nextTest = &Normal8;
	//// Create Test Group
	unitTestGroup NormalUnitTests = { "Normal Unit Tests", &Normal1a };



	// Error
	//// Error1 - NULL stream
	//// Error2 - NULL var pointer
	//// Error3 - Negative number of bytes to print
	//// Error4 - 0 number of bytes to print

	// ARRAY OF TEST GROUPS
	unitTestGroup* arrayOfTests[] = { &NormalUnitTests, \
									  NULL, };
									 // &ErrorUnitTests, \
				 					 // &BoundaryUnitTests, \
					 				 // &SpecialUnitTests, \
					 				 //	NULL };

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
			// Prepare File
			currTst->filePtr = fopen(currTst->fileName, "w");

			if (currTst->filePtr)
			{
				// Function call
				print_binary(currTst->filePtr, currTst->inputPtr, currTst->numBytes, FALSE);

				// Close the file
				fclose(currTst->filePtr);

				// Read the file
				currTst->filePtr = fopen(currTst->fileName, "r");

				if (currTst->filePtr)
				{
					// Read File Contents
					tmpInt = (currTst->numBytes * 8);
					tmpInt += ((tmpInt / 4) - 1);
					tmpRet = fread(currTst->actualStr, sizeof(char), tmpInt, currTst->filePtr);

					if (tmpRet != tmpInt)
					{
						printf("\t\tERROR READING FROM FILE... READ %zu BYTES OF %zu EXPECTED BYTES\n", tmpRet, tmpInt);
					}

					// Close the File
					fclose(currTst->filePtr);

					// Test Printed Value
					printf("\t\tOutput:\t");
					numTests++;
					// printf("\nActual String:\t%s\n", currTst->actualStr);
					// printf("\nExpected String:\t%s\n", currTst->expectedStr);
					
					if (!strncmp(currTst->expectedStr, currTst->actualStr, tmpInt))
					// if (!strncmp(currTst->expectedStr, currTst->actualStr, BUFF_SIZE))
					{
						printf("Pass\n");
						numPass++;
					}
					else
					{
						printf("FAIL\n");
						printf("\t\t\tExpected:\t%s\n", currTst->expectedStr);
						printf("\t\t\tReceived:\t%s\n", currTst->actualStr);
					    // if (!strcmp("Normal2a", currTst->testName)) { print_binary(stdout, currTst->inputPtr, currTst->numBytes); };  // DEBUGGING
					}
				}
				else
				{
					printf("\t\tTEST FAILED TO READ FILE:\t%s\n", currTst->fileName);
				}
			}
			else
			{
				printf("\t\tTEST FAILED TO RUN FOR FILE:\t%s\n", currTst->fileName);
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
