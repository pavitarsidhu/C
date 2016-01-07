#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>


/* 
	BY PAVITAR SIDHU
	V00799092
	
	CSC 360 - ASSIGNMENT 1
*/



//The only global variable in this program
//Used by a number of different functions throughout the program
int numberOfProcesses;

//Checks to see if Process ID entered by user is a valid one for use
int validateProcess(int runningProcesses[], int target) {
	int result = 1; 
	int i = 0; 
	while(runningProcesses[i] != -999) {
		if(runningProcesses[i] == target) {
			result = 0; 
			return result; 
		}
		i++; 
	}
	
	return result;
}

void printMainMenu(int runningProcesses[]) {
	
	const char* prompt = "Please input your selection: \n 1. List \n 2. Suspend \n 3. Resume \n 4. Terminate \n 5. Exit \n";
	char *reply = readline(prompt);
	int userSelection = atoi(reply);
	
	if(userSelection == 5) {
		killpg(getpgrp(),SIGTERM);
		
	} else if(userSelection == 1) {	
	
		char buffer[10];
		int value = (int)getsid(getpid());
		sprintf(buffer, "%d", value);
		
		//The parameters needed to produce the PID,PPID,Time of each child
		char *const parmList[] = {"ps","-o","pid,ppid,time","-g","-r",buffer,NULL};
		
		int childrenListProcess = fork();
		if(childrenListProcess<0) {
			printf("The fork has failed."); 
			exit(EXIT_FAILURE); 
		} else if(childrenListProcess == 0) {
			setsid();
			printf("\n");
			execvp("/bin/ps", parmList);
		} else {
			int status;
			waitpid(childrenListProcess, &status, 0); 
		}
		
		const char* prompt = "";
		char* reply = readline(prompt);
		printMainMenu(runningProcesses); 

	} else if(userSelection == 2) {
		const char* prompt = "Please Enter Process ID: ";
		char* reply = readline(prompt);
		
		int result = validateProcess(runningProcesses, atoi(reply));
		if(result == 0) {
			int returnedValue = kill(atoi(reply),SIGSTOP); 
			printMainMenu(runningProcesses);	
		} else {
			printf("That was not a valid Process ID.\n");
			printMainMenu(runningProcesses);
		}
	} else if(userSelection == 3) {
		const char* prompt = "Please Enter Process ID: ";
		char* reply = readline(prompt);
		
		int result = validateProcess(runningProcesses, atoi(reply));
		if(result == 0) {
			int returnedValue = kill(atoi(reply),SIGCONT); 
			printMainMenu(runningProcesses);
		} else {
			printf("That was not a valid Process ID.\n");
			printMainMenu(runningProcesses);
		}
	} else if(userSelection == 4) {
		const char* prompt = "Please Enter Process ID: ";
		char* reply = readline(prompt);
		
		int result = validateProcess(runningProcesses, atoi(reply));
		if(result == 0) {
		
			/*This will remove the process that we are about to terminate
			 from our running list of on-going processes
			 */
			int i = 0;
			while(runningProcesses[i] != -999) {
				if(runningProcesses[i] == atoi(reply)) {
				runningProcesses[i] = -777;
				}
				i++;
			}
		
			int returnedValue = kill(atoi(reply),SIGTERM); 	
			int status;
			waitpid(atoi(reply),&status, 0); 
			printMainMenu(runningProcesses);	
		
			} else {
				printf("That was not a valid Process ID.\n"); 
				printMainMenu(runningProcesses);
			}
	} else {
		printMainMenu(runningProcesses);
	}
}

void ProGen() {

	//Sets the Process Group ID to the original process of the program
	//This is useful in calling the Exit function of the PMS
	pid_t processGroupID = setsid();
	  
	int realNumberOfProcesses = numberOfProcesses;
	
	int i;
	int targetPid = getpid(); 
	int designatedPid = targetPid; 
	int randomNumber;
		
	while(realNumberOfProcesses > 0) {

		randomNumber = rand() % (realNumberOfProcesses + 1 - 1) + 1; 
		
		//If the current process is the required one, then update the designated pid
		if(targetPid == (int)getpid()) {
			designatedPid = getpid();
		}
		
		/*This code only executes if the current process is the one 
		that is in charge of producing more descendants */
		if((int)getpid() == designatedPid) {
			for(i = 0; i<randomNumber; i++) {
				if(i == 0) {
					int t = fork();
					if(t<0) {
						printf("The fork has failed.");
						exit(EXIT_FAILURE);
					} else if(t == 0) {
						targetPid = getpid();
						designatedPid = getpid();
						setpgid(getpid(),processGroupID);
						break;
					} else if(t!= 0) {
						realNumberOfProcesses = 0;
					}
				} else {
					int t2= fork();
					if(t2 < 0) {
						printf("The fork has failed."); 
						exit(EXIT_FAILURE);
					} if(t2 == 0) {
						setpgid(getpid(),processGroupID);
						while(1);
					}
				}
			}
		}
	
		realNumberOfProcesses = realNumberOfProcesses - randomNumber; 
	}
}

int main(int argc, char *arg[]) {

	int originalPid = getpid(); 
	
	//Must be set once to ensure random generator function works properly
	srand (time(NULL));

	if(argc < 2) {
		printf("Please provide the number of processes you would like to start.\n");
	} else {
	
		//initiate number of wanted processes
		numberOfProcesses = atoi(arg[1]);
		ProGen();
		
		//Only the main process will provide the user with a PMS menu
		if((int)getpid() == originalPid) {
		
		//Initiates an array with all children descendants of parent
		int arraySize = numberOfProcesses + 5; 
		int runningProcesses[arraySize]; 
		
		int counter = 0; 
		int i; 
		for(i = 0; i<numberOfProcesses; i++) {
			runningProcesses[i] = (int)getpid() + i + 1;
			counter++; 
		}
		runningProcesses[counter] = -999;
		
		printMainMenu(runningProcesses);	
		} else {
		/*All children except the root are put in an endless while loop in order
		to ensure that they continue to execute throughout the program. */
			while(1); 
		}
	}		
	return 0; 
}
