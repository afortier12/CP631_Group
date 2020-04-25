/* 
 * CP631 - Group Project
 * Adam Fortier  
 * Ashwini Choudhari
 * Ning Ma
 * 
 * openmp.c
 * 	main program for calling openmp
 *  gaussian functions
 *  reads and writes image files
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "image.h"
#include "gaussian.h"
#include "omp.h"

int main(int argc, char* argv[]) {
    Image img, img_RGB, img_Gray;
    Matrix *mtx;
    double start, finish;
    int kernel_dim = 10;
    double kernel_sigma = 5;
    size_t kernel_size;
    float *kernel;

    /* get matrix dimension */
    if(argc == 4){
	    //printf("arguments supplied are: filename = %s kernel dimension = %s, kernel sigma = %s\n", argv[1], argv[2], argv[3]);
	    kernel_dim = atoi(argv[2]);
        kernel_sigma = atof(argv[3]);
	    if (kernel_dim == 0){
        	printf("Please enter a valid integer for kernel dimension between 5 and 100 as an argument\n");
	        return -1;
	    } else if( ((int)kernel_sigma) > (kernel_dim/2) || kernel_sigma < 3.0){
	        printf("Please enter a double for kernel sigma that is between 3 and %d (half of kernel dimension)\n", kernel_dim/2);
	        return -1;
	    }
    } else {
        printf("Please provide filename , kernel dimension and kernel sigma\nUsage: %s filename dimension sigma ,\n \twhere dimension and sigma are integeres\n", argv[0]);
	    return -1;
    }

    kernel_size = (kernel_dim*2+1)*(kernel_dim*2+1)*sizeof(float);
    kernel = (float *) malloc(kernel_size);
    
    printf("opening image\n");
    //load image from file
    if (Image_load(&img, argv[1]) != 0){
        printf("Error in loading the image\n");       
        return -1;
    }

    mtx = malloc(sizeof(Matrix));

    printf("image to matrix\n");
    // Convert the image to Matrix
    if (Image_to_Matrix(&img, mtx) != 0){
        printf("Error creating Matrix\n");       
        return -1;
    }

	//original image no longer needed
    Image_free(&img);

    //record starting time for measurement of execution time        
    start = omp_get_wtime(); 

	//call gaussian functions
    Get_Gaussian_Kernel(kernel, kernel_dim, kernel_sigma);
    Apply_Gaussian_Blur_Filter(kernel, kernel_dim, mtx);

    //determine execution time
    finish = omp_get_wtime();

    printf("matrix to RGB image\n");
    //Convert RGB matrices to image
    if (Matrix_to_RGB_Image(mtx, &img_RGB) != 0){
        printf("Error creating RGB image from matrix\n");       
        return -1;
    }
 
    printf("matrix to grayscale image\n");
    //Convert Gray matrix to image
    if (Matrix_to_Gray_Image(mtx, &img_Gray) != 0){
        printf("Error creating grayscale image from matrix\n");       
        return -1;
    }

    printf("finished in %3.7fs\n", finish-start);


    // Save images
    Image_save(&img_Gray, "cube_gray.png");
    Image_save(&img_RGB, "cube_RGB.png");
 

    // Release memory
    Image_free(&img_Gray);
    Image_free(&img_RGB);
 
    Matrix_free(mtx);
    free(kernel);

    return 0;
}
