# Compiler to use
CC=gcc

# Compiler flags
CFLAGS=-Wall

# Name of your final executable
TARGET=printHeap

all: $(TARGET)

$(TARGET): myMalloc.o printing.o test.o
	$(CC) $(CFLAGS) myMalloc.o printing.o test.o -o $(TARGET)

myMalloc.o: myMalloc.c myMalloc.h
	$(CC) $(CFLAGS) -c myMalloc.c

printing.o: printing.c printing.h
	$(CC) $(CFLAGS) -c printing.c

test.o: test.c myMalloc.h printing.h
	$(CC) $(CFLAGS) -c test.c

clean:
	rm -f *.o $(TARGET)
	rm -f ../results/*.txt $(TARGET)
	rm -f ../results/*.json $(TARGET)