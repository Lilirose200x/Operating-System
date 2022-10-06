
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "interpreter.h"
#include "shellmemory.h"


int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
	int i;
	char prompt = '$';  				// Shell prompt
	char userInput[MAX_USER_INPUT];		// user's input stored here
	int errorCode = 0;					// zero means no error, default

	printf("%s\n", "Shell version 1.1 Created January 2022");
	help();

	//init user input
	for (i=0; i<MAX_USER_INPUT; i++)
		userInput[i] = '\0';
	
	//init shell memory
	mem_init();

	while(1) {							
		printf("%c ",prompt);
		if (feof(stdin))
			break;
		fgets(userInput, MAX_USER_INPUT-1, stdin);
        char* commands[100];
		char tem[200];
		int a,b;
		if (strstr(userInput,";")!=NULL){		
		int w=0; // wordID

	    for(a=0; userInput[a]==' ' && a<1000; a++);		// skip white spaces

	    while(userInput[a] != '\0' && a<1000) {

		 for(b=0; userInput[a]!='\0' && userInput[a]!=';' && a<1000; a++, b++)
			tem[b] = userInput[a];						// extract a word
	 
		tem[b] = '\0';

		commands[w] = strdup(tem);

		a++; 
		w++;
	   }
	   for(int j=0;j<w;j++){
       errorCode=parseInput(commands[j]);
	   if (errorCode == -1) exit(99);	// ignore all other errors
		memset(userInput, 0, sizeof(userInput));
	   }
		}else{
		errorCode = parseInput(userInput);
		if (errorCode == -1) exit(99);	// ignore all other errors
		memset(userInput, 0, sizeof(userInput));
		}
	}

	return 0;

}

// Extract words from the input then call interpreter
int parseInput(char ui[]) {
 
	char tmp[200];
	char *words[100];							
	int a,b;							
	int w=0; // wordID

	for(a=0; ui[a]==' ' && a<1000; a++);		// skip white spaces

	while(ui[a] != '\0' && a<1000) {

		for(b=0; ui[a]!='\0' && ui[a]!=' ' && a<1000; a++, b++)
			tmp[b] = ui[a];						// extract a word
	 
		tmp[b] = '\0';

		words[w] = strdup(tmp);

		a++; 
		w++;
	}

	return interpreter(words, w);
}
