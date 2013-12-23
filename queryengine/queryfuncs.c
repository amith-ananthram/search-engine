/*
	queryfuncs.c

	Contains the functional code for query.c.

	int pullQueries   	- parses the string input_line into QUERYs
					  	- places those QUERYs into the list queries
					  	- returns the number of QUERYs parsed
	
	void buildResults 	- goes through each QUERY in query
					  	- goes through each word associted with each QUERY
					  	- pulls WordNode from index for each word
					  	- goes through each DocumentNode associated
					  	- creates a RESULT for each DocumentNode and places
					      it at its proper index (ie page_id) in results
						- uses temp_counts for OR conditions

	int sortResults   	- uses temp_counts to figure out where RESULTs are
						  in results
						- sorts the RESULTs in results by rank
						- stores the sorted RESULTs in sorted_results
						- returns the number of results sorted
	
	int printResults	- goes through sorted_results, pulls the page for
						  each RESULT and gets its url, and outputs it all

	Important Variables Explained:
	
	RESULT results[MAX_NUM_FILES]
		- each index corresponds to the page whose name bears that index value

	int temp_counts[MAX_NUM_FILES]
		- each index corresponds to a page whose name bears that index value
		- once an index contains a RESULT in results (ie a page has come up once
		  for any of the keywords in a query), temp_counts keeps track of its
		  rank for the purpose of ORing (replaces the rank in the corresponding
		  RESULT at the end if it's higher)

	RESULT sorted_results[MAX_NUM_FILES]
		- completely filled with results up to the return vaue of sortResults
		- sorted greatest to least by rank (page_word_frequency)
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

// takes a char* input_line, a QUERY** queries, and a pointer to an int num_queries
// parses input_line for QUERYs, placing them into queries, and incrementing 
// num_queries as it does so
// returns -1 if input_line is bad (empty, ends in "OR")
// returns 1 if input_line == "q" (quit command)
// returns 0 if successful
int pullQueries(char* input_line, QUERY** queries, int* num_queries)
{
	char *current_keywords[MAX_NUM_KEYWORDS];
	char *word;	
	int current_index;
	QUERY* query;
	int position;

	word = malloc(MAX_KEYWORD_LENGTH*sizeof(char)); 
	BZERO(word, MAX_KEYWORD_LENGTH*sizeof(char));
	
	*num_queries = 0;
	current_index = 0;		// corresponds to index of current_keywords
	position = 0;			// matches index in input_line

// getNextWord parses the input_line for a word, storing it into word
// works just like getNextURL
	while((position = getNextWord(input_line, word, position)) != -1)
	{	
		word[strlen(word)] = '\0';		

// if quit command
		if(current_index == 0 && strcmp(word, "q") == 0)
		{
			free(word);			
			return 1;
		}

// if OR (a new QUERY is about to begin)
		if(strcmp(word, "OR") == 0)
		{
			query = malloc(sizeof(QUERY)); 
			MALLOC_CHECK(query);

// for each keyword in current_keywords, put it into the search_words
// parameter of query
			for(int i = 0; i < current_index; i++)
			{
				query->search_words[i] = malloc(MAX_KEYWORD_LENGTH*sizeof(char));	 	
				BZERO((query->search_words)[i], MAX_KEYWORD_LENGTH*sizeof(char));
				strncpy((query->search_words)[i], current_keywords[i], MAX_KEYWORD_LENGTH*sizeof(char));
				free(current_keywords[i]);
			}

// include a null-terminator just in case
			query->search_words[current_index] = NULL;

// place the new query into queries (while incrementing num_queries)
			queries[(*num_queries)++] = query;

// empty current_keywords and reset its index current_index
			BZERO(current_keywords, MAX_NUM_KEYWORDS);
			current_index = 0;
		}

// if it's a regular keyword
		else
		{

// make it lower case
			NormalizeWord(word);		

// add it to current_keywords while incrementing its index current_index	
			current_keywords[current_index] = malloc(MAX_KEYWORD_LENGTH*sizeof(char));			
			BZERO(current_keywords[current_index], MAX_KEYWORD_LENGTH*sizeof(char));
			strncpy(current_keywords[current_index++], word, MAX_KEYWORD_LENGTH*sizeof(char));		
		}

// empty the word out
		BZERO(word, MAX_KEYWORD_LENGTH*sizeof(char));
	}

	free(word); 

// if current_index = 0, that means the last word in input_line was "OR"
// and therefore the input is bad
	if(current_index == 0)
	{
		for(int i = 0; i < *num_queries; i++)
		{
			query = queries[i];
			
			position = 0;			

			while((word = (query->search_words)[position++]) != NULL)
				free(word);

			free(query);
		}

		return -1;
	}

// otherwise, the last QUERY hasn't been created yet, and we need to make it
// in the same way
	query = malloc(sizeof(QUERY)); 
	MALLOC_CHECK(query);

	for(int i = 0; i < current_index; i++)
	{
		query->search_words[i] = malloc(MAX_KEYWORD_LENGTH*sizeof(char));				
		BZERO((query->search_words)[i], MAX_KEYWORD_LENGTH);
		strncpy((query->search_words)[i], current_keywords[i], MAX_KEYWORD_LENGTH);
		free(current_keywords[i]);
	}

	query->search_words[current_index] = NULL;

	queries[(*num_queries)++] = query;
			
	return 0;	
}

// takes a INVERTED_INDEX* index, a list of results RESULT* results, a list of 
// ints int* temp_counts, a list of QUERYs QUERY** queries, and an int
// num_queries corresponding to that list
void buildResults(INVERTED_INDEX* index, RESULT* results, int* temp_counts, QUERY** queries, int num_queries)
{
	QUERY* current_query;

	char* current_keyword;	// corresponds to a word in search_words in each QUERY
	int keyword_index;		// corresponds to index of search_words in each QUERY

	DNODE* wordnode;
	DocumentNode* docnode;
	RESULT result;

	int page_id;
	int rank;

// for each query
	for(int i = 0; i < num_queries; i++)
	{
		current_query = queries[i];

		keyword_index = 0;

// for each keyword contained in the query (search_words)
		while((current_keyword = (current_query->search_words)[keyword_index++]) != NULL)
		{

// if the word has a corresponding WordNode in the index
			if((wordnode = getData(index, current_keyword)) != NULL)
			{
				docnode = wordnode->data;

// for each DocumentNode associated with that WordNode
				while(docnode != NULL)
				{
					page_id = docnode->document_id;
					rank = docnode->page_word_frequency;

// if this doc is new (ie we haven't come across it yet)
					if(!temp_counts[page_id])
					{
// creates a RESULT for the document						
						result.document_id = page_id;
						result.page_word_frequency = rank;

// places this RESULT into the list of RESULTs result
						results[page_id] = result;

// includes its rank at its page_id index in temp_counts
						temp_counts[page_id] = rank;
					}
// if we have come across this doc already
					else
					{
// if its index in temp_counts == -1 (explained in the next for loop)
						if(temp_counts[page_id] == -1)
							temp_counts[page_id] += rank+1;
						else
							temp_counts[page_id] += rank;
					}
	
					docnode = docnode->next;
				}
			}	

			free(current_keyword);
		}

// for each page index
		for(int page_index = 0; page_index < MAX_NUM_FILES; page_index++)
		{
// if there's a count for it
			if(temp_counts[page_index])	// handles OR
			{	
// if the count is greater than what's currently there
				if(temp_counts[page_index] > (results[page_index].page_word_frequency))
					results[page_index].page_word_frequency = temp_counts[page_index];

// set to -1 so we know the page has been visited on later iterations (0 = NULL so that doesn't work)
				temp_counts[page_index] = -1;
			}
		}

		free(queries[i]);	
	}	
}

// takes a list of RESULT results, a list of ints temp_counts, and
// an empty list of RESULT called sorted_results
// returns the number of results placed into sorted_results
int sortResults(RESULT* results, int* temp_counts, RESULT* sorted_results)
{
	RESULT temp;
	int s;
	int num_results = 0;

// for each page in page_index
	for(int page_index = 0; page_index < MAX_NUM_FILES; page_index++)
	{
// if the page has been visited
		if(temp_counts[page_index])	// handles OR
		{
// set it to 0 for later queries (ie unvisited)	
			temp_counts[page_index] = 0;		
// store it in the smallest available index in sorted_results
			sorted_results[num_results] = results[page_index];
			num_results += 1;		
		}
	}

	s = 1;				// to break out of bubble sort

// bubble sorts the results by rank
	for(int i = 0; i < num_results && s != 0; i++)
	{
		s = 0;		
		
		for(int j = 0; j < num_results-1; j++)
		{
			if(sorted_results[j].page_word_frequency < sorted_results[j+1].page_word_frequency)
			{
				temp = sorted_results[j];
				sorted_results[j] = sorted_results[j+1];
				sorted_results[j+1] = temp;
				s++;
			}
		}
	}

	return num_results;
}

// takes a list of RESULT sorted_results and an int num_results
// that corresponds to the number of RESULTs in sorted_results (max index)
// prints out the corresponding URLS
void printResults(RESULT* sorted_results, int num_results)
{
	RESULT current;
	int rank;
	char page_id[10];

	FILE *fp;
	char* url;

// for each RESULT
	for(int i = 0; i < num_results; i++)
	{
		current = sorted_results[i];
		sprintf(page_id, "%d", current.document_id);
		rank = current.page_word_frequency;

// open the page associated with it
		fp = fopen(page_id, "r");

// get the first line from the page (ie the URL)
		url = malloc(MAX_URL_LENGTH*sizeof(char));
		BZERO(url, MAX_URL_LENGTH);
		fgets(url, MAX_INPUT_LENGTH, fp);

// print it out
		printf("%d:\tRANK: %d\tID:%s\tURL:%s", i, rank, page_id, url);

		fclose(fp);
		free(url);
	}	
}

