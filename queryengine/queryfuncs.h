/*
	queryfuncs.h

	Functions fully defined and explained in queryfuncs.c
*/

int pullQueries(char* input_line, QUERY** queries, int* num_queries);

void buildResults(INVERTED_INDEX* index, RESULT* results, int* temp_counts, QUERY** queries, int num_queries);

int sortResults(RESULT* results, int* temp_counts, RESULT* sorted_results);

void printResults(RESULT* sorted_results, int num_results);
