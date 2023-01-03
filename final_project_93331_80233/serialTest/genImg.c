#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 128 
#define HEIGHT 128
#define MAX_OBJECTS 10

unsigned char* gen_image() {
  // Allocate memory for the image
  unsigned char* image = (unsigned char*) malloc(WIDTH * HEIGHT * sizeof(unsigned char));
  srand(time(NULL));
  // Generate image data
  int objects = rand() % MAX_OBJECTS;
  for (int y = 0; y < HEIGHT; y++) {
    int objectRow = rand() % 100;
    int ff = rand() % WIDTH;
    int obj_cord = rand() % (WIDTH-1); 
    while(ff == obj_cord || ff == obj_cord+1 || ff == obj_cord-1) {
        obj_cord = rand() % (WIDTH-1);        
    }
    
    for (int x = 0; x < WIDTH; x++) {
      if (x == ff) {
        image[y * WIDTH + x] = 0xFF;
      }else if((x==obj_cord || x==obj_cord+1 ) && (objects > 0) && (objectRow > 80) ){
        image[y * WIDTH + x] = 0x80;
        // printf(" %d : %d \n", x ,image[y * WIDTH + x]);
        if(x==obj_cord+1){
            objects--;
            // printf("i have been called \n");    
        }
      }else{
        image[y * WIDTH + x] = 0x00;
      }
      printf("%d",image[y * WIDTH + x]);
    }
    printf("\n");
  }

  return image;
}

int main(int argc, char** argv) {
    unsigned char* image = gen_image();
    FILE* fp = fopen(argv[1], "wb");
    fwrite(image, sizeof(unsigned char), WIDTH * HEIGHT, fp);
    fclose(fp);
    usleep(200000);
    return 0;
}