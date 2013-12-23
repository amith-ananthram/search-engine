#define INTERVAL_PER_FETCH 0
#define URL_PREFIX "http://www.cs.dartmouth.edu"
#define MAX_URL_LENGTH 2049
#define MAX_DEPTH 10
#define MAX_TRY 3
#define MAX_URL_PER_PAGE 1000

char *url_list[MAX_URL_PER_PAGE];

typedef struct _URL
{
	char url[MAX_URL_LENGTH];
	int depth;
	int visited;
} __URL;

typedef struct _URL URLNODE;

void cleanUp();

char* getAddressFromTheLinksToBeVisited(int current_depth);

void setURLasVisited(char* url);

void updateListLinkToBeVisited(int depth);

void extractURLs(char* html_buffer, char* current);

int allDigits(char *input_string);

char* getPage(char* url, int depth);
