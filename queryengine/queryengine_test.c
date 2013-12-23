/* Filename: Test cases for query.h/.c

   IMPORTANT NOTE: This test battery requires ../index/index.dat to exist,
   as it reads it in for testing purpose.  This file should have been included
   in this location in the tarball.  It takes 10 seconds at the beginning
   to read the file into the appropriate data structure.

   Test Harness Spec:
   ------------------

   It tests the following functions, which can all be found in query.h/.c:

   	int pullQueries(char* input_line);
	void buildResults(INVERTED_INDEX* index, RESULT* results, int* temp_counts);
	int sortResults(RESULT* results, int* temp_counts, RESULT* sorted_results);
	void printResults(RESULT* sorted_results, int num_results);	

   It depends on the following functions which are defined elsewhere and not tested here:
	
   It goes through several test cases for each function, and prints its status if it fails.
   If all the tests succeed, it prints its success status.

   Test Cases:
   -----------

   int pullQueries(char* input_line);

   Test case: pullQueries:1
   This test case calls pullQueries() for the condition where input_line is an empty string.

   Test case: pullQueries:2
   This test case calls pullQueries() for the condition where input_line is a bad query,
   (e.g. "cat dog OR").

   Test case: pullQueries:3
   This test case calls pullQueries() for the condition where input_line is "q" (the exit command).

   Test case: pullQueries:4
   This test case calls pullQueries() for the condition of a simple query,
   but in the case that it is capitalized.

   Test case: pullQueries:5
   This test case calls pullQueries() for the condition of a simple query (one word).   

   Test case: pullQueries:6
   This test case calls pullQueries() for the condition where input_line is two words (an AND query).

   Test case: pullQueries:7
   This test case calls pullQueries() for the condition where input_line is two words 
   joined by "OR" (an OR query).

   Test case: pullQueries:8
   This test case calls pullQueries() for the condition where input_line should create
   3 separate QUERY structures (ie at least 2 ORs and multiple ANDs).

   -----

   void buildResults(INVERTED_INDEX* index, RESULT* results, int* temp_counts);

   Test case: buildResults:1
   This test case calls buildResults() for keywords that don't exist in index.
   
   Test case: buildResults:2
   This test case calls buildResults() for a keyword which should return results.

   -----

   int sortResults(RESULT* results, int* temp_counts, RESULT* sorted_results);

   Test case: sortResults:1
   This test case calls sortResults() in the case where results is unordered.

   -----

   void printResults(RESULT* sorted_results, int num_results);

   A simple print function.  No necessary test scenarios for it as the previous test
   cases control for what it's passed.  In general, that's why the number of test cases 
   went down (cause the possible scenarios for buildResults, sortResults, and printResults
   are controlled by the output of pullQueries, which handles most of user input).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"
#include "queryfuncs.h"
#include "../util/header.h"

// -----------------
//      MACROS
// -----------------

// Taken from the lecture notes.

// each test should start by setting the result count to zero

#define START_TEST_CASE  int rs=0

// check a condition and if false print the test condition failed
// e.g., SHOULD_BE(dict->start == NULL)

#define SHOULD_BE(x) if (!(x))  {rs=rs+1; \
    printf("Line %d Fails\n", __LINE__); \
  }

// return the result count at the end of a test

#define END_TEST_CASE return rs

//
// general macro for running a best
// e.g., RUN_TEST(TestDAdd1, "DAdd Test case 1");
// translates to:
// if (!TestDAdd1()) {
//     printf("Test %s passed\n","DAdd Test case 1");
// } else { 
//     printf("Test %s failed\n", "DAdd Test case 1");
//     cnt = cnt +1;
// }
//

#define RUN_TEST(x, y) if (!x()) {              \
    printf("Test %s passed\n", y);              \
} else {                                        \
    printf("Test %s failed\n", y);              \
    cnt = cnt + 1;                              \
}

// -----------------
//    TEST CASES
// -----------------
    
// Test case: pullQueries:1
// This test case calls pullQueries() for the condition where input_line is an empty string.

INVERTED_INDEX* index; 

int pullQueries1()
{
	START_TEST_CASE;

	int return_val;		

	char* input_line = "\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);

	SHOULD_BE(return_val == -1);

        END_TEST_CASE;
}

// Test case: pullQueries:2
// This test case calls pullQueries() for the condition where input_line is a bad query,
// (e.g. "cat dog OR").

int pullQueries2()
{
	START_TEST_CASE;
	
	int return_val;
	char* input_line = "cat dog OR\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);
	SHOULD_BE(return_val == -1);
	END_TEST_CASE;
}

// Test case: pullQueries:3
// This test case calls pullQueries() for the condition where input_line is "q" (the exit command).

int pullQueries3()
{
	START_TEST_CASE;
	
	int return_val;
	char* input_line = "q\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);
	SHOULD_BE(return_val == 1);
	END_TEST_CASE;
}

// Test case: pullQueries:4
// This test case calls pullQueries() for the condition of a simple query,
// but in the case that it is capitalized.

int pullQueries4()
{
	START_TEST_CASE;

	int return_val;
	char* input_line = "CAT\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);
	
	SHOULD_BE(return_val == 0);
	SHOULD_BE(num_queries == 1);
	SHOULD_BE(strcmp((queries[0]->search_words)[0], "cat") == 0);
	SHOULD_BE((queries[0]->search_words)[1] == NULL);
	
	free(queries[0]->search_words[0]);
	free(queries[0]);
	END_TEST_CASE;
}

// Test case: pullQueries:5
// This test case calls pullQueries() for the condition of a simple query (one word).

int pullQueries5()
{
	START_TEST_CASE;

	int return_val;
	char* input_line = "cat\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);
	
	SHOULD_BE(return_val == 0);
	SHOULD_BE(num_queries == 1);
	SHOULD_BE(strcmp((queries[0]->search_words)[0], "cat") == 0);
	SHOULD_BE((queries[0]->search_words)[1] == NULL);
	
	free(queries[0]->search_words[0]);
	free(queries[0]);
	END_TEST_CASE;
}

// Test case: pullQueries:6
// This test case calls pullQueries() for the condition where input_line is two words (an AND query).

int pullQueries6()
{
	START_TEST_CASE;
	
	int return_val;
	char* input_line = "cat dog\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);
	
	SHOULD_BE(return_val == 0);
	SHOULD_BE(num_queries == 1);
	SHOULD_BE(strcmp((queries[0]->search_words)[0], "cat") == 0);
	SHOULD_BE(strcmp((queries[0]->search_words)[1], "dog") == 0);
	SHOULD_BE((queries[0]->search_words)[2] == NULL);
	
	free(queries[0]->search_words[0]);
	free(queries[0]->search_words[1]);
	free(queries[0]);
	END_TEST_CASE;
}

// Test case: pullQueries:7
// This test case calls pullQueries() for the condition where input_line is two words 
// joined by "OR" (an OR query).

int pullQueries7()
{
	START_TEST_CASE;
	
	int return_val;
	char* input_line = "cat OR dog\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);
	
	SHOULD_BE(return_val == 0);
	SHOULD_BE(num_queries == 2);
	SHOULD_BE(strcmp((queries[0]->search_words)[0], "cat") == 0);
	SHOULD_BE(strcmp((queries[1]->search_words)[0], "dog") == 0);
	SHOULD_BE((queries[0]->search_words)[1] == NULL);
	SHOULD_BE((queries[1]->search_words)[1] == NULL);
	
	free(queries[0]->search_words[0]);
	free(queries[0]);
	free(queries[1]->search_words[0]);
	free(queries[1]);
	END_TEST_CASE;
}

// Test case: pullQueries:8
// This test case calls pullQueries() for the condition where input_line should create
// 3 separate QUERY structures (ie at least 2 ORs and multiple ANDs).

int pullQueries8()
{
	START_TEST_CASE;
	
	int return_val;
	char* input_line="cat dog OR finkelstein OR palmer computer\n";
	int num_queries;
	QUERY* queries[MAX_NUM_QUERIES];
	
	return_val = pullQueries(input_line, queries, &num_queries);
	
	SHOULD_BE(return_val == 0);
	SHOULD_BE(num_queries == 3);
	SHOULD_BE(strcmp((queries[0]->search_words)[0], "cat") == 0);
	SHOULD_BE(strcmp((queries[0]->search_words)[1], "dog") == 0);
	SHOULD_BE((queries[0]->search_words)[2] == NULL);
	SHOULD_BE(strcmp((queries[1]->search_words)[0], "finkelstein") == 0);
	SHOULD_BE((queries[1]->search_words)[1] == NULL);
	SHOULD_BE(strcmp((queries[2]->search_words)[0], "palmer") == 0);
	SHOULD_BE(strcmp((queries[2]->search_words)[1], "computer") == 0);
	SHOULD_BE((queries[2]->search_words)[2] == NULL);

	free(queries[0]->search_words[0]);
	free(queries[0]->search_words[1]);
	free(queries[0]);
	free(queries[1]->search_words[0]);
	free(queries[1]);
	free(queries[2]->search_words[0]);
	free(queries[2]->search_words[1]);
	free(queries[2]);
	END_TEST_CASE;
}

// Test case: buildResults:1
// This test case calls buildResults() for keywords that don't exist in index.

int buildResults1()
{
	START_TEST_CASE;
	
	QUERY* queries[MAX_NUM_QUERIES];
	int num_queries;

	RESULT results[MAX_NUM_FILES];
	int temp_counts[MAX_NUM_FILES];

	BZERO(results, MAX_NUM_FILES);
	BZERO(temp_counts, MAX_NUM_FILES);

	char* input_line = "thisclearlydoesntexist OR neitherdoesthissilly\n";

	pullQueries(input_line, queries, &num_queries);

	buildResults(index, results, temp_counts, queries, num_queries);

	for(int i=0; i < MAX_NUM_FILES; i++)
		SHOULD_BE(temp_counts[i] == 0);

	END_TEST_CASE;
}

// Test case: buildResults:2
// This test case calls buildResults() for a keyword which should return results.

int buildResults2()
{
	START_TEST_CASE;
	
	QUERY* queries[MAX_NUM_QUERIES];
	int num_queries;

	RESULT results[MAX_NUM_FILES];
	int temp_counts[MAX_NUM_FILES];
	int flag = 0;

	BZERO(results, MAX_NUM_FILES);
	BZERO(temp_counts, MAX_NUM_FILES);

	char* input_line = "dartmouth\n";

	pullQueries(input_line, queries, &num_queries);

	buildResults(index, results, temp_counts, queries, num_queries);

	for(int i=0; i < MAX_NUM_FILES; i++)
		if(temp_counts[i] != 0)
			flag = 1;

	SHOULD_BE(flag == 1);

	END_TEST_CASE;
}


// Test case: sortResults:1
// This test case calls sortResults() in the case where results is unordered.

int sortResults1()
{
	START_TEST_CASE;
	
	QUERY* queries[MAX_NUM_QUERIES];
	int num_queries;

	RESULT results[MAX_NUM_FILES];
	int temp_counts[MAX_NUM_FILES];

	RESULT sorted_results[MAX_NUM_FILES];
	int num_results;

	int flag = 1;

	char* input_line = "cat\n";

	BZERO(results, MAX_NUM_FILES);
	BZERO(temp_counts, MAX_NUM_FILES);

	pullQueries(input_line, queries, &num_queries);

	buildResults(index, results, temp_counts, queries, num_queries);
	num_results = sortResults(results, temp_counts, sorted_results);

	for(int i=0; i < num_results-1; i++)
		if(sorted_results[i].page_word_frequency < sorted_results[i+1].page_word_frequency)
			flag = 0;

	SHOULD_BE(flag == 1);

	END_TEST_CASE;
}

int main(int argc, char** argv) 
{
  	int cnt = 0;

	index = readIndex("../crawler/data/index.dat");

  	RUN_TEST(pullQueries1, "Pull Queries case 1");
  	RUN_TEST(pullQueries2, "Pull Queries case 2");
  	RUN_TEST(pullQueries3, "Pull Queries case 3");
  	RUN_TEST(pullQueries4, "Pull Queries case 4");
	RUN_TEST(pullQueries5, "Pull Queries case 5");
	RUN_TEST(pullQueries6, "Pull Queries case 6");
	RUN_TEST(pullQueries7, "Pull Queries case 7");
	RUN_TEST(pullQueries8, "Pull Queries case 8");

	RUN_TEST(buildResults1, "Build Results case 1");
	RUN_TEST(buildResults2, "Build Results case 2");

	RUN_TEST(sortResults1, "Sort Results case 1");

	cleanIndex(index);

  	if (!cnt) 
	{
    		printf("All passed!\n"); return 0;
  	} 
	else 
	{
    		printf("Some fails!\n"); return 1;
  	}
}
