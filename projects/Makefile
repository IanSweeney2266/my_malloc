CC = gcc

CFLAGS = -Wall -pedantic -g

LD = gcc

LDFLAGS = /Users/Iansweeney/CSC453/projects

TARGET = tests

INPUTFILE = 

LIBFILES = my_malloc

all: clean shared $(TARGET) 

archive: $(LIBFILES).o
	ar r $(LIBFILES).a $(LIBFILES).o

shared: $(LIBFILES).o
	$(CC) -shared -fpic -o $(LIBFILES).so $(LIBFILES).o

$(LIBFILES).o: $(LIBFILES).c
	$(CC) $(CFLAGS) -c $(LIBFILES).c 



$(TARGET): $(TARGET).c
	export LD_LIBRARY_PATH=$(LDFLAGS):$LD_LIBRARY_PATH 
	$(CC) $(CFLAGS) -c -o $(TARGET).o $(TARGET).c
	$(CC) -L $(LDFLAGS) -o $(TARGET) $(TARGET).o $(LIBFILES).so

.PHONY: clean
clean:
	$(RM) $(TARGET) *.o *~
