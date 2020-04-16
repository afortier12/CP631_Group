#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "image.h"
#include "gaussian.h"
#include "omp.h"

#define kernel_dim 10
#define kernel_sigma 5
#define kernel_size ((kernel_dim*2+1)*(kernel_dim*2+1))
float kernel[kernel_size];

int main(int argc, char* argv[]) {
    Image img, img_RGB, img_Gray;
    Matrix *mtx;
    double start, finish;

    
    
    printf("opening image\n");
    //load image from file
    if (Image_load(&img, "cube_640x480.png") != 0){
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

    //printf("width=%d, height=%d\n", mtx->width, mtx->height);
    //record starting time for measurement of execution time        
    start = omp_get_wtime(); 

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

    return 0;
}
