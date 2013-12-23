// DESIGN SPECS FOR INDEXER.C

#include "../util/dictionary.h"

// updateIndex takes a word, a document_id, and an index.  It adds the document to the index,
// and the word itself if it's not already contained in the index.  Returns 0 if success, 1 if failure.
int updateIndex(char* word, int document_id, INVERTED_INDEX* index);

// saveFile takes an index and a file_name, and saves the contents of the index
// to the file "file_name" in the format specified in the header 
// Returns 0 if it succeeds and 1 if it fails.
int saveFile(INVERTED_INDEX* index, char* file_name);
