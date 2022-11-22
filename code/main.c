#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "web_scraper.h"
#include <pthread.h>
#include <string.h>
#include <time.h>
#define WORDS 100
#define LENGTH 200

int main(int nargs, char *argv[]){
	/* nthreads is the number of threads to use, words are total words to search for */
	int i=0, nthreads=1, words = 0, nurls = 0, outfile = 0; /*j = 0;*/ /* nthreads must have one to do the work */
	/* url is the .txt file containing list of urls. word_list is list of words to search for */
	char url[20], word_list[WORDS][LENGTH] = {{0}}; 


	// clock work
	clock_t start, end;
	double time_elapsed;
	start =  clock(); /* Begin Timer */
	// End clock work 

	/* CHECKS ARGUMENTS, UPDATES: wordlist, nthreads, words */
	if(arg_check(url, word_list, argv, &nthreads, nargs, &words, &outfile) != 0){
		printf("\tError arg_check \n");
		return 1;
	}
	/* check total urls to count */
	if((nurls = url_count(url)) < 0 ){
		printf("FAILED url_count");
		return 1;
	}
	//printf("threads: %d\n",nthreads); // debugging purposes
	//printf("total urls: %d\n", nurls); // debugging purposes
	/* If threads */ 
	if(nthreads){
		/* innitialize mutex */
		if(pthread_mutex_init(&mutex, NULL) != 0){ 
			fprintf(stderr, "Fail: pthread_mutex_init for mutex\n");
			return 1; 
		}
		if(outfile){
			if(pthread_mutex_init(&f_mutex, NULL) != 0){ 
				fprintf(stderr, "Fail: pthread_mutex_init for f_mutex\n");
				pthread_mutex_destroy(&mutex);
				return 1; 
		}
		}
		
		int total_threads = 0;
		/* limits threads to total urls or nthreads+1 */ 
		total_threads = nthreads > nurls ?  nurls :  nthreads; // max threads is total urls 
		TI thread_info[total_threads];
		pthread_t threads[total_threads];//, r_threads[total_threads];
		FILE * fptr = fopen(url, "r");
		size_t n = 0;
		char * str = NULL, url_list[nurls][LENGTH];
		if(!fptr){
			printf("Failed to open %s\n", url);
			return 1; 
		}
		for(i = 0; i < nurls ; i++ ){
			if (getline(&str, &n, fptr) != -1){
				strcpy(url_list[i],strtok(str, "\n"));
			} 
			else{
				printf("FAIL creating url list\n");
				return 1;
			}
		}
		free(str);
		fclose(fptr);

		//printf("total_threads: %d\n",total_threads);
		/* Sending total_threads to download all URLs */
		for(i = 0; i < total_threads; i++){
			thread_info[i].id = i; /* thread id */
			thread_info[i].urls = nurls; /* total urls */
			thread_info[i].outfile = outfile; /* writing to file? yes(1) no(0) */
			thread_info[i].words = words; /* total words */
			thread_info[i].word_list = &(word_list[0][0]); /* point to word_list*/
			thread_info[i].url_list = &(url_list[0][0]);
			
			/* send thread to function to count all instances on url */
			if(pthread_create(&threads[i], NULL, download, &thread_info[i]) != 0){
				fprintf(stderr, "Fail to create thread %d\n",i);
				break;
			}
		}
		for(i=0; i< total_threads; i++){
			//printf("Thread %d status: %d\n",i,thread_info[i].status);
			if(pthread_join(threads[i], NULL) != 0){ /* join all threads */
				fprintf(stderr, "Fail to free remaining threads after %d\n",i);
				return -1;
			}
		}
	}
	int one=0;
	if((one = pthread_mutex_destroy(&mutex) )){
		printf("%d: Error destroying mutex\n", one);
	}
	if(outfile){ // if writing to file, close f_mutex 
		int two=0;
		if((two = pthread_mutex_destroy(&f_mutex))){
				printf("%d: Error destroying f_mutex\n", two);
		}
	}
	// clock work
	end = clock();
	time_elapsed = ((double) (end-start)); /* Stop Timer */
	printf("\nTime Elapsed = %f Milliseconds\n", time_elapsed/1000);
	// End clock work
	return 0;
}

