all: serial openmp hybrid

serial: 
	gcc -lm sample.c image.c gaussian.c -o serial

openmp: 
	gcc -fopenmp sample_openmp.c gaussian_openmp.c image.c -o openmp

hybrid:
	mpicc -O2 -fopenmp sample_hybrid.c gaussian_openmp.c image.c -o hybrid

clean:
	rm serial openmp hybrid cube_gray.png cube_RGB.png *.stackdump