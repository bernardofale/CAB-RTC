/******************************************************************
 * Paulo Pedreiras, Sept/2022
 * DETI/UA/IT - Real-Time Operating Systems course
 *
 * Base code for periodic thread execution using clock_nanosleep
 * Known issues and limitations: 
 * 		- Assumes that periods and execution times are below 1 second
 *    
 *
 *****************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h> //sched_setscheduler
#include <pthread.h>
#include <errno.h>
#include <signal.h> // Timers
#include <stdint.h>
#include <unistd.h>
#include <math.h>


/* ***********************************************
* App specific defines
* ***********************************************/
#define TRUE 		1				// Define TRUE logic value
#define FALSE		0				// Define FALSE logic value
#define NS_IN_SEC 1000000000L

#define PERIOD_NS (100*1000*1000) 	// Period (ns component)
#define PERIOD_S (0)				// Period (seconds component)

#define BOOT_ITER 10				// Number of activations for warm-up
                                    // There is an initial transient in which first activations
                                    // often have an irregular behaviour (cache issues, ..)


/* ***********************************************
* Prototypes
* ***********************************************/
void Heavy_Work(unsigned char FirstFlag);
struct  timespec TsAdd(struct  timespec  ts1, struct  timespec  ts2);
struct  timespec TsSub(struct  timespec  ts1, struct  timespec  ts2);

// argument structure for thread1_code function
struct arg_struct {
    char procname[40];
    int arg2;
};

/* *************************
* Thread_1 code 
* **************************/

void * Thread_1_code(void *arg)
{
	struct arg_struct *args = arg;
    /* Timespec variables to manage time */
	struct timespec ts, // thread next activation time (absolute)
			ta, 		// activation time of current thread activation (absolute)
			tiat, 		// thread inter-arrival time,
			ta_ant, 	// activation time of last instance (absolute),
			tp; 		// Thread period
		
	/* Other variables */
	uint64_t min_iat, max_iat; // Hold the minimum/maximum observed inter arrival time
	int niter = 0; 	// Activation counter
	int update; 	// Flag to signal that min/max should be updated
	
	/* Set absolute activation time of first instance */
	tp.tv_nsec = args->arg2 * (1000 * 1000); //period inserted by user (ms to ns)
	tp.tv_sec = PERIOD_S;	
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts = TsAdd(ts,tp);	
	
	/* Periodic jobs ...*/ 
	while(1) {

		/* Wait until next cycle */
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,&ts,NULL);
		clock_gettime(CLOCK_MONOTONIC, &ta);		
		ts = TsAdd(ts,tp);		
		
		niter++; // Count number of activations
		
		/* Compute latency and jitter */		
		tiat=TsSub(ta,ta_ant);  // Compute time since last activation
		if(niter == BOOT_ITER) {	// Boot time finished. Init max/min variables	    
			  min_iat = tiat.tv_nsec;
		      max_iat = tiat.tv_nsec;
		      update = 1;
		} else {
			if( niter > BOOT_ITER) { 	// Update max/min, if boot time elapsed 	    
				if(tiat.tv_nsec < min_iat) {
					min_iat = tiat.tv_nsec;
					update = 1;
				}
				if(tiat.tv_nsec > max_iat) {
					max_iat = tiat.tv_nsec;
					update = 1;
				}
			}
		}
		
		ta_ant = ta; // Update ta_ant
	
		  
  		/* Print maximum/minimum inter-arrival time */
		if(update) {
		  printf("Task %s inter-arrival time (us): min: %10.3f / max: %10.3f \n\r", args->procname, (float)min_iat/1000, (float)max_iat/1000);
		  update = 0;
		}
		
		/* Do the actual processing */		
		if(niter == 1)
			Heavy_Work(TRUE); /* For the first activation estimate the execution time */
		else
			Heavy_Work(FALSE);		
	}  
  
    return NULL;
}

/* *************************
* main()
* **************************/

int main(int argc, char *argv[])
{
	int err;
	pthread_t threadid;
	//scheduler parms and initialization of attributes
	pthread_attr_t attr;
	struct sched_param parm;
	pthread_attr_init(&attr); 
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED); 
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	/* Process input args */
	if(argc != 4) {
	  printf("Usage: %s PROCNAME, where PROCNAME is a string\n\r", argv[0]);
	  printf("Usage: %s PROCNAME, where PRIORITY is a int [1-99]\n\r", argv[0]);
	  printf("Usage: %s PROCNAME, where PERIODICITY is a int [50-500]ms\n\r", argv[0]);
		//sair do programa se os valores forem maiores ou menores? ou usar valores default
	  return -1; 
	}
	//ccheck if priority > 1 and < 99, if not set default value 50
	parm.sched_priority = atoi(argv[2]) <= 99 && atoi(argv[2]) >= 1 ?  atoi(argv[2]) : 50 ;
	pthread_attr_setschedparam(&attr, &parm);
	
	struct arg_struct args;
	strcpy(args.procname, argv[1]);
	//ccheck if periodicity > 50 and < 500, if not set default value 100ms
	args.arg2 = atoi(argv[3]) <= 500 & atoi(argv[3] ) >=50 ?  atoi(argv[3]) : 100 ;
	

	//set affinity mask to bound cpu0 to our threads (??)
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);
	
		
	/* Create periodic thread/task */
	err=pthread_create(&threadid, &attr, Thread_1_code, &args);
	pthread_setaffinity_np(threadid, sizeof(set), &set);
 	if(err != 0) {
		printf("\n\r Error creating Thread [%s]", strerror(err));
		return -1;
	}
	else 
		while(1); // Ok. Thread shall run
	
	/* Last thing that main() should do */
    pthread_exit(NULL);
		
	return 0;
}




/* ***********************************************
* Auxiliary functions 
* ************************************************/

/* Emulates task processing ... 
 * In the case integrates numerically a function
 * The "first" argument is a flag to signal the first (1) or subsequent
 * (0) activations 
 * "subInterval", which is the resolution, has a sigificant impact in the execution time
 */
 
#define f(x) 1/(1+pow(x,2)) /* Define function to integrate*/

void Heavy_Work(unsigned char FirstFlag)
{
	float lower, upper, integration=0.0, stepSize, k;
	int i, subInterval;
	
	struct timespec ts, // Function start time
			tf; 		// Function finish time
	
	
	/* Get start time */
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	/* Integration parameters */
	/* These values can be tunned to cause a desired load*/
	lower=0;
	upper=100;
	subInterval=650000;

	 /* Calculation */
	 /* Finding step size */
	 stepSize = (upper - lower)/subInterval;

	 /* Finding Integration Value */
	 integration = f(lower) + f(upper);
	 for(i=1; i<= subInterval-1; i++)
	 {
		k = lower + i*stepSize;
		integration = integration + 2 * f(k);
 	}
	integration = integration * stepSize/2;
 	
 	/* Get finish time and show results */
 	if (FirstFlag == TRUE) {
		clock_gettime(CLOCK_MONOTONIC, &tf);
		tf=TsSub(tf,ts);  // Compute time difference form start to finish
 	
		printf("Integration value is: %.3f. It took %4.2f ms to compute.\n", integration, (float)tf.tv_nsec/1000000);	
	}

}


// Adds two timespect variables
struct  timespec  TsAdd(struct  timespec  ts1, struct  timespec  ts2){
	
    struct  timespec  tr;
	
	// Add the two timespec variables
    	tr.tv_sec = ts1.tv_sec + ts2.tv_sec ;
    	tr.tv_nsec = ts1.tv_nsec + ts2.tv_nsec ;
	// Check for nsec overflow	
	if (tr.tv_nsec >= NS_IN_SEC) {
        	tr.tv_sec++ ;
		tr.tv_nsec = tr.tv_nsec - NS_IN_SEC ;
    	}

    return (tr) ;
}

// Subtracts two timespect variables
struct  timespec  TsSub (struct  timespec  ts1, struct  timespec  ts2) {
  struct  timespec  tr;

  // Subtract second arg from first one 
  if ((ts1.tv_sec < ts2.tv_sec) || ((ts1.tv_sec == ts2.tv_sec) && (ts1.tv_nsec <= ts2.tv_nsec))) {
    // Result would be negative. Return 0
    tr.tv_sec = tr.tv_nsec = 0 ;  
  } else {						
	// If T1 > T2, proceed 
        tr.tv_sec = ts1.tv_sec - ts2.tv_sec ;
        if (ts1.tv_nsec < ts2.tv_nsec) {
            tr.tv_nsec = ts1.tv_nsec + NS_IN_SEC - ts2.tv_nsec ;
            tr.tv_sec-- ;				
        } else {
            tr.tv_nsec = ts1.tv_nsec - ts2.tv_nsec ;
        }
    }

    return (tr) ;

}


