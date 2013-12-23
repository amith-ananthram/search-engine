/*

  FILE: crawler.c

  Description:

  Inputs: ./crawler [SEED URL] [TARGET DIRECTORY WHERE TO PUT THE DATA] [MAX CRAWLING DEPTH]

  Outputs: For each webpage crawled the crawler program will create a file in the 
  [TARGET DIRECTORY]. The name of the file will start a 1 for the  [SEED URL] 
  and be incremented for each subsequent HTML webpage crawled. 

  Each file (e.g., 10) will include the URL associated with the saved webpage and the
  depth of search in the file. The URL will be on the first line of the file 
  and the depth on the second line. The HTML will for the webpage 
  will start on the third line.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#include "../util/header.h"
#include "../util/html.h"
#include "../util/file.h"
#include "../util/hash.h"
#include "../util/dictionary.h"
#include "crawler.h"

/*


(5) *Crawler*
-------------

// Input command processing logic

(1) Command line processing on arguments
    Inform the user if arguments are not present
    IF target_directory does not exist OR depth exceeds max_depth THEN
       Inform user of usage and exit failed

// Initialization of any data structures

(2) *initLists* Initialize any data structure and variables

// Bootstrap part of Crawler for first time through with SEED_URL

(3) page = *getPage(seedURL, current_depth, target_directory)* Get HTML into a string and return as page, 
            also save a file (1..N) with correct format (URL, depth, HTML) 
    IF page == NULL THEN
       *log(PANIC: Cannot crawl SEED_URL)* Inform user
       exit failed
(4) URLsLists = *extractURLs(page, SEED_URL)* Extract all URLs from SEED_URL page.
  
(5) *free(page)* Done with the page so release it

(6) *updateListLinkToBeVisited(URLsLists, current_depth + 1)*  For all the URL 
    in the URLsList that do not exist already in the dictionary then add a DNODE/URLNODE 
    pair to the DNODE list. 

(7) *setURLasVisited(SEED_URL)* Mark the current URL visited in the URLNODE.

// Main processing loop of crawler. While there are URL to visit and the depth is not 
// exceeded keep processing the URLs.

(8) WHILE ( URLToBeVisited = *getAddressFromTheLinksToBeVisited(current_depth)* ) DO
        // Get the next URL to be visited from the DNODE list (first one not visited from start)
 
      IF current_depth > max_depth THEN
    
          // For URLs that are over max_depth, we just set them to visited
          // and continue on
    
          setURLasVisited(URLToBeVisited) Mark the current URL visited in the URLNODE.
          continue;

    page = *getPage(URLToBeVisited, current_depth, target_directory)* Get HTML into a 
            string and return as page, also save a file (1..N) with correct format (URL, depth, HTML) 

    IF page == NULL THEN
       *log(PANIC: Cannot crawl URLToBeVisited)* Inform user
       setURLasVisited(URLToBeVisited) Mark the bad URL as visited in the URLNODE.
       Continue; // We don't want the bad URL to stop us processing the remaining URLs.
   
    URLsLists = *extractURLs(page, URLToBeVisited)* Extract all URLs from current page.
  
    *free(page)* Done with the page so release it

    *updateListLinkToBeVisited(URLsLists, current_depth + 1)* For all the URL 
    in the URLsList that do not exist already in the dictionary then add a DNODE/URLNODE 
    pair to the DNODE list. 

    *setURLasVisited(URLToBeVisited)* Mark the current URL visited in the URLNODE.

    // You must include a sleep delay before crawling the next page 
    // See note below for reason.

    *sleep(INTERVAL_PER_FETCH)* Sneak by the server by sleeping. Use the 
     standard Linux system call

(9)  *log(Nothing more to crawl)

(10) *cleanup* Clean up data structures and make sure all files are closed,
      resources deallocated.

*/

// -------------------------
// ------- VARIABLES -------
// -------------------------

int page_number = 0; // a count of all the pages being download (from 1 to n)
int url_index;       // a global variable whose value is the final index in url_list

DICTIONARY* dict;    // the main data structure

int main(int argc, char *argv[])
{
	char* program;
  	char* seed_url;
  	char* target_dir;
  	int max_depth;

  	int current_depth;
  	char* page;
  	char* url;
	URLNODE* seednode;

	program = argv[0];
	
// checks to see if it's been given the proper # of arguments
  	if(argc != 4)
  	{
    	fprintf(stderr, "%s: Invalid number of arguments.  Crawler requires 3 (seed URL, download file, depth).\n", program);
    	return 1;
  	}

  	seed_url = argv[1];
  	target_dir = argv[2];
  	max_depth = atoi(argv[3]); 

// validates target directory
  	if (chdir(target_dir))
  	{
    	fprintf(stderr, "%s: Target directory %s does not exist.\n", program, target_dir);
    	return 1;
  	}

// validates depth
  	if(max_depth < 0 || max_depth > MAX_DEPTH || allDigits(argv[3]) == 0)
  	{
    	fprintf(stderr, "%s: The third argument, depth, must be an integer between 0 and %d inclusive.\n", program, MAX_DEPTH);
    	return 1;
  	}
  
// -- Bootstrap Seed_url --

// gets the seed url page and extracts the URLs from it
  	current_depth = 0;
  	page = getPage(seed_url, current_depth);
  	extractURLs(page, seed_url);
  	free(page);

// creates a URLNODE from the seed_url
  	seednode = malloc(sizeof(URLNODE));
  	MALLOC_CHECK(seednode);
  	seednode->depth = current_depth;
  	seednode->visited = 0;
  	BZERO(seednode->url, MAX_URL_LENGTH);
  	strncpy(seednode->url, seed_url, MAX_URL_LENGTH);

// sets up the dictionary
  	dict = initializeDict();

// links the seednode into the dictionary as a dnode
	dict->start = dict->end = malloc(sizeof(DNODE));
  	MALLOC_CHECK(dict->start); 
  	dict->start->prev = dict->start->next = NULL;
  	dict->hash[hash(seed_url)] = dict->start;
  	dict->start->data = seednode;
  	BZERO(dict->start->key, KEY_LENGTH);
  	strncpy(dict->start->key, seed_url, KEY_LENGTH);

// puts new urls into the doubly linked list to be crawled later
  	updateListLinkToBeVisited(++current_depth);

// sets this url node as already visited
  	setURLasVisited(seed_url);

// main functioning loop
// cycles through the doubly linked list until it runs out of unvisited urlnodes at each level (up to the max depth)
  	while(current_depth <= max_depth)
  	{
    		while((url = getAddressFromTheLinksToBeVisited(current_depth))) // gets the next unvisited url
    		{
      			setURLasVisited(url); 

      			if((page = getPage(url, current_depth))) 		// gets the page for that url
      			{	
        			extractURLs(page, url); 			// pulls the urls from the new page
        			updateListLinkToBeVisited(current_depth+1); 	// adds new urls to the doubly linked list
      			}
      			else
        		{
				fprintf(stderr, "Bad URL: %s\n", url);
				continue;
			}
      
      			free(page); 						// free the malloced page

      			printf("Logged url: %s at depth %d\n", url, current_depth); 			// log the success of the download

      			//char cmdLine[10];
      			//sprintf(cmdLine, "sleep %d", INTERVAL_PER_FETCH);	// sleep
      			//system(cmdLine);
    		}
    		++current_depth;
  	}

  	cleanUp();								// frees all malloced memory

  	return 0;
}

// ----------------------------------
// ------ FUNCTION DEFINITIONS ------
// ----------------------------------

// cleanUp is called at the end, and frees all dynamically allocated memory (to prevent memory leaks)
void cleanUp()
{ 
  	cleanDict(dict);
}

// getAddressFromTheLinksToBeVisited returns the next unvisited url at the specified depth (current_depth)
char* getAddressFromTheLinksToBeVisited(int current_depth)
{
  	DNODE* current;
  	URLNODE* unode;
  	int visited;
  	int depth;

  	current = dict->start; 							// starts with the first node
  
  	while(current != NULL)
  	{
    		unode = (URLNODE*)(current->data);				// pulls the urlnode out of it

    		visited = unode->visited;
    		depth = unode->depth;
 
    		if(visited == 0 && depth == current_depth)			// checks to see if it's unvisited and at the right depth, and if so returns it
      			return (unode->url);
    		else
      			current = current->next;				// if not, cycles through the remaining nodes
  	}

  	return NULL;								// returns NULL if all urls have been visited
}

// setURLasVisited takes a string and sets the associated URLNODE as visited
void setURLasVisited(char* url)
{
	URLNODE* unode = getData(dict, url)->data;

	if(unode != NULL)
  		unode->visited = 1;
}

// takes the urls from the global array url_list and adds them to the doubly linked list if they are unique
// takes a int depth, which is the current level when called
void updateListLinkToBeVisited(int depth)
{
	char* url;
	URLNODE* newunode;

	for(int i = 0; i < url_index; i++)					// cycles through the urls in url_list
  	{
    		url = url_list[i];
 
    		newunode = malloc(sizeof(URLNODE));				// creates a new URLNODE for it
    		MALLOC_CHECK(newunode);
    		newunode->depth = depth;
    		newunode->visited = 0;
    		BZERO(newunode->url, MAX_URL_LENGTH);
    		strncpy(newunode->url, url, MAX_URL_LENGTH);

    		if(addData(dict, newunode, url) == 1)
			free(newunode);

    		free(url_list[i]);						// free the dynamically allocated URL (allocated in extractURLs)
  	}
}

// extractURLs takes to strings and fills the url_list with new URLs to crawl.
// html_buffer, is the HTML of the page found at the URL current
void extractURLs(char* html_buffer, char* current)
{
	char* temp_list[1]; 
	int position;

 	*url_list = NULL;									// empty out the current URLs in url_list
	url_index = 0;										// and change the global index value to reflect that

  	position = 0;										// the index within the page (ie html_buffer); needed for GetNextURL

  	while(position < strlen(html_buffer))							// while there's still more html to parse in the buffer
  	{
    		*temp_list = NULL;								// temp_list is needed to verify that the URL pulled out matches the prefix
    		temp_list[0] = malloc(MAX_URL_LENGTH*sizeof(char));				// it needs to be allocated similarly to url_list later
    		MALLOC_CHECK(temp_list[0]);
    		BZERO(temp_list[0], MAX_URL_LENGTH*sizeof(char));

    		position = GetNextURL(html_buffer, current, *temp_list, position);		// GetNextURL returns the position in html_buffer where it stopped; 
												// fills temp_list with one URL at a time

    		if(strncmp(URL_PREFIX, temp_list[0], strlen(URL_PREFIX)*sizeof(char)) == 0)	// if it matches the prefix, put it into the main url_list
    		{
      			url_list[url_index] = malloc(MAX_URL_LENGTH*sizeof(char)); 		// this gets freed in updateListLinktoBeVisited
      			MALLOC_CHECK(url_list[url_index]);
      			BZERO(url_list[url_index], MAX_URL_LENGTH*sizeof(char));

      			strncpy(url_list[url_index++], temp_list[0], MAX_URL_LENGTH*sizeof(char));
    		}
        
    		free(temp_list[0]);								// free the temp_list
  	}
}

// allDigits takes an input_string and returns 0 if it contains any numeric characters
// otherwise it returns 1
int allDigits(char *input_string)
{
  	for(int i = 0; i < strlen(input_string); i++)
    		if(isdigit(input_string[i]) == 0)
      			return 0;

  	return 1;
}

// getPage takes a url string and a depth integer, and downloads the HTML
// of the page found at that url.  It puts this information in files of
// increasing number (1 2 3... n), along with the depth info and URL.
char* getPage(char* url, int depth)
{
  	char cmdLine[MAX_URL_LENGTH+30]; 							// max url length + extra chars below
  	FILE *fp;
	int size;
	char* pagetext;
	char output_file[5];

	sprintf(cmdLine, "wget '%s' -q -O temp", url);						// wget command that downloads the HTML to a file called temp
  	system(cmdLine);
  
  	size = fileLength("temp");
	pagetext = readFile("temp");


  	++page_number;										// output the HTML to an appropriatley incremented file
  	sprintf(output_file, "%d", page_number);

  	fp = fopen(output_file, "w");								// open up the new file and write the depth and URL to it
  	fprintf(fp, "%s\n%d\n", url, depth);
  	fwrite(pagetext, size, 1, fp);
  	fclose(fp);										// close the new file
	
  	return pagetext;									// return the newly downloaded HTML
} 
