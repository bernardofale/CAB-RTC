/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <drivers/adc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <hal/nrf_saadc.h>

/* Size of stack area used by each thread (can be thread specific, if necessary)*/
#define STACK_SIZE 1024

/* Thread scheduling priority */
#define thread_SENSOR_prio 1
#define thread_FILTER_prio 1
#define thread_OUTPUT_prio 1

/* Thread periodicity (in ms)*/
#define SAMP_PERIOD_MS 1000


/* Create thread stack space */
K_THREAD_STACK_DEFINE(thread_SENSOR_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_FILTER_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_OUTPUT_stack, STACK_SIZE);
  
/* Create variables for thread data */
struct k_thread thread_SENSOR_data;
struct k_thread thread_FILTER_data;
struct k_thread thread_OUTPUT_data;

/* Create task IDs */
k_tid_t thread_SENSOR_tid;
k_tid_t thread_FILTER_tid;
k_tid_t thread_OUTPUT_tid;

/* Thread code prototypes */
void thread_SENSOR_code(void *argA, void *argB, void *argC);
void thread_FILTER_code(void *argA, void *argB, void *argC);
void thread_OUTPUT_code(void *argA, void *argB, void *argC);
void filter(uint16_t *arr);

/* Semaphores for task synch */
struct k_sem sem_sensor_filter;
struct k_sem sem_filter_output;

/*ADC definitions and includes*/

#define ADC_RESOLUTION 10
#define ADC_GAIN ADC_GAIN_1_4
#define ADC_REFERENCE ADC_REF_VDD_1_4
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40)
#define ADC_CHANNEL_ID 1  
#define ADC_CHANNEL_INPUT NRF_SAADC_INPUT_AIN1 
#define BUFFER_SIZE 10
#define ADC_NODE DT_NODELABEL(adc)  
const struct device *adc_dev = NULL;
#define N_SAMPLES 10 /* N_SAMPLES = 1 + EXTRA_SAMPLES */

/* ADC channel configuration */
static const struct adc_channel_cfg my_channel_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id = ADC_CHANNEL_ID,
	.input_positive = ADC_CHANNEL_INPUT
};
/* Setup ADC to take 10 samples */
static const struct adc_sequence_options my_sequence_options = {
    .interval_us = ADC_ACQUISITION_TIME,
    .callback = NULL,
    .user_data = NULL,
    .extra_samplings = N_SAMPLES - 1 /* N_SAMPLES = 1 + EXTRA_SAMPLES */
};

/* GPIO definitions */
#define LED0_NODE DT_NODELABEL(led0)
#define LED1_NODE DT_NODELABEL(led1)
#define LED2_NODE DT_NODELABEL(led2)
#define LED3_NODE DT_NODELABEL(led3)

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

/* Interrupt definitions */
#define MY_DEV_IRQ  24	/* device uses IRQ 24 */
#define MY_DEV_PRIO  2	/* device uses interrupt priority 2 */ 
#define MY_ISR_ARG  DEVICE_GET(DT_NODELABEL(nvic))
#define MY_IRQ_FLAG IRQ_ZERO_LATENCY       /* IRQ flags */

/* Static Variables */
static uint16_t adc_sample_buffer[BUFFER_SIZE];
static uint16_t distance;

/* Takes ten sample */
static int adc_sample(void)
{
	int ret;
	const struct adc_sequence sequence = {
		.options = &my_sequence_options,
		.channels = BIT(ADC_CHANNEL_ID),
		.buffer = adc_sample_buffer,
		.buffer_size = sizeof(adc_sample_buffer),
		.resolution = ADC_RESOLUTION,
	};

	if (adc_dev == NULL) {
            printk("adc_sample(): error, must bind to adc first \n\r");
            return -1;
	}

	ret = adc_read(adc_dev, &sequence);
	if (ret) {
            printk("adc_read() failed with code %d\n", ret);
	}	

	return ret;
}

void main(void)
{
	int arg1=0, arg2=0, arg3=0; // Input args of tasks (actually not used in this case)
	int err;
	/* ADC setup: bind and initialize */
    adc_dev = device_get_binding(DT_LABEL(ADC_NODE));
	if (!adc_dev) {
        printk("ADC device_get_binding() failed\n");
    } 
    err = adc_channel_setup(adc_dev, &my_channel_cfg);
    if (err) {
        printk("adc_channel_setup() failed with error code %d\n", err);
    }

	/* Create and init semaphores */
    k_sem_init(&sem_sensor_filter, 0, 1);
    k_sem_init(&sem_filter_output, 0, 1);

    thread_SENSOR_tid = k_thread_create(&thread_SENSOR_data, thread_SENSOR_stack,
        K_THREAD_STACK_SIZEOF(thread_SENSOR_stack), thread_SENSOR_code,
        &arg1, &arg2, &arg3, thread_SENSOR_prio, 0, K_NO_WAIT);

    thread_FILTER_tid = k_thread_create(&thread_FILTER_data, thread_FILTER_stack,
        K_THREAD_STACK_SIZEOF(thread_FILTER_stack), thread_FILTER_code,
        NULL, NULL, NULL, thread_FILTER_prio, 0, K_NO_WAIT);

    thread_OUTPUT_tid = k_thread_create(&thread_OUTPUT_data, thread_OUTPUT_stack,
        K_THREAD_STACK_SIZEOF(thread_OUTPUT_stack), thread_OUTPUT_code,
        NULL, NULL, NULL, thread_OUTPUT_prio, 0, K_NO_WAIT);

    return;
}

/* Thread code implementation */
void thread_SENSOR_code(void *argA , void *argB, void *argC)
{
    int err;
	/* Thread loop */
    while(1) {
		printk("Thread SENSOR released\n");
        /* It is recommended to calibrate the SAADC at least once before use, and whenever the ambient temperature has changed by more than 10 °C */
    	NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;

		/* Gets all samples, checks for errors and prints the values */
        err=adc_sample();
        if(err) {
            printk("adc_sample() failed with error code %d\n\r",err);
        }
		k_sem_give(&sem_sensor_filter); //ready to be taken, count increases (unless it's not max)

        /* Periodicity of task */
        k_msleep(SAMP_PERIOD_MS);
        
    }
}

void thread_FILTER_code(void *argA , void *argB, void *argC)
{	
    while(1) {
        k_sem_take(&sem_sensor_filter,  K_FOREVER); //takes the sensor semaphore to compute something with shared memory
		printk("Thread FILTER released\n");
		for(int i = 0; i < BUFFER_SIZE; i++){
				if(adc_sample_buffer[i] > 1023) {
					adc_sample_buffer[i] = 0;
					printk("Sensor reading %d out of range\n\r", i+1);
				}
				else {
					/* ADC is set to use gain of 1/4 and reference VDD/4, so input range is 0...VDD (3 V), with 10 bit resolution */
					printk("Sensor :%4u m \n\r", (uint16_t)(100*adc_sample_buffer[i]*((float)3/1023)));
				}
			}
		filter(adc_sample_buffer);
		k_sem_give(&sem_filter_output); //ready to be taken, count increases (unless it's not max)
  	}
}

void thread_OUTPUT_code(void *argA , void *argB, void *argC)
{	
	int ret;

	/* Check if devices are ready */
	if (!device_is_ready(led1.port)) {
		return;
	}
	if (!device_is_ready(led2.port)) {
		return;
	}
	if (!device_is_ready(led3.port)) {
		return;
	}
	if (!device_is_ready(led4.port)) {
		return;
	}

	/* Configure the GPIO pin for output */
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}
	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}
	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}
	ret = gpio_pin_configure_dt(&led4, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

    while(1) {
		k_sem_take(&sem_filter_output, K_FOREVER); //takes the semaphore given by the filter task
		printk("Thread OUTPUT released\n");
		printk("Distance after filter ->%4u m \n", distance);
		if(distance >= 30){
			/* LED1 ON */
			ret = gpio_pin_set_dt(&led1, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led2, 0);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led3, 0);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led4, 0);
			if (ret < 0) {
				return;
			}
		}else if((distance >= 20) & (distance < 30)){
			/* LEDS1,2 ON */
			ret = gpio_pin_set_dt(&led1, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led2, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led3, 0);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led4, 0);
			if (ret < 0) {
				return;
			}
		}else if((distance >=  10) & (distance < 20)){
			/* LEDS1,2,3 ON */
			ret = gpio_pin_set_dt(&led1, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led2, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led3, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led4, 0);
			if (ret < 0) {
				return;
			}
		}else{
			/* LEDS1,2,3,4 ON */
			ret = gpio_pin_set_dt(&led1, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led2, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led3, 1);
			if (ret < 0) {
				return;
			}
			ret = gpio_pin_set_dt(&led4, 1);
			if (ret < 0) {
				return;
			}
		}
  	}
}

void filter(uint16_t *arr){
	uint16_t sum = 0;
	uint16_t v;
	for (int i = 0; i < N_SAMPLES; i++){
		v = (uint16_t)(100*arr[i]*((float)3/1023));
		if(v != 0){
			sum += v;
		}
	}
	uint16_t avg=(uint16_t)sum/N_SAMPLES;
	distance = avg;
}
