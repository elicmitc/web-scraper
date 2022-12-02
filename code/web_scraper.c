#include "web_scraper.h"
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define USAGE "Error..\n\tUsage: ./main [-t <1-5>] [-u <urlfile.txt>] [-w <words>] \n\tExample: ./main -t 2 -u url.txt -w hello world"

int url_index = 0; /* shared index for checking urls */

size_t write_data(char *ptr, size_t size, size_t nmemb, void *filestream){
	return fwrite(ptr, size, nmemb, filestream);
}
void * download(void * arg){

	TI *info = (TI *) arg;
	CURL *curl_handle = curl_easy_init(); /* initialize the curl easy handle */
	if(url_index<0 || url_index >= info->urls){
		fprintf(stderr, "Failed: no more urls\n");
		return NULL;
	}

	if(curl_handle == NULL){ /* check initializatin */
			fprintf(stderr, "Failed (%d): curl_easy_init\n", info->id); /* print error */
			return NULL; /* exit failure */
	}
		/* check setopt */
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
	int l_url_index = 0,i;
	FILE *fptr;
	int outfile = info->outfile;
	while(url_index < info->urls){ // while urls left to inspect
		pthread_mutex_lock(&mutex); 
		if(url_index >= info->urls){ // if no urls to inspect, unlock and exit 
			pthread_mutex_unlock(&mutex);
			break;
		}
		l_url_index = url_index; // thread is taking url in current index in file 
		url_index++; // increase index 
		pthread_mutex_unlock(&mutex);

		char filename[15] = "file", ind[5];
		sprintf(ind, "%d", l_url_index+1); /* filenames start at 1 */
		strcat(filename,ind);
		strcat(filename, ".txt");
		fptr = fopen(filename,"w");


		if(!fptr){
				printf("Failed (%d): to create %s\n", info->id, info->url_list);
				return NULL; /* exit failure */
		}
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fptr); // update pointer to file to write to 
		curl_easy_setopt(curl_handle, CURLOPT_URL, info->url_list+(l_url_index*LENGTH)); // update file to download
		

		if(curl_easy_perform(curl_handle)){ /* check perform */
				fprintf(stderr, "Failed (%d): Could not download contents from %s\n", info->id, info->url_list+(l_url_index*LENGTH)); /* print error */
				return NULL; /* exit failure */
		}
			//curl_easy_cleanup(curl_handle);
		fclose(fptr);
		for(i=0; i < info->words; i++){
			if(check_contents(filename, info->word_list+(i*LENGTH), info->url_list+((l_url_index)*LENGTH), outfile)){ 
			}
		}
	}
	curl_easy_cleanup(curl_handle);

	return NULL; /* exit with success*/
}

int check_contents(char* filename, char * word, char * url, int outfile){
	size_t n = 0;
	char * str = NULL, *lstr = NULL;
	int count=0;	
	FILE * fptr = fopen(filename, "r");
	if(fptr == NULL){
		fprintf(stderr, "Failed: Couldn't open %s.\n",filename);
		return -1;
	}
	while(getline(&lstr, &n, fptr) != -1){ /* lstr iterates through the url file */
		str = lstr; /* str points at the beginning of the line */
		while((str = strstr(str, word)) != NULL ){
			count++;
			str = str + 1;
		}
	}
	fclose(fptr);
	free(lstr);
	if(outfile){ /* write information to outfile.txt */
		pthread_mutex_lock(&f_mutex);
		FILE * outptr = fopen("outfile.txt","a");
		if(outptr == NULL){
			fprintf(stderr, "Failed: Couldn't open outfile.txt\n");
			pthread_mutex_unlock(&f_mutex);
			return -1;
		}
		fprintf(outptr, "Found %d instances of \"%s\" in %s\n", count,word,url);
		fclose(outptr);
		pthread_mutex_unlock(&f_mutex);
	}else{
		printf("Found %d instances of \"%s\" in %s\n", count,word,url);
	}
	return 0;
}

int arg_check(char *url, char word_list[WORDS][LENGTH], char **argv, int *nthreads, int nargs, int *index, int *outfile){
	int i,j, t = 0, u = 0, w = 0, o = 0; /* t: threads u: url file w: words to search o: output file */ 
	
	if(nargs < 5){
		printf("%s\n",USAGE);
	}
	for(i = 1; i<nargs; i++){
		//printf("argv[%d]: %s\t",i,argv[i]);
		if((t == 0) && (strncmp(argv[i], "-t",2) == 0) && (strlen(argv[i]) == 2)){
			*nthreads = atoi(argv[i+1]);
			i++; // 
			t++; // mark threads as accounte for
			if(*nthreads > 5 || *nthreads <= 0){
				printf("Number must be between 1 and 5\n");
				return 1;
			}
		}
		else if((u == 0) && (strncmp(argv[i], "-u",2) == 0) && (strlen(argv[i]) == 2)){ // if haven't read url file && argv[i] == '-u'
			strcpy(url, argv[i+1]);
			u++; // url has been given
			i++; 
		}
		else if((o == 0) && (strncmp(argv[i], "-o",2) == 0) && (strlen(argv[i]) == 2)){ // writing to file 
			o++;
			*outfile = 1;
			//printf("We have an outfile occurance\n");
		}
		else if((w == 0) && (strncmp(argv[i], "-w",2) == 0) && (strlen(argv[i]) == 2)){
			w++;
			for(j = i+1; j<nargs; j++){ // loop through arguments following -w flag
				if(strncmp(argv[j],"-",1) == 0){ // check if flag
					//printf("\n\t%s\n",argv[j]);
					if(strlen(argv[j]) == 2){ // check if flag is 2 characters
						char flag = argv[j][1];
						if((flag == 'u' && u) || (flag == 't' && t) || (flag == 'w' && w) || (flag == 'o' && o)){ // if extra u/t/w\0 flag then exit
							printf("\n\n%s\n\n", USAGE);
							return 1; 
						}
						else if(!u || !t || !o){ // check if u or t need fullfilled 
							break; // if true then break
						} 
					}
				}
				if(*index < WORDS){ // only taking in WORDS words
					strcpy(word_list[*index], argv[j]); /* add word to search words list */
					(*index)++; // increase index in word list 
					i++; // increase index for i 
				}
				else{ // exit loop if more than WORDS words
					break;
				}
			}
		}
		else{
			printf("\n\n%s\n\n", USAGE);
			return 1;
		}
	}
	return 0;
}

int url_count(char * filename){
	FILE *fptr = fopen(filename, "r");
	int nurls = 0,lsize=0;
	char *lstr = NULL;
	size_t n=0;
	if(fptr){
		while((lsize = getline(&lstr, &n, fptr)) != -1){ /* lstr iterates through the url file */
			if(lsize>0){ /* if line isn't empty*/
				nurls++;
			}
		}
		fclose(fptr); /* close file */
		free(lstr);   /* free lstr */
		return nurls; /* return total url count */
	}
	else{ /* if file doesn't open */
		printf("FAILURE: BAD FILE <%s>\n", filename);
		return 1;
	}
}
