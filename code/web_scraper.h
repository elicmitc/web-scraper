#include<stdio.h>
#include <pthread.h>
#ifndef WEB_SCRAPER_H_
#define WEB_SCRAPER_H_
#define WORDS 100
#define LENGTH 200
#define MAX_URL 100

pthread_mutex_t mutex;
pthread_mutex_t f_mutex;

/*struct holding information for thread function */
struct thread_info {
	char* word_list;/* pointer to word_list */
    char *url_list; /* pointer to URL list */
    int urls;       /* total urls */
    int words;      /* total words to search */
    int id;         /* thread id */
    int outfile;    /* output file? yes (1) no(0) */
};
/* type TI */
typedef struct thread_info TI;

/* check total urls */
int url_count(char * filename);

/* helper function for download */
size_t write_data(char *ptr, size_t size, size_t nmemb, void *filestream);

/* downloads url contents to file and counts instances */
void * download(void * arg);

/* iterates through file and returns total occurances */
int check_contents(char* filename, char * word, char* url, int outfile);

/* checks and updates the command line arguments into struct */
int arg_check(char *url, char word_list[WORDS][LENGTH], char **argv, int *nthreads, int nargs, int *index, int *outfile);

#endif
