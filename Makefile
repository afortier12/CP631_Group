all: serial openmp mpi hybrid

serial: 
	gcc -lm main.c image.c gaussian.c -o serial

openmp: 
	gcc -fopenmp -lm openmp.c gaussian_openmp.c image.c -o openmp

mpi:
	mpicc -O2 -lm mpi.c gaussian.c image.c -o mpi

hybrid:
	mpicc -O2 -lm -fopenmp mpi.c gaussian_openmp.c image.c -o hybrid

clean:
	rm serial openmp mpi hybrid cube_gray.png cube_RGB.png *.stackdump