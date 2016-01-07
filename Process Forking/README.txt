By Pavitar Sidhu

This program uses the following algorithm in generating its descendant children from fork().

currentProcess = PMS;
currentNum = n;
while (currentNum >0) {
	generate a random number r in [1, currentNum];
	currentProcess forks r child processes;
	set the first child process to be currentProcess;
	currentNum = currentNum -r;
}

In order to run the program, first run the makefile. Then type in "./PMS 5" But the number 
5 will be replaced with however many descendant forks that you would like to generate.