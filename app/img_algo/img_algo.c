#include "img_algo.h"

/* Function that detects he position and agle of the guideline */
void guideLineSearch(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH], uint16_t* pos, float* angle) {
    uint16_t pos_gf, pos_gn ; /* Positions of guide line in GF and GN */
    uint16_t x_gn = 0;
    uint16_t x_gf = GN - 1;
    /* Iterate through GN and GF to discover the position of the guidelines */
    for(uint16_t i = GF; i <= GN; i += GN - 1){
        for(uint16_t j = 0; j < IMGWIDTH; j++){
            if(imageBuf[i][j] == GUIDELINE_COLOR){
                if(i == GF){
                    pos_gf = j; /* (X,Y) = (pos_gf, gn) */
                }else{
                    pos_gn = j; /* (X,Y) = (pos_gn, 0) */ 
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
uint8_t nearObstSearch(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH]) {
    uint8_t flag = 0;
    /* Iterating through the CSA, tops to bottom, left column to right column */
	for (uint16_t i = CSA_FRONT; i < IMGHEIGHT; i++)
    {
        for (int j = CSA_LEFT; j < CSA_RIGHT; j++)
        {   /* If the pixel is an obstacle returns 1 */    
            if(imageBuf[i][j] == OBSTACLE_COLOR){
                flag = 1;
                return flag;
            }
        }
    }
    return flag;
}

/* Function that counts obstacles. */
uint16_t obstCount(uint8_t imageBuf[IMGHEIGHT][IMGWIDTH]) {
    /* Number of obstacles */
    uint16_t obs = 0; 
    /* Counter for obstacle pixels, counts as obstacles when at least 2 pixels long */
    uint16_t c_pixels = 0; 
    /* Iterating through the CSA, tops to bottom, left column to right column */
	for (uint16_t i = 0; i < IMGHEIGHT; i++)
    {

        for (int j = 0; j < IMGWIDTH; j++)
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