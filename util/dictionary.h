#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#define KEY_LENGTH 2049
#define MAX_HASH_SLOT 10000

// DNODE functions the same as WordNode, and will therefore be implemented as such
typedef struct _DNODE
{
	struct _DNODE *next;
	struct _DNODE *prev;
	void* data;
	char key[KEY_LENGTH];
} __DNODE;

typedef struct _DNODE DNODE;

typedef struct _DICTIONARY
{
	DNODE* hash[MAX_HASH_SLOT];
	DNODE* start;
	DNODE* end;
} __DICTIONARY;

typedef struct _DICTIONARY DICTIONARY;

// contains the DocumentNode structure
// next points to the next DocNode
// doc_id is the id for the document
// page_word_frequency is the number of times it occurs
typedef struct _DocumentNode
{
	struct _DocumentNode *next;
	int document_id;
	int page_word_frequency;
} __DocumentNode;

typedef struct _DocumentNode DocumentNode;

// the following typedefs are included to use the existing data structures and
// functions from crawler.  they're defined in dictionary.h and dictionary.c
typedef struct _DNODE WordNode;

typedef struct _DICTIONARY INVERTED_INDEX;

int hash(char* string);

DICTIONARY* initializeDict();

void cleanDict(DICTIONARY* dict);

int addData(DICTIONARY* dict, void* data, char* key);

INVERTED_INDEX* readIndex(char* filename);

DNODE* getData(DICTIONARY* dict, char* key);

// cleanIndex takes an index and frees all the data structures that it
// contains.  After looping through them all, it finally frees the index.
void cleanIndex(INVERTED_INDEX* to_be_cleaned);

#endif
