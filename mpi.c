#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "image.h"
#include "gaussian.h"

#define kernel_dim 30
#define kernel_sigma 5
#define kernel_size ((kernel_dim*2+1)*(kernel_dim*2+1))
float kernel[kernel_size];

int main(int argc, char* argv[]) {
    Image img, img_RGB, img_Gray;
    Matrix *mtx, *local_mtx, *output_mtx;
    int rank, np, i;
    MPI_Status status;
    double start, end;

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    mtx = malloc(sizeof(Matrix));

    if (rank == 0) {
        printf("opening image\n");
        //load image from file
        if (Image_load(&img, "cube_1620x1215.png") != 0){
            printf("Error in loading the image\n");       
            return -1;
        }

        printf("image to matrix\n");
        // Convert the image to Matrix
        if (Image_to_Matrix(&img, mtx) != 0){
            printf("Error creating Matrix\n");       
            return -1;
        }

	    //original image no longer needed
        Image_free(&img);

        Get_Gaussian_Kernel(kernel, kernel_dim, kernel_sigma);

        mtx->height = mtx->height - mtx->height % np;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
     
    MPI_Bcast(&kernel, kernel_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mtx->width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mtx->height, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    // printf("Matrix width in Process %d: %d\n", rank, mtx->width);
    
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
    local_mtx->height = mtx->height / np + 4 * kernel_dim; // With stradles

    size_t local_mtx_size = local_mtx->width * local_mtx->height * sizeof(uint8_t);
    local_mtx->R = malloc(local_mtx_size);
	local_mtx->G = malloc(local_mtx_size);
	local_mtx->B = malloc(local_mtx_size);
	local_mtx->Gy = malloc(local_mtx_size);
    // printf("Local Matrix Size in Process %d: %d * %d\n", rank, local_mtx->width, local_mtx->height);

    if (rank == 0) {
        local_mtx->R = mtx->R;
        local_mtx->G = mtx->G;
        local_mtx->B = mtx->B;
        local_mtx->Gy = mtx->Gy;

        for (i=1; i<np; i++) {
            if (i<np-1) {
                MPI_Send(mtx->R + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height, MPI_UINT8_T, i, 0, MPI_COMM_WORLD);
                MPI_Send(mtx->G + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height, MPI_UINT8_T, i, 1, MPI_COMM_WORLD);
                MPI_Send(mtx->B + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height, MPI_UINT8_T, i, 2, MPI_COMM_WORLD);
                MPI_Send(mtx->Gy + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height, MPI_UINT8_T, i, 3, MPI_COMM_WORLD);
            } else {
                MPI_Send(mtx->R + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, i, 0, MPI_COMM_WORLD);
                MPI_Send(mtx->G + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, i, 1, MPI_COMM_WORLD);
                MPI_Send(mtx->B + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, i, 2, MPI_COMM_WORLD);
                MPI_Send(mtx->Gy + mtx->width * mtx->height / np * i - mtx->width * kernel_dim * 2, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, i, 3, MPI_COMM_WORLD);
            }
        }
    } else {
        if (rank < np-1) {
            MPI_Recv(local_mtx->R, local_mtx->width * local_mtx->height, MPI_UINT8_T, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(local_mtx->G, local_mtx->width * local_mtx->height, MPI_UINT8_T, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(local_mtx->B, local_mtx->width * local_mtx->height, MPI_UINT8_T, 0, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(local_mtx->Gy, local_mtx->width * local_mtx->height, MPI_UINT8_T, 0, 3, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(local_mtx->R, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(local_mtx->G, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(local_mtx->B, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, 0, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(local_mtx->Gy, local_mtx->width * local_mtx->height - mtx->width * kernel_dim * 2, MPI_UINT8_T, 0, 3, MPI_COMM_WORLD, &status);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    Apply_Gaussian_Blur_Filter(kernel, kernel_dim, local_mtx);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        MPI_Gather(local_mtx->R, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->R, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        MPI_Gather(local_mtx->G, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->G, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        MPI_Gather(local_mtx->B, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->B, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        MPI_Gather(local_mtx->Gy, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->Gy, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    } else {
        MPI_Gather(local_mtx->R + mtx->width * kernel_dim * 2, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->R, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        MPI_Gather(local_mtx->G + mtx->width * kernel_dim * 2, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->G, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        MPI_Gather(local_mtx->B + mtx->width * kernel_dim * 2, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->B, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        MPI_Gather(local_mtx->Gy + mtx->width * kernel_dim * 2, mtx->width * mtx->height / np, MPI_UINT8_T, output_mtx->Gy, mtx->width * mtx->height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    if (rank == 0) {
        printf("Finished in %3.7fs\n", end-start);
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