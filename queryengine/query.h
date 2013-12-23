/*
	query.h
	
	Most of the design is explaiend in query.c.

	QUERY data structure 	- contains a list of keywords ANDed together
							- OR separates different QUERYs

	RESULT data structure = DocumentNode (each one matches a page)
*/

#define MAX_NUM_QUERIES 20
#define MAX_NUM_KEYWORDS 20
#define MAX_KEYWORD_LENGTH 50
#define MAX_URL_LENGTH 2096
#define MAX_INPUT_LENGTH 1000
#define MAX_NUM_FILES 3000

#include "../util/dictionary.h"

typedef struct _QUERY
{
	char* search_words[MAX_NUM_KEYWORDS];
} __QUERY;

typedef struct _QUERY QUERY;

typedef struct _DocumentNode RESULT;
