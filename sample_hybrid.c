#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "image.h"
#include "gaussian.h"

#define kernel_dim 10
#define kernel_sigma 5
#define kernel_size ((kernel_dim*2+1)*(kernel_dim*2+1))
float kernel[kernel_size];

int main(int argc, char* argv[]) {
    Image img, img_RGB, img_Gray;
    Matrix *mtx, *local_mtx, *output_mtx;
	int scaled;
    int rank, np;

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    double start = MPI_Wtime();
    // Do something here
    double end = MPI_Wtime();

    if (rank == 0) {
        printf("opening image\n");
        //load image from file
        if (Image_load(&img, "cube.png") != 0){
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

        Get_Gaussian_Kernel(kernel, kernel_dim, kernel_sigma);
    }
     
    MPI_Bcast(&kernel, kernel_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mtx->width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mtx->height, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    printf("Matrix width in Process %d: %d\n", rank, mtx->width);
    
    output_mtx = malloc(sizeof(Matrix));
    output_mtx->width = mtx->width;
    output_mtx->height = mtx->height;
    output_mtx->channels = mtx->channels;
    output_mtx->A = mtx->A;
    output_mtx->R = malloc(mtx->width * mtx->height);
    output_mtx->G = malloc(mtx->width * mtx->height);
    output_mtx->B = malloc(mtx->width * mtx->height);
    output_mtx->Gy = malloc(mtx->width * mtx->height);
    
    local_mtx = malloc(sizeof(Matrix));
    local_mtx->width = mtx->width;
    local_mtx->height = mtx->height / np;

    local_mtx->R = malloc(mtx->width * mtx->height / np * sizeof(uint8_t));
	local_mtx->G = malloc(mtx->width * mtx->height / np * sizeof(uint8_t));
	local_mtx->B = malloc(mtx->width * mtx->height / np * sizeof(uint8_t));
	local_mtx->Gy = malloc(mtx->width * mtx->height / np * sizeof(uint8_t));
    printf("Local Matrix Size in Process %d: %d * %d\n", rank, local_mtx->width, local_mtx->height);

    MPI_Scatter(mtx->R, mtx->width * mtx->height / np, MPI_UINT8_T, local_mtx->R, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Scatter(mtx->G, mtx->width * mtx->height / np, MPI_UINT8_T, local_mtx->G, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Scatter(mtx->B, mtx->width * mtx->height / np, MPI_UINT8_T, local_mtx->B, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Scatter(mtx->Gy, mtx->width * mtx->height / np, MPI_UINT8_T, local_mtx->Gy, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);

    Apply_Gaussian_Blur_Filter(kernel, kernel_dim, local_mtx);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Gather(local_mtx->R, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->R, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gather(local_mtx->G, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->G, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gather(local_mtx->B, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->B, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gather(local_mtx->Gy, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->Gy, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        // printf("Width: %d    Height: %d\n", output_mtx->width, output_mtx->height);
        // printf("======== %u\n", output_mtx->R[output_mtx->width * output_mtx->height - 1]);

        printf("matrix to RGB image\n");
        //Convert RGB matrices to image
        if (Matrix_to_RGB_Image(output_mtx, &img_RGB) != 0){
            printf("Error creating RGB image from matrix\n");       
            return -1;
        }
 
        printf("matrix to grayscale image\n");
        // Convert Gray matrix to image
        if (Matrix_to_Gray_Image(output_mtx, &img_Gray) != 0){
            printf("Error creating grayscale image from matrix\n");       
            return -1;
        }

        // Save images
        Image_save(&img_Gray, "cube_gray.png");
        Image_save(&img_RGB, "cube_RGB.png");
    }

    MPI_Finalize();
    return 0;
}