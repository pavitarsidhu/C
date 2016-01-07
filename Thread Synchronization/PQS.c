#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

//Will act as the data structure for each customer
typedef struct _customer {
	int id; 
	int priority; 
	double arrivalTime; 
	double serviceTime;
	double lapsedTime;
}customer; 

struct timeval start;

//These variables hold useful information regarding customers
int servicedCustomers = 0; 
int arrivedCustomers = 0; 
int totalCustomers;
int sortCustomerTracker = 0;

int inService = 0;

//Information variables for interruption of lower-level priority customers
int interruptFlag = -1;
int interruptedCustomerID = 0;

customer **priorityQueue;
customer **arrivedCustomerList;
int arrivedCustomerListIndex = 0;

pthread_mutex_t priorityQueueLock, synchronizingMutex, signalLock, existingChildrenCheck, customerWaitingLock;
pthread_cond_t synchronizingCond = PTHREAD_COND_INITIALIZER;


/*This method will ensure that the correct customer
is always the one at the front of the queue. */
void sortCustomers() {
	
	int i,j;
	customer *tmp;
	
	//Sort based on priority queue
	for(i=0;i<sortCustomerTracker-1;i++) {
    	for(j=0;j<sortCustomerTracker-1;j++) {
        	if(priorityQueue[j+1]->priority > priorityQueue[j]->priority) {
                tmp=priorityQueue[j];
                priorityQueue[j]=priorityQueue[j+1];
                priorityQueue[j+1]=tmp;
           }
        }
    }
    
    //The case to handle for ties in priority between customers
    if(sortCustomerTracker > 1) {
    	customer *currentLeader;
    	currentLeader = priorityQueue[0];
    	int x;
    	for(x=0;x<sortCustomerTracker;x++) {
    		int arrival1 = (int)(priorityQueue[x]->arrivalTime * 10);
    		int arrival2 = (int)(currentLeader->arrivalTime * 10);
    		if((priorityQueue[x]->priority == currentLeader->priority) && (arrival1 < arrival2)) {
    			customer *temp = priorityQueue[x];
    			priorityQueue[0] = temp;
    			priorityQueue[x] = currentLeader;
    		}
    	}
    	
    	//The case to handle for ties in priority and arrival time between customers
    	currentLeader = priorityQueue[0];
    	for(x=0;x<sortCustomerTracker;x++) {
    		if((priorityQueue[x]->priority == currentLeader->priority) && ((int) (priorityQueue[x]->arrivalTime * 10) == (int)(currentLeader->arrivalTime * 10)) && ((int) (priorityQueue[x]->serviceTime) < (int)(currentLeader->serviceTime)) ) {
    			customer *temp = priorityQueue[x];
    			priorityQueue[0] = temp;
    			priorityQueue[x] = currentLeader;
    		}
    	}
    }
}

//Return time elapsed since simulation began
double execution_time() {

	struct timeval current, result;

	gettimeofday(&current, NULL);
	timersub(&current, &start, &result);	
	return ((double)result.tv_sec + (double)result.tv_usec * .000001);
}

//All newly created customer threads use this function
void *customerServiceThread(void *customerInfo) {	
	 
	customer *thisCustomer = (customer *) customerInfo;	
	
	int checkExistance = 0;
	int z = 0;
	
	//Check if this thread (customer) has already "arrived"
	pthread_mutex_lock(&existingChildrenCheck);
	while(arrivedCustomerList[z] != NULL) {
		if(thisCustomer->id == arrivedCustomerList[z]->id) {
			checkExistance = 1;
			break;
		}
		z++;
	}
	
	if(checkExistance == 0) {
		printf("Customer %2d arrives: arrival time (%.2f), service time (%.1f), priority (%2d). \n",thisCustomer->id,execution_time(),thisCustomer->serviceTime,thisCustomer->priority);
		arrivedCustomerList[arrivedCustomerListIndex] = (customer *) malloc(sizeof(customer));
		arrivedCustomerList[arrivedCustomerListIndex] = thisCustomer;
		arrivedCustomerListIndex++;
	}
	
	pthread_mutex_unlock(&existingChildrenCheck);



/*
	REASON WHY MUTEX LOCK BELOW DOES NOT LOCK OUT ALL THREADS - IN A NUTSHELL, THE WAIT 
	CONDITION WORK AS A BLACK BOX AND ATOMICALLY RELEASES THE MUTEX WHILE THE CALLER
	THREAD SLEEPS!
 This routine should be called while mutex is locked, and it will automatically release the mutex 
 while it waits. After signal is received and thread is awakened, mutex will be automatically 
 locked for use by the thread. The programmer is then responsible for unlocking mutex when the thread is finished with it.
*/
	pthread_mutex_lock(&synchronizingMutex);
	while ((priorityQueue[0]->id != thisCustomer->id) || (inService == 1)) {
		//Check to see who is ahead of you in priority queue
		pthread_mutex_lock(&priorityQueueLock);
		if((sortCustomerTracker > 1) && (priorityQueue[1]->id == thisCustomer->id)) {
			printf("customer %2d waits for the finish of customer %2d. \n",thisCustomer->id,priorityQueue[0]->id);
		}
		
		pthread_mutex_unlock(&priorityQueueLock);
        pthread_cond_wait(&synchronizingCond, &synchronizingMutex);
    }   
	pthread_mutex_unlock(&synchronizingMutex);
	
	//Service the customer below 
	//This is the critical section
	
	inService = 1;
	
	//Tells us that this thread is interrupting another one
	if(interruptFlag == thisCustomer->id) {
		printf("customer %2d interrupts the service of lower-priority customer %2d. \n",thisCustomer->id,interruptedCustomerID);
		interruptFlag = -1;
	}
	
	printf("The clerk starts serving customer %2d at time %.2f. \n",thisCustomer->id,execution_time());
	while((int)(thisCustomer->lapsedTime * 10) != (int)(thisCustomer->serviceTime * 10)) {
		
		pthread_mutex_lock(&priorityQueueLock);
		if(priorityQueue[0]->id != thisCustomer->id) {
			interruptFlag = priorityQueue[0]->id;
			interruptedCustomerID = thisCustomer->id;
		
			pthread_mutex_unlock(&priorityQueueLock);
		
			inService = 0;
	 		//Send signal to the next customer to begin
			pthread_mutex_lock(&signalLock);
			pthread_cond_broadcast(&synchronizingCond);
			pthread_mutex_unlock(&signalLock);
		
	 		//Create thread for particular customer to continue after being interrupted
	 		pthread_t customerThread;
	 		int validThread = pthread_create(&customerThread, NULL, customerServiceThread, thisCustomer); 
			if(validThread) {
        		fprintf(stderr,"Error - pthread_create() return code: %d\n",validThread);
         		exit(EXIT_FAILURE);
    		}
			
			srand(time(NULL));
	 		intptr_t randN = rand() % 100;
	 		return (void *)randN;		
		}
		pthread_mutex_unlock(&priorityQueueLock);
		
		usleep(100000);
		thisCustomer->lapsedTime+=0.1;
	}
	 
	printf("The clerk finishes the service to customer %2d at time %f. \n", thisCustomer->id,execution_time());
	servicedCustomers++;
	
	//Remove serviced customer from array and block off priority queue
	pthread_mutex_lock(&priorityQueueLock);	
	
	int target;
	for(target=0;target<totalCustomers; target++) {
		if(priorityQueue[target]->id == thisCustomer->id) {
			break;
		}
	}
			
	priorityQueue[target] = NULL;
	
	//Move all customers to the left one
	int x;
	customer *temp;
	for(x=target;x<=totalCustomers-2;x++){
		priorityQueue[x] = priorityQueue[x+1];
	}
	sortCustomerTracker--;

	//Sort the customer queue
	sortCustomers();
	
	pthread_mutex_unlock(&priorityQueueLock);			

	inService = 0;
	//Send signal to the next customer to begin 
	pthread_mutex_lock(&signalLock);
	pthread_cond_broadcast(&synchronizingCond);
	pthread_mutex_unlock(&signalLock);
	
	 //Return a random number to end function
	 srand(time(NULL));
	 intptr_t randNo = rand() % 100;
	 return (void *)randNo;	
}

//The main dispatcher thread to control when customers are created
void customerDispatch(customer customers[], int numberOfCustomers) { 
	
	//All the required mutex variables are initiated
	
   if (pthread_mutex_init(&priorityQueueLock, NULL)) {
        printf("The mutex init failed.\n");
        exit(EXIT_FAILURE);
    }
	if (pthread_mutex_init(&synchronizingMutex, NULL)) {
    	printf("The mutex init failed.\n");
    	exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&existingChildrenCheck, NULL)) {
    	printf("The mutex init failed.\n");
    	exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&customerWaitingLock, NULL)) {
    	printf("The mutex init failed.\n");
    	exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&signalLock, NULL)) {
    	printf("The mutex init failed.\n");
    	exit(EXIT_FAILURE);
    }
    
	
	//This will hold priority sequence of customers
	priorityQueue = malloc(numberOfCustomers * sizeof(*priorityQueue));
	arrivedCustomerList = malloc(numberOfCustomers * sizeof(*priorityQueue));
	customer *creationCustomers[numberOfCustomers];
	
	int j;
	for(j = 0; j<numberOfCustomers; j++) {
		priorityQueue[j] = NULL;
		creationCustomers[j] = NULL;
		arrivedCustomerList[j] = NULL;
	}

	
	//Get time when the simulation begins
	gettimeofday(&start, NULL);
	
	int customerCreateCount = 0;
	
	/*Check each 1/10th of a second and bring in 
	arriving customers appropriately */	
	double trackTime = 0; 
	while(arrivedCustomers != numberOfCustomers) {
		trackTime += 0.1;
		
		//Find all customers that match current time
		int i;
		customerCreateCount = 0;
		for(i = 0; i<numberOfCustomers; i++) {
			if((int) (customers[i].arrivalTime *10) == (int)(trackTime *10)) {
				int h;
				for(h = 0; h<numberOfCustomers; h++) {
					//Add new customer to queue and initiate thread
					if(priorityQueue[h] == NULL) {
						
						pthread_mutex_lock(&priorityQueueLock);			
						
						priorityQueue[h] = (customer *) malloc(sizeof(customer));
						creationCustomers[customerCreateCount] = priorityQueue[h];
						priorityQueue[h]->id = customers[i].id;
						priorityQueue[h]->arrivalTime = customers[i].arrivalTime;
						priorityQueue[h]->serviceTime = customers[i].serviceTime;
						priorityQueue[h]->priority = customers[i].priority;
						priorityQueue[h]->lapsedTime = 0;
												
						//Sort queue after addition of new customer
						sortCustomerTracker++;
						sortCustomers();
						
						pthread_mutex_unlock(&priorityQueueLock);
						
						//Indicates how many children to create for each 0.1 second that passes
						customerCreateCount++;
						break;
					}
				}
			}

		}
		int counter;
		for(counter = 0; counter<customerCreateCount; counter++) {
			pthread_t customerThread;
			int validThread = pthread_create(&customerThread, NULL, customerServiceThread, creationCustomers[counter]); 
			if(validThread) {
        		fprintf(stderr,"Error - pthread_create() return code: %d\n",validThread);
         		exit(EXIT_FAILURE);
    		}				
			arrivedCustomers++;
		}
		usleep(100000);
	}
}


int main(int argc, char*arg[]) {
	int numberOfCustomers;
	
	//Ensures that exactly 1 input file is provided
	if((argc < 2) || (argc > 2)) {
		printf("Please provide one input file name.\n");
		exit(-1); 	
	} else {
		FILE *customerFile = fopen(arg[1],"r"); 
		
		//Invalid file name given, exit out of program
		if(customerFile == NULL) {
			printf("Please provide a valid file to open.\n");
			exit(-1); 
		} else {
			fscanf(customerFile,"%d",&numberOfCustomers); 
			customer customers[numberOfCustomers];
									
			int i;
			for(i = 0; i<numberOfCustomers; i++) {
				fscanf(customerFile,"%d:%lf,%lf,%d",&customers[i].id,&customers[i].arrivalTime,&customers[i].serviceTime,&customers[i].priority); 
				customers[i].arrivalTime = customers[i].arrivalTime/10;
				customers[i].serviceTime = customers[i].serviceTime/10;
			}

			//Close the provided file
			fclose(customerFile); 
			
			totalCustomers = numberOfCustomers;
			customerDispatch(customers,numberOfCustomers); 
		}
	}
	
	while(servicedCustomers != numberOfCustomers) {
		continue;
	}
	return 0; 
}