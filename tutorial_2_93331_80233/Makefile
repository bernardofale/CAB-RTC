# Standard Makefile to buiild Xenomai 3 applications, 
#   with alchemy skin (former native skin)
# Use xeno-config to get the correct compile and link flags
# Don't forget to specify the skin
XENO_CONFIG := /usr/xenomai/bin/xeno-config
CFLAGS := $(shell $(XENO_CONFIG) --skin=alchemy --cflags)
LDFLAGS := $(shell $(XENO_CONFIG) --skin=alchemy --ldflags)
# Add -lm if math functions are necessary 
LDFLAGS += -lm  
CC := $(shell $(XENO_CONFIG) --cc)

EXECUTABLE := pt_a2 pt_a3

all: $(EXECUTABLE)

pt_a2: A2.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) 
pt_a3: A3.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) 
clean:	
	rm $(EXECUTABLE)


	
