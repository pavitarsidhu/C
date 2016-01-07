By Pavitar Sidhu

This is a C program that will simulate a priority queueing system (PQS). 
In such a system, there are arriving customers and a clerk who serves the customers. 
When a customer arrives and the clerk is busy with serving other customers, the arriving customer needs to wait.

sortCustomers() - This function will ensure that the priority queue of customers is arranged
in the correct order. It is sorted based on the following criteria: 
	(a) The one with the highest priority will be served first.
	(b) If there is a tie at the highest priority, the one whose arrival time is the earliest will be served first.
	(c) If there is still a tie, the one who has the shortest service time will be served first.
	(d) If there is still a tie, the one who appears first in the input file will be served first.

customerDispatch() - This function will control traffic flow of the arriving customers
and create threads for them. It acts as the "dispatch center" for the customers.

customerServiceThread() - This is the function that every newly created thread (customer)
runs. It has a protected critical section where only one customer is allowed in at any time.

execution_time() - This function will return time when the output event occurs minus the 
machine time when the simulation starts.

In order to run the program, run the makefile. And then type in "32 PQS customers.txt"
And the customers.txt must use the following format:

3
1:3,60,3
2:6,70,1
3:3,50,3

The very first number specifies the total number of customers arriving, which is "3" in the
example above.

customerNo. ArrivalTime  ServiceTime    Priority
	1 			0.3s		 6s 		   3
	2 			0.6s 		 7s 		   1
	3 			0.3s 		 5s 		   3
	
Priority must be between 1 and 10.