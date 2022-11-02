/* ************************************************************
* Xenomai - creates a periodic task
*	
* Paulo Pedreiras
* 	Out/2020: Upgraded from Xenomai V2.5 to V3.1    
* 
************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include <sys/mman.h> // For mlockall

// Xenomai API (former Native API)
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/queue.h>

#define MS_2_NS(ms)(ms*1000*1000) /* Convert ms to ns */

/* *****************************************************
 * Define task structure for setting input arguments
 * *****************************************************/
 struct taskArgsStruct {
	 RTIME taskPeriod_ns;
	 int some_other_arg;
 };

/* *******************
 * Task attributes 
 * *******************/ 
#define TASK_MODE 0 	// No flags
#define TASK_STKSZ 0 	// Default stack size

#define ACK_PERIOD_MS MS_2_NS(2000)


RT_TASK sensor_task_desc; // Task decriptor
RT_TASK processing_task_desc;
RT_TASK storage_task_desc;

/* *******************
 * Queue attributes 
 * *******************/ 
#define SENSOR_QUEUE "Sensor_Readings"
#define PROCESSING_QUEUE "Filtered_Readings"
RT_QUEUE sensor_queue_desc;
RT_QUEUE processing_queue_desc; 

/* *******************
 * File attributes 
 * *******************/ 
#define READ_FILENAME "sensordata.txt"
#define WRITE_FILENAME "output.txt"

/* *********************
* Function prototypes
* **********************/
void catch_signal(int sig); 	/* Catches CTRL + C to allow a controlled termination of the application */
void wait_for_ctrl_c(void);
void Heavy_Work(void);      	/* Load task */
void sensor_task_code(void *args); 	/* Task body */
void storage_task_code(void *args); 	/* Task body */
void processing_task_code(void *args); 	/* Task body */
int16_t* read_sensor(int16_t read_line); //read sensor from file, change macro file name if needed 
void write_output(void); //write filtered data in output file, change macro file name if needed




/* ******************
* Main function
* *******************/ 
int main(int argc, char *argv[]) {
	int err; 
	struct taskArgsStruct taskAArgs;
	
	/* Lock memory to prevent paging */
	mlockall(MCL_CURRENT|MCL_FUTURE); 
	/* Create RT task */
	/* Args: descriptor, name, stack size, priority [0..99] and mode (flags for CPU, FPU, joinable ...) */
	err=rt_task_create(&sensor_task_desc, "Task Sensor", TASK_STKSZ, 50, 0);
	if(err) {
		printf("Error creating task Sensor (error code = %d)\n",err);
		return err;
	} else 
		printf("Task Sensor created successfully\n");
	
	
			
	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	
    
	err=rt_task_create(&processing_task_desc, "Processing", TASK_STKSZ, 50,0 );
	if(err) {
		printf("Error creating task Processing (error code = %d)\n",err);
		return err;
	} else 
		printf("Processing Task created successfully\n");
	
			
	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	
    
	err=rt_task_create(&storage_task_desc, "Task C", TASK_STKSZ, 50, 0);
	if(err) {
		printf("Error creating task Storage (error code = %d)\n",err);
		return err;
	} else 
		printf("Task Storage created successfully\n");
	
	//bound to cpu0
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);

	/*create message queue
	create sensor message queue with 100 byte poolsize, and limit 
	the number of messages to 5, since we should only have 5 samples of 2 bytes each;

	create filtered readings with 100 bytes poolsize with unlimited number of messages*/
	rt_queue_create(&sensor_queue_desc, SENSOR_QUEUE, 500, Q_UNLIMITED, Q_FIFO); 
	rt_queue_create(&processing_queue_desc, PROCESSING_QUEUE, 500, Q_UNLIMITED, Q_FIFO); 

	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	taskAArgs.taskPeriod_ns = ACK_PERIOD_MS; 	
    rt_task_start(&sensor_task_desc, &sensor_task_code, (void *)&taskAArgs);
    //rt_task_set_affinity(&sensor_task_desc, &set);

    //rt_task_start(&processing_task_desc, &processing_task_code, NULL);
    //rt_task_set_affinity(&processing_task_desc, &set);
	
	//rt_task_start(&storage_task_desc, &storage_task_code, NULL);
	//rt_task_set_affinity(&storage_task_desc, &set);
	/* wait for termination signal */	
	wait_for_ctrl_c();

	return 0;
		
}

/* ***********************************
* Task body implementation
* *************************************/
void storage_task_code(void *args){
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	struct taskArgsStruct *taskArgs;

	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	taskArgs=(struct taskArgsStruct *)args;
	printf("Task %s init, period:%llu\n", curtaskinfo.name, taskArgs->taskPeriod_ns);

	
	for(;;) {
		
		/* Task "load" */
		Heavy_Work();
}
return;
}
void processing_task_code(void *args){
	int err;
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;

	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	/* Bind to messague queues that are needed to exchange data between tasks*/
	RT_QUEUE *sensor_queue;
	RT_QUEUE *processing_queue;
	RT_QUEUE_INFO sensor_queue_info;

	//message = xnmalloc(5,2);

	
	
	err = rt_queue_bind(sensor_queue, SENSOR_QUEUE, TM_INFINITE);
	if(err) {
			printf("Can't bind to sensor queue!!\n");
		}

	err = rt_queue_bind(processing_queue, PROCESSING_QUEUE, TM_INFINITE);
	if(err) {
			printf("Can't bind to processing queue!!\n");
		}

	for(;;) {
		rt_queue_inquire(sensor_queue, &sensor_queue_info);
		if(sensor_queue_info.nmessages == 5){
			//rt_queue_read(sensor_queue, &message, 2, TM_INFINITE);
		}
		/* Task "load" */
		Heavy_Work();
	}
	return;

}
void sensor_task_code(void *args) {
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	struct taskArgsStruct *taskArgs;
	unsigned long overruns;
	int err;
	
	/* Get task information */
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	taskArgs=(struct taskArgsStruct *)args;
	printf("Task %s init, period:%llu\n", curtaskinfo.name, taskArgs->taskPeriod_ns);

	/* Message setup */
	int16_t* msg;
	int16_t line = 1; //first line to read, read next line in each exec of task
	int16_t* data; //number returned by the sensor reading
			
	/* Set task as periodic */
	err=rt_task_set_periodic(NULL, TM_NOW, ACK_PERIOD_MS);
	if(err){
		printf("error on set periodic.");
	}

	for(;;) {
		err=rt_task_wait_period(&overruns);
		if(err) {
			printf("task %s overrun!!!\n", curtaskinfo.name);
			break;
		}
		data = read_sensor(line); //sensor reading
		printf("%d\n", *data); //print value for debugging purposes
		line++; //increment line each time the task executes

		//allocate message buffer in the queue
		msg = rt_queue_alloc(&sensor_queue_desc, sizeof(int16_t));
		if(!msg){
			break;
		}
		msg = data;
		//send the message and free the messague buffer immedietly after
		rt_queue_send(&sensor_queue_desc, msg, sizeof(int16_t), Q_NORMAL);
		rt_queue_free(&sensor_queue_desc, msg);
		
	}
	return;
}

/* **************************************************************************
 *  Catch control+c to allow a controlled termination
 * **************************************************************************/
 void catch_signal(int sig)
 {
	 return;
 }

void wait_for_ctrl_c(void) {
	signal(SIGTERM, catch_signal); //catch_signal is called if SIGTERM received
	signal(SIGINT, catch_signal);  //catch_signal is called if SIGINT received

	// Wait for CTRL+C or sigterm
	pause();
	
	// Will terminate
	printf("Terminating ...\n");
}


/* **************************************************************************
 *  Task load implementation. In the case integrates numerically a function
 * **************************************************************************/
#define f(x) 1/(1+pow(x,2)) /* Define function to integrate*/
void Heavy_Work(void)
{
	float lower, upper, integration=0.0, stepSize, k;
	int i, subInterval;
	
	RTIME ts, // Function start time
		  tf; // Function finish time
			
	static int first = 0; // Flag to signal first execution		
	
	/* Get start time */
	ts=rt_timer_read();
	
	/* Integration parameters */
	/*These values can be tunned to cause a desired load*/
	lower=0;
	upper=100;
	subInterval=1000000;

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
 	if (!first) {
		tf=rt_timer_read();
		tf-=ts;  // Compute time difference form start to finish
 	
		printf("Integration value is: %.3f. It took %9llu ns to compute.\n", integration, tf);
		
		first = 1;
	}

}

int16_t* read_sensor(int16_t read_line){
	FILE *fp;
	fp = fopen("sensordata.txt", "r");
	if (fp == NULL){
		perror("Unable to open file.");
		exit(1);
	}
	char line[16];
	int16_t i = 1;
	int16_t number;
	int16_t* n;
	while(fgets(line, sizeof(line), fp) != NULL){
		if(read_line == i){
			number = atoi(line);
			n = &number;
			return n;
		}
		i++;
	}
	fclose(fp);
	return n;
}

