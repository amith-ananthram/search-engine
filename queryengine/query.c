/*
	INPUT: query [INDEX FILE] [TARGET DIR WHERE PAGES ARE LOCATED]

	While looping, waits for KEY WORD(s)
		- words separated by " " are ANDed together
		- words separated by "OR" are ORed together
		- AND > OR (ie cat dog OR mouse = (cat AND dog) OR mouse)
		
		- entering q will break out of the loop and quit the program

	OUTPUT: Lists the top MAX_OUTPUTTED_RESULTS (in this case 10) for a
		given search (as outlined above), ordered in rank from least
		to greatest.
	
	Calculating Rank:
		For words ANDed together, rank = sum of individual occurences per page.
		
		For a certain page, if (cat AND dog) OR mouse was the query, and the 
		first QUERY had a rank of 10 for that page, and the second QUERY had
		a rank of 11, the page's overall rank is 11 (OR defaults to the larger).

	Data Structures:
		Uses: All the structures used in crawler + indexer
		
		RESULT = DocumentNode (just renamed here, same properties)
			document_id - page for the result
			page_word_frequency - rank
		
		QUERY (char* search_words[MAX_NUM_KEYWORDS])
			Each QUERY contains search words banded together
			with AND--ie just a space.  As far as user input 
			is concerned, "OR" separates instances of QUERY.
			
			e.g. 	"cat dog OR fish"
			     	- leads to 2 QUERYs
					1) search_words = ["cat", "dog"]
					2) search_words = ["fish"]

		RESULT* results[MAX_NUM_FILES] - each index corresponds to a file

	Definitions:
		MAX_NUM_KEYWORDS	- the maximum number of individual words per QUERY
					- set to 20
		MAX_NUM_QUERIES 	- the maximum number of individual QUERYs (sep. by OR)
					- set to 20
		MAX_KEYWORD_LENGTH 	- maximum length of individual words in queries
					- set to 50
		MAX_URL_LENGTH		- maximum URL length
					- set to 2096 (from crawler)
		MAX_INPUT_LENGTH	- maximum length of an input line from the terminal
					- set to 1000
		MAX_OUTPUTTED_RESULTS	- max # of results outputted 
					- set to 10
		MAX_NUM_FILES		- maximum number of files contained in [TARGET DIR]
					- set to 3000 (about 1000 more than crawled at depth 3)
		
		Besides MAX_OUTPUTTED_RESULTS, breaking any of these boundaries results 
		in a SEGFAULT.

	Pseudocode:
		1) Validates input
		2) Read index into INVERTED_INDEX data structure.
		3) Continuous while loop
			1) Separate user query into QUERYs (pullQueries)
			2) buildResult() using QUERYs
			3) sortResults()
			4) printResults()

	Explained in more detail throughout the code.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#include "query.h"
#include "queryfuncs.h"
#include "../util/header.h"
#include "../util/html.h"
#include "../util/file.h"
#include "../util/hash.h"
#include "../util/dictionary.h"

int main(int argc, char *argv[])
{
	char* program_name;
	char* index_file;
	char* target_dir;

	INVERTED_INDEX* index;				// where index_file is read into

	char* input_line;					// reads input_line

	QUERY* queries[MAX_NUM_QUERIES];	// holds QUERY structures
	int num_queries;					// corresponds to the length of queries

	RESULT results[MAX_NUM_FILES];		
	RESULT sorted_results[MAX_NUM_FILES];	
	int num_results;					// length of sorted_results

	int temp_counts[MAX_NUM_FILES];		// used for ORing 
	
	int query_return_val;				// stores the return value of pullQueries

	program_name = argv[0];

	if(argc != 3)						// if incorrect # of arguments
	{
		fprintf(stderr, "%s: Requires [INDEX FILE] [TARGET DIRECTORY] as arguments.\n", program_name);
		return -1;
	}

	index_file = argv[1];
	target_dir = argv[2];

	if(!regularFile(index_file))		// if bad index file
	{
		fprintf(stderr, "%s: Bad index file: %s", program_name, index_file);
		return -1;
	}

	if(!directoryExists(target_dir))// if [TARGET DIR] does not exist
	{
		fprintf(stderr, "%s: Directory does not exist: %s", program_name, target_dir);
		return -1;
	}

	index = readIndex(index_file);	// reads index_file into INVERTED_INDEX
	chdir(target_dir);				// changes directory to the target_dir

	while( 1 )					// continuous loop
	{
		num_queries = 0;
		num_results = 0;

		printf("KEY WORD(s): ");

		input_line = malloc(MAX_INPUT_LENGTH*sizeof(char));
		BZERO(input_line, MAX_INPUT_LENGTH);
		fgets(input_line, MAX_INPUT_LENGTH, stdin);

// ---- the 4 main functions (pullQueries, buildResults, sortResults, and ----
// ---- printResults are explained  in more detail in queryfuncs.c        ----

// pullQueries parses the input_line into QUERY structures, placing them in 
// queries and updating num_queries
		query_return_val = pullQueries(input_line, queries, &num_queries);

		free(input_line);

// pullQueries determined the input line was bad
		if(query_return_val == -1)
		{
			fprintf(stderr, "Invalid query.  Try again!\n");
			continue;
		}
// pullQueries determined the input line was the quit command ("q")
		else if(query_return_val == 1)
		{
			break;
		}

// buildResults goes through each query and each keyword contained in each 
// query it generates DocumentNodes (ie RESULTs) for each page which contains 
// any of the words, placing them into "results," using page_id as the index
// temp_counts corresponds to results, and is used to determine ORing and 
// whether or not a result exists
		buildResults(index, results, temp_counts, queries, num_queries);	

// sortResults goes through the entirety of buildResults, sorts it from 
// greatest rank to least rank and places them into sorted_results 
// (ignoring empty indices) it returns the total number of results sorted
		num_results = sortResults(results, temp_counts, sorted_results);

// printResults outputs the sorted_results in an easily understandable fashion
		printResults(sorted_results, num_results);
		
		BZERO(results, MAX_NUM_FILES);
		BZERO(temp_counts, MAX_NUM_FILES);
		BZERO(queries, num_queries);
		BZERO(sorted_results, MAX_NUM_FILES);
	}

// frees index data structure
	cleanIndex(index);
}
