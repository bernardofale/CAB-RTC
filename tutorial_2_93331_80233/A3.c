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
#define WRITE_FILENAME "sensordataFiltered.txt"

/* *********************
* Function prototypes
* **********************/
void catch_signal(int sig); 	/* Catches CTRL + C to allow a controlled termination of the application */
void wait_for_ctrl_c(void);
void Heavy_Work(void);      	/* Load task */

void sensor_task_code(void *args); 	/* Task body */
void storage_task_code(void *args); 	/* Task body */
void processing_task_code(void *args); 	/* Task body */
char* read_sensor(int16_t read_line); /* read sensor from file, change macro file name if needed */
void write_output(uint16_t value); /* write filtered data in output file, change macro file name if needed */
uint16_t* filter(uint16_t *arr); /* create the point moving average and filter the sensor readings */




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
	err=rt_task_create(&sensor_task_desc, "Sensor", TASK_STKSZ, 80, 0);
	if(err) {
		printf("Error creating task Sensor (error code = %d)\n",err);
		return err;
	} else 
		printf("Task Sensor created successfully\n");
	
	
			
	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	
    
	err=rt_task_create(&processing_task_desc, "Processing", TASK_STKSZ, 80,0 );
	if(err) {
		printf("Error creating task Processing (error code = %d)\n",err);
		return err;
	} else 
		printf("Processing Task created successfully\n");
	
			
	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	
    
	err=rt_task_create(&storage_task_desc, "Storing", TASK_STKSZ, 80, 0);
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
	rt_queue_create(&sensor_queue_desc, SENSOR_QUEUE, 800, Q_UNLIMITED, Q_FIFO); 
	rt_queue_create(&processing_queue_desc, PROCESSING_QUEUE, 800, Q_UNLIMITED, Q_FIFO); 

	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	taskAArgs.taskPeriod_ns = ACK_PERIOD_MS; 	
    rt_task_start(&sensor_task_desc, &sensor_task_code, (void *)&taskAArgs);
    //rt_task_set_affinity(&sensor_task_desc, &set);

    /* Should sporadic tasks be always executing or should they start in sensor task function?
       Should they be bounded to one CPU? */
    rt_task_start(&processing_task_desc, &processing_task_code, NULL);
    //rt_task_set_affinity(&processing_task_desc, &set);
	
	rt_task_start(&storage_task_desc, &storage_task_code, NULL);
	//rt_task_set_affinity(&storage_task_desc, &set);
	
	/* wait for termination signal */	
	wait_for_ctrl_c();

	return 0;
		
}

/* ***********************************
* Task body implementation
* *************************************/
void storage_task_code(void *args){
	
	int err;
	RTIME ta=0,tf=0,min=(RTIME)INFINITY,max=0;
	RTIME tiat = 0, ta_ant = 0; 
	RTIME min_tiat = (RTIME) INFINITY;
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	/* Variable declarations for filter*/
	void* msg;
	ssize_t len;
	
	/* Bind to queue, receive message from the processing task and free the message buffer;
	   Write to file */
	
	err = rt_queue_bind(&processing_queue_desc, PROCESSING_QUEUE, TM_INFINITE);
	if(err){
		printf("%s", "Error binding to queue");
	}

	/* Wait for message and free the message buffer*/
	while((len = rt_queue_receive(&processing_queue_desc, &msg, TM_INFINITE)) > 0){
		ta=rt_timer_read();
		tiat = ta - ta_ant;
		if(tiat < min_tiat) min_tiat = tiat;
		uint16_t *val = (uint16_t*) msg;
		rt_queue_free(&processing_queue_desc, msg);
		printf("%s -> %d\n", "STORAGE RECEIVED MESSAGE FROM PROCESSING", *val);
		write_output(*val);
		ta_ant = ta;
		tf=rt_timer_read()-ta;
		if(tf<min) min = tf;
		if(tf>max) max = tf;
		printf("Task %s - minimo : %llu , máximo : %llu, Min T inter-arrival time : %llu \n", curtaskinfo.name,min,max,min_tiat);
	}
	
	rt_queue_unbind(&processing_queue_desc);

	return;
}

#define N_SAMPLES 5 //Number of samples for moving average filter, change if needed

void processing_task_code(void *args){
	int err;
	RT_QUEUE_INFO sensor_queue_info;
	RTIME ta=0,tf=0,min=(RTIME)INFINITY,max=0;
	RTIME min_tiat = (RTIME) INFINITY, tiat = 0, ta_ant = 0; 
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	/* Variable declarations for filter*/
	uint16_t buffer[N_SAMPLES];
	int index_buffer=0;
	void* msg;
	uint16_t* storage_msg;
	ssize_t len;
	uint16_t* average;

	/* Bind to sensor queue to receive message */
	err = rt_queue_bind(&sensor_queue_desc, SENSOR_QUEUE, TM_INFINITE);
	if(err){
		printf("%s", "Error binding to queue");
	}

	/* Wait for message from sensor readings task */
	while((len = rt_queue_receive(&sensor_queue_desc, &msg, TM_INFINITE)) > 0){
		ta=rt_timer_read();
		tiat = ta - ta_ant;
		if(tiat < min_tiat) min_tiat = tiat;
		printf("%s -> %s\n", "PROCESSING RECEIVED MESSAGE FROM SENSOR", (char *)msg);
		buffer[index_buffer%N_SAMPLES] = atoi((char *)msg);
		index_buffer++;

		rt_queue_free(&sensor_queue_desc, msg);
		/* Get value from filter function so if other filter is applied we don't need to change in here */
		if(index_buffer > N_SAMPLES-1){
			average = filter(buffer);
			//printf("%s -> %d\n", "AVERAGE", *average);
			
			/* Prepare memory to send message to storage */
			storage_msg = rt_queue_alloc(&processing_queue_desc, sizeof(uint16_t*)); 
			if(!storage_msg){
				printf("No memory available");
				break;
			}
			/* Copy contents of average to message and send to the processing queue, so it can be retrieved by
			   the storage task*/
			*storage_msg = *average;
			printf("PROCESSING SEND MESSAGE TO STORAGE-> %d\n", *storage_msg);
			err = rt_queue_send(&processing_queue_desc, storage_msg, sizeof(storage_msg), Q_NORMAL);
			if(err){
				//printf("%s", "Error sending message to storage\n");
			}
			
			rt_queue_free(&processing_queue_desc, (uint16_t *)msg);
			
			}
			ta_ant = ta;
			tf=rt_timer_read()-ta;
			if(tf<min) min = tf;
			if(tf>max) max = tf;
			printf("Task %s - minimo : %llu , máximo : %llu, Min T inter-arrival time : %llu \n", curtaskinfo.name,min,max, min_tiat);
		}	
	rt_queue_unbind(&sensor_queue_desc);
	return;

}
void sensor_task_code(void *args) {
	RTIME ta=0,tf=0,min=(RTIME)INFINITY,max=0;
	RTIME min_tiat = (RTIME) INFINITY, tiat = 0, ta_ant = 0; 
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	RT_QUEUE_INFO queueinfo;
	struct taskArgsStruct *taskArgs;
	unsigned long overruns;
	int err;
	
	/* Get task information */
	curtask=rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
	taskArgs=(struct taskArgsStruct *)args;
	printf("Task %s init, period:%llu\n", curtaskinfo.name, taskArgs->taskPeriod_ns);

	/* Message setup */
	void *msg;
	int16_t line = 1; //first line to read, read next line in each exec of task
	char *data; //number returned by the sensor reading
	int length;
	
	/* Set task as periodic */
	err=rt_task_set_periodic(NULL, TM_NOW, ACK_PERIOD_MS);
	if(err){
		printf("Error on set periodic.");
	}

	for(;;) {
		err=rt_task_wait_period(&overruns);
		if(err) {
			printf("task %s overrun!!!\n", curtaskinfo.name);
			break;
		}
		ta=rt_timer_read();
		tiat = ta - ta_ant;
		if(tiat < min_tiat) min_tiat = tiat;
		/* Get value from the read_sensor() function and calculate its length for allocation purposes*/
		data = read_sensor(line); 
		length = strlen(data) + 1;
		
		/* Allocate message buffer to prepare the queue */
		msg = rt_queue_alloc(&sensor_queue_desc, length); 
		if(!msg){
			printf("No memory available");
			break;
		}
		/* Copy contents of data to msg */
		strcpy(msg, data);	
		printf("%s\n","***********************************************************************************");
		printf("SEND MESSAGE TO PROCESSING-> %s\n",(char *)msg);
		
		/* Send message and free the queue message buffer */
		err = rt_queue_send(&sensor_queue_desc, msg, length, Q_NORMAL);
		rt_queue_free(&sensor_queue_desc, (char *)msg);
		ta_ant = ta;
		line++; //increment line each time the task executes
		tf=rt_timer_read()-ta;
		if(tf<min) min = tf;
		if(tf>max) max = tf;
		printf("Task %s - minimo : %llu , máximo : %llu, Min T inter-arrival time : %llu \n", curtaskinfo.name,min,max,min_tiat);
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

char* read_sensor(int16_t read_line){

	//create file and open
	FILE *fp;
	fp = fopen(READ_FILENAME, "r");
	if (fp == NULL){
		perror("Unable to open file.");
		exit(1);
	}

	char *line;
	line = malloc(sizeof(char) * 15);
	int16_t i = 1;
	
	while(fgets(line, sizeof(line), fp) != NULL){
		if(read_line == i){
			return line;
		}
		i++;
	}
	fclose(fp);
	printf("%s", "No more readings!\n******************\nCLOSING PROGRAM\n******************\n");
	_Exit(0);
	return "EXIT";
}

uint16_t* filter(uint16_t *arr){
	uint16_t *average;
	int sum = 0;
	for (int i = 0; i < N_SAMPLES; i++){
		sum+=arr[i];

	}

	uint16_t avg=(uint16_t)sum/N_SAMPLES;
	average = &avg;

	return average;
}

void write_output(uint16_t value){

	FILE *fp;
	fp = fopen(WRITE_FILENAME, "a");
	if (fp == NULL){
		perror("Unable to open file.");
		exit(1);
	}
	fprintf(fp,"%u\n", value);
	fclose(fp);
}


