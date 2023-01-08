#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define WIDTH 128
#define HEIGHT 128

int main(int argc, char *argv[])
{
  // Open the input file
  FILE *fp = fopen("image.raw", "rb");
  if (!fp) {
    printf("Error opening input file\n");
    return 1;
  }

  // Allocate the image buffer
  uint8_t *buffer = malloc(WIDTH * HEIGHT);
  if (!buffer) {
    printf("Error allocating buffer\n");
    return 1;
  }

  // Read the image data into the buffer
  fread(buffer, 1, WIDTH * HEIGHT, fp);

  // Close the input file
  fclose(fp);

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
    printf("%d ", buffer[y * WIDTH + x]);
    }
    printf("%s", "\n");
  }

  // Free the image buffer
  free(buffer);

  return 0;
}
