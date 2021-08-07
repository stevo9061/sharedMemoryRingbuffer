CC = gcc
CFLAGS = -g -Wall -Wpedantic -Wextra
EXE = write read

all: 
	gcc $(CFLAGS) -o write write.c -lrt -lpthread
	gcc $(CFLAGS) -o read read.c -lrt -lpthread
	
clean:
	rm -f *.dSYM *.o $(EXE)
