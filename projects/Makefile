CC = gcc

CFLAGS = -Wall -pedantic -g

LD = gcc

LDFLAGS = -L/Users/Iansweeney/CSC453/projects

TARGET = tests

INPUTFILE = 

LIBFILES = my_malloc

all: clean archive $(TARGET) 

archive: $(LIBFILES).o
	ar r $(LIBFILES).a $(LIBFILES).o

shared:
	$(CC) -shared -fpic -o $(LIBFILES).so $(LIBFILES).o

$(LIBFILES).o: $(LIBFILES).c
	$(CC) $(CFLAGS) -c $(LIBFILES).c 



$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS) $(LIBFILES)

.PHONY: clean
clean:
	rm $(TARGET) *.o *~
