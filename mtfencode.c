#include <stdio.h>
#include <stdlib.h> 
#include <string.h>


int A = 1000; 
int B = 81; 
 
int main(int argc, char *argv[]) { 	
 	
 	FILE *input;
 	input = fopen(argv[1], "r");
 	
 	char name[50]; 
 	
 	int counter = 0; 
 	while(argv[1][counter] != '.'){
 		name[counter] = argv[1][counter];
 		counter++; 
 	}
 	
 	//The new name with proper ending for the output file
 	name[counter] = '.';
 	counter++;
 	name[counter] = 'm';
 	counter++;
 	name[counter] = 't'; 
 	counter++; 
 	name[counter] = 'f'; 
 	counter++;
 	name[counter] = '\0';
 	
 	FILE *output; 
 	output = fopen(&name[0], "wb"); 
 	
 	//Insert magic for file
 	fputc(0xfa,output);
	fputc(0xce,output);
	fputc(0xfa,output);
	fputc(0xde,output);
 	
 	
 	char wordStore[A][B];
 	char buffer[B];
 	char *token; 
 	
 	int index = 0x81; 
 	int ascii; 
 	int num_words = 0;  
 	
 	char top[21];
 	char bottom[21];
 	
 	char *s;
 	
 	int result;
 	 	 	 	 		
 	//Take in content from file line by line
 	//And do what you need to do with each said line
 	while(fgets(buffer, B, input) != NULL) {

 		token = strtok(buffer, " ");
 		while(token != NULL) { 	
 		
 		
 		result = -1;
 		 		
 		int target = 0x81;
	
		int track; 	
		for(track = 0; track<A; track++) {
		
		int trimmer = strlen(token);
		if((token[trimmer -1] == '\n') && token[trimmer] == '\0') {
		
			token[trimmer-1] = '\0';
		
			if(strcmp(wordStore[track],token) == 0) {
				token[trimmer-1] = '\n';
				token[trimmer] = '\0';
				result = target; 
				break;
			} else {
				token[trimmer-1] = '\n';
				token[trimmer] = '\0';
				target++; }
			
			} else {
				if(strcmp(wordStore[track],token) == 0) {
				result = target; 
				break;
			} 
			target++;
			}
		}

 	 		if((strlen(token) == 1) && strchr(token, '\n')) {
 				fputc(0x0a,output);
 		} 
 		
 	 	else if(result != -1) {
 	 			//DO SOMETHING WHEN WORD DOES EXIST
 	 			
 	 			fputc(result,output);
 	 			if(strchr(token, '\n')) { 
 	 				fputc(0x0a,output);
 	 				}
					
 				//DO THE SWAP; as in bring the existing word to top
 				int num_swaps = result - 0x81;
 				
 				for(counter = num_swaps; counter > 0; counter = counter -1) {
 		 			strcpy(bottom, wordStore[counter]);
 		 			strcpy(top, wordStore[counter-1]);	
 		 			
 		 			strcpy(wordStore[counter],top);
 		 			strcpy(wordStore[counter-1],bottom);		
 				}
 		
 	 		} else {
 	 		
 	 			//DO SOME STUFF IF THE WORD DOES NOT EXIST
 	 			fputc(index,output); 
 	 			index++; 
 
 		  		strcpy(top, wordStore[0]);
 		  		
 	 			for(counter = 0; counter < num_words; counter++) {
 					strcpy(bottom, wordStore[counter+1]); 	
 					strcpy(wordStore[counter+1], top);  
 					strcpy(top, bottom); 
 	 		}

 	 		
 	 		counter = 0; 
 	 		
 	 		//now add each individual char from word to output
 	 			while(*token != '\0') {
 	 			
 	 				if(*token == '\n') {
 	 					fputc(0x0a,output);
						token++;
 	 				} else {
 	 			
 	 				ascii = (int) *token;
 	 				fputc(ascii,output); 
 	 				wordStore[0][counter] = *token;
 	 				
 	 				counter++;
 	 				token++;
 	 				}
 	 			}
 	 			wordStore[0][counter] = '\0';
 	 			num_words++;	
 	 		}
 	  	 	token = strtok(NULL, " ");
 		}
 		
 	}
  	
  		 	
 	//Close all files
 	fclose(input); 
 	fclose(output);
 	
 	return 0; 	
 
}