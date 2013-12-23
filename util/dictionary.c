// Contains the various dictionary related functions.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "header.h"
#include "hash.h"
#include "dictionary.h"

// Takes a string and returns the integer hash value for it (modified by the MAX_HASH_SLOT for the dictionary).
int hash(char* string) 
{
	return (hash1(string) % MAX_HASH_SLOT);
}

// Cycles through all the values in dict, freeing them.
void cleanDict(DICTIONARY *dict)
{
  	DNODE* current = dict->start;

  	while(current != NULL)
  	{
		if(current->data != NULL)
		{
      			free(current->data);
    		}


		DNODE* next = current->next;
		free(current);
		current = next;
	}

	free(dict);
}

// cleanIndex takes an index and frees all the data structures that it
// contains.  After looping through them all, it finally frees the index.
void cleanIndex(INVERTED_INDEX* to_be_cleaned)
{
	WordNode* currentword;
	WordNode* tempword;
	DocumentNode* currentpage;
	DocumentNode* temppage;

	currentword = to_be_cleaned->start;
	
	while(currentword != NULL)
	{
		currentpage = currentword->data;
		
		while(currentpage != NULL)
		{
			temppage = currentpage->next;
			free(currentpage);
			currentpage = temppage;
		}
		
		tempword = currentword->next;
		free(currentword);
		currentword = tempword;
	}
		
	free(to_be_cleaned);
}

// Returns an empty dictionary.
DICTIONARY* initializeDict()
{
	DICTIONARY* dict = (DICTIONARY*)malloc(sizeof(DICTIONARY));
  	MALLOC_CHECK(dict);
  	BZERO(dict, sizeof(DICTIONARY));
  	dict->start = dict->end = NULL;

	for(int i=0; i<MAX_HASH_SLOT; i++)
	{
		dict->hash[i] = NULL;
	}

  	return dict;
}

// Adds the void* data to the dictionary at the given key.
// Returns 0 if it succeeds and 1 if data is already contained
// in dict at key.
// For this lab, modified so that void* data must be a DNODE.
int addData(DICTIONARY* dict, void* data, char* key)
{
	int hash_index;

	DNODE* currentdnode;
	DNODE* newdnode;

	char* current_key;

	hash_index = hash(key);
	newdnode = malloc(sizeof(DNODE));
	MALLOC_CHECK(newdnode);
	newdnode->data = data;
	BZERO(newdnode->key, KEY_LENGTH);
	strncpy(newdnode->key, key, KEY_LENGTH);

	if((dict->hash[hash_index]) != NULL)
	{
		currentdnode=dict->hash[hash_index];

		while( 1 )
		{
			current_key = currentdnode->key;
			
			if(strcmp(current_key, key) == 0)	// the difference in data structures occurs here (increment instead of throwing it out)
			{
				free(newdnode);	
				
				return 1;
			}
			
			if(currentdnode->next == NULL || hash(currentdnode->next->key) != hash_index)
				break;
			else
				currentdnode = currentdnode->next;
		}

		newdnode->next = currentdnode->next;
		newdnode->prev = currentdnode;
			
		if((currentdnode->next) != NULL)
			currentdnode->next->prev = newdnode;

		currentdnode->next=newdnode;
		//dict->end = newdnode;
	}
	else
	{
		currentdnode=dict->start;
		
		if(currentdnode != NULL)
		{
			while(currentdnode->next != NULL)
				currentdnode = currentdnode->next;	
		
			currentdnode->next = newdnode;	
		}
		else
			dict->start = newdnode;	

		newdnode->next = NULL;
		newdnode->prev = currentdnode;
		dict->end = newdnode;
		dict->hash[hash_index] = newdnode;
	}	

	return 0;
}

// Returns the DNODE associated with the char* key in dict.
DNODE* getData(DICTIONARY* dict, char* key)
{
	int hash_index;
	DNODE* dnode;

	hash_index = hash(key);
	dnode = dict->hash[hash_index];

	while(dnode != NULL)
	{
		if(strcmp((dnode->key), key) == 0)
			break;
		else
			dnode = dnode->next;
	}

	return dnode;
}

// readIndex takes a file_name (which points to an index file), a reads the data into
// an index structure and returns that structure.
INVERTED_INDEX* readIndex(char* file_name)
{
	FILE* fp;
	INVERTED_INDEX* new_index;

	char *word;	
	int page_count;
	int page;
	int count;

	WordNode* wordnode;
	DocumentNode* docnode;
	DocumentNode* currentdocnode;

	new_index = initializeDict();

	fp = fopen(file_name, "r");

	word = malloc(500*sizeof(char));
	MALLOC_CHECK(word);
	BZERO(word, 500*sizeof(char));

// goes through each line and pulls the first two strings from it (ie the word and the page count)
	while(fscanf(fp, "%s %d", word, &page_count) == 2)
	{
		wordnode = NULL;

// goes through the rest of the line, pulling two integers from it a time up to the page_count
		for(int i = 0; i < page_count; i++)
		{
			fscanf(fp, "%d %d", &page, &count);
			currentdocnode = NULL;
			docnode = NULL;

// creates a DocumentNode from the given page and count information
			docnode = malloc(sizeof(DocumentNode));
			MALLOC_CHECK(docnode);
			docnode->document_id = page;
			docnode->page_word_frequency = count;
			docnode->next = NULL;

// if there's already a WordNode for the word, it adds it to its data
			if((wordnode = getData(new_index, word)) != NULL)
			{
				currentdocnode = wordnode->data;

				while((currentdocnode->next) != NULL)
					currentdocnode = currentdocnode->next;

				currentdocnode->next = docnode;
			}
			else
// otherwise it adds the WordNode and the DocumentNode
				addData(new_index, docnode, word);
		}	
	}	

	free(word);
	fclose(fp);

	return new_index;
}

