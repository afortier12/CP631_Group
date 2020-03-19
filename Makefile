CC = gcc
CFLAGS = -lm

main: sample.c image.o gaussian.o
	$(CC) $(CFLAGS) sample.c image.o gaussian.o -o main

image.o: image.c 
	$(CC) $(CFLAGS) -c image.c 
	
gaussian.o: gaussian.c
	$(CC) $(CFLAGS) -c gaussian.c 

clean:
	rm *.o main cube_gray.png cube_RGB.png