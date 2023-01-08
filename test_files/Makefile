CC =  gcc # Set the compiler
C_FLAGS = -c

all: test.o cab.o program
.PHONY: all

# Project compilation
test.o: test.c
	$(CC) $(C_FLAGS) $< 
cab.o: cab.c
	$(CC) $(C_FLAGS) $< 
program: cab.o test.o 
	$(CC) $^ -o $@ 
	
.PHONY: clean 

clean:
	rm -f *.c~ 
	rm -f *.o
	rm program


# Some notes
# $@ represents the left side of the ":"
# $^ represents the right side of the ":"
# $< represents the first item in the dependency list   


	
