/* *******************************************************************
 * SOTR 22-23
 * Paulo Pedreiras, Nov. 2022
 * nRF boards register as /dev/ttyACMx (usually ACM0)
 * Default config is 115200,8,n,1
 * Can try it with any nRF example that outputs text to the terminal
 * and/or with the console example
 * 
 * Adapted form:  
 *  LINUX SERIAL PORTS USING C/C++
 *  Linux Serial Ports Using C/C++
 *  Article by:Geoffrey Hunter
 *  Date Published:	June 24, 2017
 *  Last Modified:	October 3, 2022
 *  https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
 * 
 * Suggested reading:
 *  The TTY demystified, by Linus Åkesson
 *  Linus Åkesson homepage/blog, at (2022/11):
 *  https://www.linusakesson.net/programming/tty/index.php
 * 
 ******************************************************************** */


// C library headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 128
#define HEIGHT 128
// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#define SERIAL_PORT "/dev/tty.usbmodem0006839528221"

int main() {
	

    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open(SERIAL_PORT, O_RDWR);

    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(serial_port, &tty) != 0) {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  
    /* Changing this settings so the read function blocks until all characters have been read */
    tty.c_cc[VTIME] = -1;    
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
    }
    /* Variables to determine the period of image uploading */
    clock_t start, end;
    double elapsed;
    double max = 0;
    int y;
    for (int i = 1; i < 100; i++)
    {
      srand(time(NULL));
      start = clock();
      char file_name[128];
      sprintf(file_name, "images/img%d.raw", i);
      // Open the input file
      FILE *fp = fopen(file_name, "rb");
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
      // Set everything to 0x00
      memset(buffer, 0x00, WIDTH * HEIGHT * sizeof(uint8_t));

      // Set one random 0xFF byte in a random column of the first row
      int y = rand() % HEIGHT;
      buffer[0 * 128 + y] = 0xFF;

      // Set one random 0xFF byte in a random column of the last row
      y = rand() % HEIGHT;
      buffer[(WIDTH - 1) * 128 + y] = 0xFF;

    
      int ret = write(serial_port, buffer, 128*128);
      end = clock();
      printf("Uploaded %s...\n", file_name);
      elapsed = (double)(end - start) / CLOCKS_PER_SEC;
      
      if(max < elapsed){
        max = elapsed;
        printf("MAX elapsed time: %f seconds\n", max); 
      }
      sleep(10);
    }       
    //MAX elapsed time with image reading: 900 micro seconds
    //MAX elapsed time without image reading: 252 micro seconds
    
    close(serial_port);
    return 0; // success
};