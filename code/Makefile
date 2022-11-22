CC = gcc
CFLAGS = -pedantic -Wall -Wextra -Werror -lcurl -lpthread
CLIBRARY = web_scraper.c

all:
	$(CC) main.c -o main $(CFLAGS) $(CLIBRARY) 

clean_files:
	rm -f main file*.txt
clean:
	rm -f main file*.txt outfile.txt