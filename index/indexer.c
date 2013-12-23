/*

  FILE: indexer.c

  Description:

  Inputs: ./indexer [TARGET DIRECTORY] [OUTPUT FILE NAME] 						-- regular functionality
	  ./indexer [TARGET DIRECTORY] [OUTPUT FILE NAME] [INPUT FILE NAME] [TEST OUTPUT FILE NAME]	-- testing

  Outputs: In the regular functionality mode, it ouputs an index [OUTPUT FILE NAME] outlining the occurences of each words contained in the documents in
	   [TARGET DIRECTORY] in the following format: "computer 2 1 6 7 10", which means the word "computer" occured in "2" documents.  Specifically, 
	   it occured in the document whose ID is "1" 6 times, and in the document whose ID is "2" 10 times.
	   In the testing mode, it does what the regular functionality does, and also reads in an index file, recreates data structures from it,
	   and outputs it once again.  This is simply to check and make sure the index file is readable by a computer (for the query engine later).

  Data Structures: An index, which is a dictionary data structure.  It contains parameters that point to the first and last node in a doubly linked list,
 		   and a hash table whose hash values point to various nodes in the linked list (for faster retrieval).
		   The nodes pointed to by the dictionary are DNODEs, but in this case, they're called WordNodes.  A WordNode has 4 properties: prev and
		   and next which point to other WordNodes, data which points to a void* (in this case a DocumentNode), and a key (which is a string).
		   DocumentNodes have 3 parameters: next, which points to another DocumentNode, document_id which is its id, and page_count_occurence
                   which refers to the number of times the word occured on that page.

*/

// ----------------------------
// ---- INCLUDE STATEMENTS ----
// ----------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#include "indexer.h"
#include "../util/header.h"
#include "../util/html.h"
#include "../util/file.h"
#include "../util/hash.h"
#include "../util/dictionary.h"

int main(int argc, char *argv[])
{
// INPUT VARIABLES
	char* program;
	char* target_dir;
	char* output_file_name;
	char* input_file_name;
	char* rewritten_file_name;	

// determines which mode the program is running in
	int indexer_test_flag;

// overall data structure
	INVERTED_INDEX* index;

// these variables handle the scandir results and pulling information from files
	int numfiles = 0;
	struct dirent **files;	
	char* file_name;
	char* file_contents;

// this variable is used for parsing the HTML
	int file_pos;

// variables used for WordNode (word == key) and DocumentNode (doc_id)
	char* word;
	int doc_id;

	indexer_test_flag = 0; // default is basic funcitonality

	program = argv[0];

// if incorrect number of arguments
	if(argc != 3 && argc != 5)
	{
		fprintf(stderr, "%s: The indexer requires either 2 (a target directory and output file name) or 4 (a target directory, output file name, input file name, and a rewritten file name\n", program);

		return 1;
	}

	target_dir = argv[1];
	output_file_name = argv[2];

// if 5 arguments --> TESTING MODE
	if(argc == 5)
	{
		indexer_test_flag = 1;
		input_file_name = argv[3];
		rewritten_file_name = argv[4];
	}

// if the target directory doesn't exist
	if(!directoryExists(target_dir))
	{
		fprintf(stderr, "%s: Invalid target directory %s\n", program, target_dir);
		return 1;
	}
	
	numfiles = getFileList(target_dir, &files);

	chdir(target_dir);

// if there are no files in the target directory
	if(numfiles <= 0)
	{
		fprintf(stderr, "%s: Error with target directory %s'n", program, target_dir);
		return 1;
	}

	index = initializeDict();

// this for loop goes through each file in "files", pulls each word out of the HTML, and updates the index data structure
	for(int i=0; i < numfiles; i++)
	{
		file_name = files[i]->d_name;

// if it's a regular file (to avoid . and .. files)
		if(regularFile(file_name))
		{
			file_contents = NULL;	
			file_contents = readFile(file_name);
			file_pos = 0;
			doc_id = atoi(file_name);

// just in case a 404 wasn't caught by the crawler
			if(file_contents != NULL)
			{
				word = NULL;
				word = malloc(500*sizeof(char));
				MALLOC_CHECK(word);
				BZERO(word, 500*sizeof(char));

// GetNextWord returns the index in file_contents where it stopped parsing, while assigning a new word to the "word"
				while((file_pos = parseHTML(file_contents, word, file_pos)) != -1)
				{
					updateIndex(word, doc_id, index);

					free(word);
					//word = NULL;
					word = malloc(500*sizeof(char));
					MALLOC_CHECK(word);
					BZERO(word, 500*sizeof(char));
				}

				free(word);
			}

			free(file_contents);
		}

		free(files[i]);
	}

	free(files);

// outputs to a file
	saveFile(index, output_file_name);
	cleanIndex(index);

// if it's in testing mode
	if(indexer_test_flag)
	{
		INVERTED_INDEX* newindex;
		newindex = readIndex(input_file_name);
		saveFile(newindex, rewritten_file_name);
		cleanIndex(newindex);
	}
}

// saveFile takes an index and a file_name, and saves the contents of the index
// to the file "file_name" in the format specified in the header 
// Returns 0 if it succeeds and 1 if it fails. 
int saveFile(INVERTED_INDEX* in_index, char* file_name)
{
	FILE* fp;
	int doc_count;
	WordNode* current;
	DocumentNode* docnode;

	fp = fopen(file_name, "w");

	current = (WordNode*)(in_index->start);

// cycles through the nodes in the index
	while(current != NULL)
	{
// prints the word to the file pointed to by fp
		fprintf(fp, "%s ", (current->key));

		doc_count = 0;
		docnode = current->data;		

// calculates the document count for that word
		while(docnode != NULL)
		{
			doc_count++;
			docnode = docnode->next;
		}

// prints the document count to the same line
		fprintf(fp, "%d ", doc_count);

		docnode = current->data;

// prints the breakdown by document to the same line
		while(docnode != NULL)
		{
			fprintf(fp, "%d %d ", (docnode->document_id), (docnode->page_word_frequency));
			docnode = docnode->next;
		}

		fprintf(fp, "\n");

		current = current->next;
	}	

	fclose(fp);

	return 0;
}

// updateIndex takes a word, a document_id, and an index.  It adds the document to the index,
// and the word itself if it's not already contained in the index.  Returns 0 if success, 1 if failure.
int updateIndex(char* word, int document_id, INVERTED_INDEX* in_index)
{
	DocumentNode* docnode;
	WordNode* wordnode;
	DocumentNode* current_doc_node;

	int page_node_exists;

	page_node_exists = 0;

// creates a DocumentNode from the doc_id
	docnode = malloc(sizeof(DocumentNode));
  	MALLOC_CHECK(docnode);
  	docnode->document_id = document_id;
  	docnode->page_word_frequency = 1;
	docnode->next = NULL;

// makes it lower case (necessary for the query system)
	NormalizeWord(word);

	if(addData(in_index, docnode, word))	// if the wordnode already exists
	{
		wordnode = getData(in_index, word);

		if(wordnode != NULL)
		{
			current_doc_node = wordnode->data;

			while(current_doc_node != NULL)
			{
				if((current_doc_node->document_id) == document_id)
				{
					page_node_exists = 1;
					current_doc_node->page_word_frequency = (current_doc_node->page_word_frequency)+1;
					free(docnode);
					break;
				}
				else
					if(current_doc_node->next == NULL)
						break;
					else
						current_doc_node = current_doc_node->next;
			}

			if(!page_node_exists)
			{
				current_doc_node->next = docnode;
			}
		}			
	}

	return 0;
}

