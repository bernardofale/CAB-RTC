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
#define SAMP_PERIOD_MS 500


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
#define N_SAMPLES 10 /* N_SAMPLES = 1 + EXTRA_SAMPLES | should be n_samples - 1 but first reading out of bounds */

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
    .extra_samplings = N_SAMPLES - 1 /* N_SAMPLES = 1 + EXTRA_SAMPLES | should be n_samples - 1 but first reading out of bounds */
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

/* Test button definitions */
#define BUTTON_NODE DT_NODELABEL(button0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback cb_data;
static bool test_flag = false;
struct k_timer my_timer;

/* Static Variables */
static uint16_t adc_sample_buffer[BUFFER_SIZE];
static uint16_t distance;

/* Interrupt Handler */

gpio_callback_handler_t button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	test_flag = true;
	return;
}

/* Takes ten samples */
static int adc_sample(void)
{
	int ret;
	/* Create sequence with options gonfigured previously */
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
	/* Reads the samples */
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

	/* Interrupt configure and adding callbacks */
	if (!device_is_ready(button.port)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return;
	}
	err = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_INT_DEBOUNCE );
	if (err < 0) {
		return;
	}
	err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (err < 0) {
		return;
	}
	 gpio_init_callback(&cb_data, button_pressed, BIT(button.pin));
	 gpio_add_callback(button.port, &cb_data);

	/* Define timer for test functionality */
	k_timer_init(&my_timer, NULL, NULL);

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

	/* Configure the GPIO pins for output */
	err = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		return;
	}
	err = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		return;
	}
	err = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		return;
	}
	err = gpio_pin_configure_dt(&led4, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		return;
	}

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

	/* Creation of tasks */
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
	uint16_t start;
	uint16_t end;
	uint16_t wc_exec_time = 10000;
	/* Thread loop */
    while(1) {
		if(test_flag){
			k_timer_start(&my_timer, K_MSEC(5000), K_NO_WAIT);
			while(k_timer_status_get(&my_timer) <= 0){
				if(k_uptime_get() % 500 == 0){
					printk("BLINK\n");
					err = gpio_pin_toggle_dt(&led1);
					if (err < 0) {
						return;
					}
					err = gpio_pin_toggle_dt(&led2);
					if (err < 0) {
						return;
					}
					err = gpio_pin_toggle_dt(&led3);
					if (err < 0) {
						return;
					}
					err = gpio_pin_toggle_dt(&led4);
					if (err < 0) {
						return;
					}
				}
			}
			test_flag = 0;
		}else{
		start = k_uptime_get(); /* start calculating exec times */
		printk("Thread SENSOR released\n");
		/* It is recommended to calibrate the SAADC at least once before use, and whenever the ambient temperature has changed by more than 10 °C */
		NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;

		/* Gets all samples */
		err=adc_sample();
		if(err) {
			printk("adc_sample() failed with error code %d\n\r",err);
		}
		
		k_sem_give(&sem_sensor_filter); //ready to be taken the processing task, count increases (unless it's not max)
		end = k_uptime_get();
		if(wc_exec_time > (end-start)) wc_exec_time = (end - start);
		printk("Ci : %4u\n", wc_exec_time);
		/* Periodicity of task */
		k_msleep(SAMP_PERIOD_MS);
		}  
	} 
}

void thread_FILTER_code(void *argA , void *argB, void *argC)
{	
	uint16_t start;
	uint16_t end;
	uint16_t wc_exec_time = 10000;
	uint16_t last = 5000; 
	uint16_t min_iat = 10000;
	
    while(1) {

		k_sem_take(&sem_sensor_filter,  K_FOREVER); /* takes the sensor semaphore to compute something with the shared memory */
		start = k_uptime_get(); /* start calculating exec times */
		printk("Thread FILTER released\n");
		for(int i = 1; i < BUFFER_SIZE ; i++){
				if(adc_sample_buffer[i] > 1023) {
					printk("Sensor reading %d out of range: %d\n\r", i+1, (uint16_t)adc_sample_buffer[i]);
					adc_sample_buffer[i] = 0;
					
				}
				else {
					/* ADC is set to use gain of 1/4 and reference VDD/4, so input range is 0...VDD (3 V), with 10 bit resolution */
					printk("Sensor %d :%4u m \n\r", i, (uint16_t)(10*adc_sample_buffer[i]*((float)3/1023)));
				}
			}
		/* Computes filtering */
		filter(adc_sample_buffer);
		k_sem_give(&sem_filter_output); //ready to be taken by the output task, count increases (unless it's not max)
		end = k_uptime_get();
		
		if(wc_exec_time > (end-start)) wc_exec_time = (end - start);
		if(min_iat > last) min_iat = start - last;
		last = end;
		printk("Ci : %4u\n", wc_exec_time);
		printk("MIN_IAT : %4u\n", min_iat);
  	}
}

void thread_OUTPUT_code(void *argA , void *argB, void *argC)
{	
	int ret;
	uint16_t start;
	uint16_t end;
	uint16_t wc_exec_time = 1000;
	uint16_t last = 5000;
	uint16_t min_iat = 100000;
    while(1) {

		k_sem_take(&sem_filter_output, K_FOREVER); //takes the semaphore given by the filter task
		start = k_uptime_get();
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
		end = k_uptime_get();
		if(wc_exec_time > (end-start)) wc_exec_time = (end - start);
		if(min_iat > last) min_iat = start - last;
		last = end;
		printk("Ci : %4u\n", wc_exec_time);
		printk("MIN_IAT : %4u\n", min_iat);
		
  	}
}

void filter(uint16_t *arr){
	uint16_t sum = 0;
	uint16_t v;
	for (int i = 1; i < N_SAMPLES + 1; i++){
		v = (uint16_t)(10*arr[i]*((float)3/1023));
		if(v != 0){
			sum += v;
		}
	}
	uint16_t avg=(uint16_t)sum/N_SAMPLES;
	distance = avg;
}