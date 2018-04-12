CC = gcc

CFLAGS = -Wall -g -fpic

LD = gcc

LDFLAGS = /Users/Iansweeney/CSC453/projects/lib64

TARGET = tests

LIBFILES = my_malloc

all: clean intel-all $(TARGET) 


intel-all: lib/libmalloc.so lib64/libmalloc.so

lib/libmalloc.so: lib malloc32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ malloc32.o

lib64/libmalloc.so: lib64 malloc64.o
	$(CC) $(CFLAGS) -shared -o $@ malloc64.o

lib:
	mkdir lib

lib64:
	mkdir lib64

malloc32.o: my_malloc.c
	$(CC) $(CFLAGS) -m32 -c -o malloc32.o my_malloc.c

malloc64.o: my_malloc.c
	$(CC) $(CFLAGS) -m64 -c -o malloc64.o my_malloc.c


$(TARGET): $(TARGET).c
	export LD_LIBRARY_PATH=$(LDFLAGS):$LD_LIBRARY_PATH 
	$(CC) $(CFLAGS) -c -o $(TARGET).o $(TARGET).c
	$(CC) -L $(LDFLAGS) -o $(TARGET) $(TARGET).o ./lib64/libmalloc.so


.PHONY: clean
clean:
	$(RM) $(TARGET) *.o *~
