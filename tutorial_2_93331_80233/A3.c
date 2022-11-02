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

#define ACK_PERIOD_MS MS_2_NS(500)


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
char* read_sensor(int16_t read_line); //read sensor from file, change macro file name if needed 
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
	rt_queue_create(&sensor_queue_desc, SENSOR_QUEUE, 800, Q_UNLIMITED, Q_FIFO); 
	rt_queue_create(&processing_queue_desc, PROCESSING_QUEUE, 800, Q_UNLIMITED, Q_FIFO); 

	/* Start RT task */
	/* Args: task decriptor, address of function/implementation and argument*/
	taskAArgs.taskPeriod_ns = ACK_PERIOD_MS; 	
    rt_task_start(&sensor_task_desc, &sensor_task_code, (void *)&taskAArgs);
    //rt_task_set_affinity(&sensor_task_desc, &set);

    rt_task_start(&processing_task_desc, &processing_task_code, NULL);
    //rt_task_set_affinity(&processing_task_desc, &set);
	
	//rt_task_start(&storage_task_desc, &storage_task_code, NULL);
	rt_task_set_affinity(&storage_task_desc, &set);
	
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


int buffer[5];
int index_buffer=0;
void processing_task_code(void *args){
	int err;
	RT_QUEUE_INFO sensor_queue_info;

	void* msg;
	ssize_t len;

	err = rt_queue_bind(&sensor_queue_desc, SENSOR_QUEUE, TM_INFINITE);
	if(err){
		printf("%s", "Error binding to queue");
	}

	while((len = rt_queue_receive(&sensor_queue_desc, &msg, TM_INFINITE)) > 0){
		printf("%s -> %s\n", "RECEIVED MESSAGE", (char *)msg);
		buffer[index_buffer%5] = atoi((char *)msg);
		// printf("%s -> %d\n", "AVERAGE", index_buffer);
		index_buffer++;

		rt_queue_free(&sensor_queue_desc, msg);
		if(index_buffer>3){
			int sum = 0;
				for (int i = 0; i < 5; i++){
					sum+=buffer[i];
					printf("%s-%d -> %d\n", "VALUE",i, buffer[i]);

				}
			
			float avg=(float)sum/5;
			printf("%s -> %.3f\n", "AVERAGE", avg);

		}
	}

		
	rt_queue_unbind(&sensor_queue_desc);
	return;

}
void sensor_task_code(void *args) {
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
		data = read_sensor(line); //sensor reading
		length = strlen(data) + 1;
		//allocate message buffer to prepare the queue
		msg = rt_queue_alloc(&sensor_queue_desc, length); 
		if(!msg){
			printf("No memory available");
			break;
		}
		//copy contents of data to msg
		strcpy(msg, data);	
		printf("msg -> %s\n",(char *)msg);
		//send message
		err = rt_queue_send(&sensor_queue_desc, msg, length, Q_NORMAL);
		rt_queue_free(&sensor_queue_desc, (char *)msg);
		if(err){
			//printf("%s", "Error sending message to queue\n");
		}
		
		line++; //increment line each time the task executes
		
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
	fp = fopen("sensordata.txt", "r");
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
	return 0;
}


