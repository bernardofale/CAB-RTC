#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define IMGHEIGHT 16
#define IMGWIDTH 16
#define BACKGROUND_COLOR 0x00 /* Color of the background (BLACK) */
#define GUIDELINE_COLOR 0xFF  /* Guideline color (WHITE) */
#define OBSTACLE_COLOR 0x80 /* Obstacle color (GRAY) */
/* Row to look for the guide line - FAR (Remember that that this is the top edge of the image, 
thus it is on the first line of the array) */
#define GF 0	
#define GN IMGHEIGHT	/* Row to look for the guide line - NEAR */
#define CSA_LEFT IMGWIDTH/4	/* Col to look for near obstacles */
#define CSA_RIGHT IMGWIDTH * 3/4
#define CSA_FRONT (IMGHEIGHT/2) - 1


uint8_t buffer[IMGHEIGHT][IMGWIDTH]= 
	{	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
		{0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
		{0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00},					
		{0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00} 
	};
/* Tops to bottom (in to r) and left to right (jn to r) */
static uint16_t count_obstacles(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH], uint16_t r, uint16_t c, uint16_t in, uint16_t jn){
    /* Number of obstacles */
    uint16_t obs = 0; 
    /* Counter for obstacle pixels, counts as obstacles when at least 2 pixels long */
    uint16_t c_pixels = 0; 
    /* Iterating through the CSA, tops to bottom, left column to right column */
	for (uint16_t i = in; i < r; i++)
    {

        for (int j = jn; j < c; j++)
        {   /* If the pixel is an obstacle the pixel count increments and goes to its next iteration */    
            if(imageBuf[i][j] == OBSTACLE_COLOR){
                c_pixels++;
                continue;
            }
            /* If the pixel count is bigger than 2 it means that it is in fact a near obstacle */
            if(c_pixels >= 2) obs++;
            /* Reset the pixel count when there no pixels in the sequence */
            c_pixels = 0;
        }
        /* Same step as done above but to avoid bugs with last pixel fromn last row */
        if(c_pixels >= 2) obs++;
        
        c_pixels = 0;
    }

    return obs;
}

/* Function that detects he position and agle of the guideline */
void guideLineSearch(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH], uint16_t *pos, float *angle) {
    uint16_t pos_gf, pos_gn ; /* Positions of guide line in GF and GN */
    uint16_t x_gn = 0;
    uint16_t x_gf = GN - 1;
    /* Iterate through GN and GF to discover the position of the guidelines */
    for(uint16_t i = GF; i <= GN; i += GN - 1){
        for(uint16_t j = 0; j < IMGWIDTH; j++){
            if(imageBuf[i][j] == GUIDELINE_COLOR){
                if(i == GF){
                    pos_gf = j; /* (X,Y) = (pos_gf, gn) */
                    printf("Position in GF (Far): (%d,%d)\n", x_gf, pos_gf);
                }else{
                    pos_gn = j; /* (X,Y) = (pos_gn, 0) */ 
                    printf("Position in GN (near): (%d,%d)\n", x_gn, pos_gn);
                }
                break;
            }
        }
    }
    /* When the guideline is vertical the angle is 0 rad, positive angles are associated with a tilt to the right 
    and negative angles to a tilt to the left 
    Calculation of angle between the two points in radians */
    *angle = atan2(pos_gf - pos_gn, x_gf - x_gn);

    /* Computing of the position */
    *pos = ((pos_gn + 1) * 100) / IMGWIDTH;
}

/* Function to look for closeby obstacles */
uint16_t nearObstSearch(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH]) {
    uint16_t nobs = count_obstacles(imageBuf, IMGHEIGHT, CSA_RIGHT, CSA_FRONT, CSA_LEFT);
    return nobs;
}

/* Function that counts obstacles. */
uint16_t obstCount(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH]) {
	uint16_t obs = count_obstacles(imageBuf, IMGHEIGHT, IMGWIDTH, 0, 0);
    return obs;
}

int main(){

    uint16_t nobs;
    nobs = nearObstSearch(buffer);
    printf("Number of obstacles in the CSA: %d\n", nobs);

    uint16_t obs;
    obs = obstCount(buffer);
    printf("Number of total obstacles: %d\n", obs);
    uint16_t pos;
    float ang;
    guideLineSearch(buffer, &pos, &ang);
    printf("Angle (Radians): %f\n", ang);
    printf("Position (Percentage): %d%%\n", pos);
    return 0;
}