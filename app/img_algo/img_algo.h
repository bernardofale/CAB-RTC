#ifndef IMG_ALGO_H
#define IMG_ALGO_H
#include <zephyr.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define IMGHEIGHT 128
#define IMGWIDTH 128
#define BACKGROUND_COLOR 0x00 /* Color of the background (BLACK) */
#define GUIDELINE_COLOR 0xFF  /* Guideline color (WHITE) */
#define OBSTACLE_COLOR 0x80 /* Obstacle color (GRAY) */
/* Row to look for the guide line - FAR (Remember that that this is the top edge of the image, 
thus it is on the first line of the array) */
#define GF 0	
#define GN IMGHEIGHT	/* Row to look for the guide line - NEAR */
#define CSA_LEFT IMGWIDTH/4	/* Left limit of the sensor area */
#define CSA_RIGHT IMGWIDTH * 3/4    /* Right limit of the sensor area */
#define CSA_FRONT (IMGHEIGHT/2) /* Upper bound of the area sensor */

void guideLineSearch(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH], uint16_t* pos, float* angle);
uint8_t nearObstSearch(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH]);
uint16_t obstCount(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH]);

#endif