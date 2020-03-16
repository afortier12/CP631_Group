CC = gcc
CFLAGS = -lm

main: sample.c image.o 
	$(CC) $(CFLAGS) sample.c image.o -o main

image.o: image.c 
	$(CC) $(CFLAGS) -c image.c 

clean:
	rm *.o main