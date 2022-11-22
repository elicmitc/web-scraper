# web_scraper 2.0
This program uses the curl library to download and store a list of webpage into a text files. User defined words or phrases are passed by command line and each file is inspected and returns the total instances in each file. 
## Use
If ```-t``` is not given then its default is 1. 
The ```-o``` flag (optional) writes output data to outfile.txt. 
```
$ make 
$ ./main [-t <1-5>] [-w <words>] [-u <url file>] [-o]
$ make clean
```
## Makefile Commands
```
$ make all
$ make clean_files
$ make clean
```
+ make all - Compiles program
+ make clean - Removes all files made by ```make all```
+ make clean_files - Removes all files except output.txt

## Examples
### Example 1
This example uses 2 threads to find instances of "the" in urls listed in urls.txt and writes output to outfile.txt
```
$ make all
$ ./main -t 2 -u urls.txt -w the -o 
$ make clean
```
### Example 2
This example finds instances of "the" "what" and "who" in urls listed in urls.txt and outputs information to screen.
```
$ make all
$ ./main -u urls.txt -w the what who 
$ make clean
```

## Url File
Maximum of 30 urls. 
### urls.txt format
```
http://info.cern.ch/hypertext/WWW/TheProject.html
http://info.cern.ch/hypertext/WWW/WhatIs.html
http://info.cern.ch/hypertext/WWW/LineMode/Defaults/QuickGuide.html
http://info.cern.ch/hypertext/WWW/LineMode/Defaults/Commands.html
http://info.cern.ch/hypertext/WWW/LineMode/Defaults/SourceExamples.html
http://info.cern.ch./hypertext/TBL_Disclaimer.html
http://info.cern.ch./hypertext/Copyright.html
http://info.cern.ch/hypertext/WWW/Addressing/Addressing.html
http://info.cern.ch/hypertext/WWW/DesignIssues/Naming.html
http://info.cern.ch/hypertext/WWW/Status.html
http://info.cern.ch/hypertext/WWW/MidasWWW/Status.html
http://info.cern.ch/hypertext/WWW/MidasWWW/Announce1.html
http://info.cern.ch/hypertext/WWW/FIND/Features.html
```

