CC =  gcc # Set the compiler
L_FLAGS = -lrt -lpthread -lm
#C_FLAGS = -g

all: pt_a2 pt_a3
.PHONY: all

# Project compilation
pt_a2: A2.c
	$(CC) $< -o $@ $(C_FLAGS) $(L_FLAGS)
pt_a3: A3.c
	$(CC) $< -o $@ $(C_FLAGS) $(L_FLAGS)
	
.PHONY: clean 

clean:
	rm -f *.c~ 
	rm -f *.o
	rm pt_a2
	rm pt_a3

# Some notes
# $@ represents the left side of the ":"
# $^ represents the right side of the ":"
# $< represents the first item in the dependency list   

