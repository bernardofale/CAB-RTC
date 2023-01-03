CC =  gcc # Set the compiler
C_FLAGS = -c

all: serialTest.c serialTest
.PHONY: all

# Project compilation 
serialTest: serialTest.c 
	$(CC) $^ -o $@ 
	
.PHONY: clean 

clean:
	rm serialTest


# Some notes
# $@ represents the left side of the ":"
# $^ represents the right side of the ":"
# $< represents the first item in the dependency list   


	
