/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "cab.h"


void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	cab* cab;
	uint16_t first = 5; 
	cab = open_cab("CAB", 1, 2, &first);
}
