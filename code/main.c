#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "web_scraper.h"
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

int main(int nargs, char *argv[]){
	/* nthreads is the number of threads to use, words are total words to search for */
	int i=0, nthreads=1, words = 0, nurls = 0, outfile = 0; /*j = 0;*/ /* nthreads must have one to do the work */
	/* url is the .txt file containing list of urls. word_list is list of words to search for */
	char url[LENGTH], word_list[WORDS][LENGTH] = {{0}}; /* url.txt filename max 200 characters */

	struct timeval begin, end;
	gettimeofday(&begin, 0); /* Begin Timer */

	/* CHECKS ARGUMENTS, UPDATES: wordlist, nthreads, words, outfile */
	if(arg_check(url, word_list, argv, &nthreads, nargs, &words, &outfile) != 0){
		printf("\tError arg_check \n");
		return 1;
	}
	/* check total urls to count */
	if((nurls = url_count(url)) < 0 ){
		printf("FAILED url_count");
		return 1;
	}
	/* If threads */ 
	if(nthreads){
		/* initialize mutex */
		if(pthread_mutex_init(&mutex, NULL) != 0){ 
			fprintf(stderr, "Fail: pthread_mutex_init for mutex\n");
			return 1; 
		}
		if(outfile){ // if writing output to file 
			if(pthread_mutex_init(&f_mutex, NULL) != 0){ // initialize file writing mutex
				fprintf(stderr, "Fail: pthread_mutex_init for f_mutex\n");
				pthread_mutex_destroy(&mutex);
				return 1; 
			}
		}
		
		int total_threads = 0; 
		total_threads = nthreads > nurls ?  nurls :  nthreads; /* limits threads to total urls */
		TI thread_info[total_threads];
		pthread_t threads[total_threads];
		FILE * fptr = fopen(url, "r");
		size_t n = 0;
		char * str = NULL, url_list[nurls][LENGTH];
		if(!fptr){ // file open check
			printf("Failed to open %s\n", url);
			return 1; 
		}
		for(i = 0; i < nurls ; i++ ){ // put urls into url_list
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

		//printf("total_threads: %d\n",total_threads); // debugging purposes

		/* Sending total_threads to download all URLs */
		for(i = 0; i < total_threads; i++){
			thread_info[i].id = i;				 /* thread id */
			thread_info[i].urls = nurls; 		/* total urls */
			thread_info[i].outfile = outfile; 	/* writing to file? yes(1) no(0) */
			thread_info[i].words = words; 		/* total words */
			thread_info[i].word_list = &(word_list[0][0]); 	/* point to word_list*/
			thread_info[i].url_list = &(url_list[0][0]);	/* point to url_list */
			
			/* send thread to function to download and count all instances on url */
			if(pthread_create(&threads[i], NULL, download, &thread_info[i]) != 0){
				fprintf(stderr, "Fail to create thread %d\n",i);
				break;
			}
		}
		/* join all threads once complete */
		for(i=0; i< total_threads; i++){
			//printf("Thread %d status: %d\n",i,thread_info[i].status);
			if(pthread_join(threads[i], NULL) != 0){ /* join all threads */
				fprintf(stderr, "Fail to free thread %d\n",i);
				//return -1;
			}
		}
	}
	int one=0;
	/* destoy mutex(s) */
	if((one = pthread_mutex_destroy(&mutex) )){
		printf("%d: Error destroying mutex\n", one);
	}
	if(outfile){ // if writing to file, close f_mutex 
		int two=0;
		if((two = pthread_mutex_destroy(&f_mutex))){
				printf("%d: Error destroying f_mutex\n", two);
		}
	}
	gettimeofday(&end, 0); /* Stop Timer */
        long seconds = end.tv_sec - begin.tv_sec;
        long microseconds = end.tv_usec - begin.tv_usec;
        double elapsed = seconds + microseconds * 1e-6;
        printf("\n\nElapsed Time: %.3f Seconds\n\n", elapsed); /* display time taken*/
	return 0;
}

