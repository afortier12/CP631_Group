#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "omp.h"
#include "image.h"
#include "gaussian.h"

int main(int argc, char* argv[]) {
    Image img, img_RGB, img_Gray;
    Matrix *mtx, *local_mtx, *adjacent_mtx;
    int offset;
    int rank, np, i, j;
    int kernel_dim = 10;
    double kernel_sigma = 5;
    size_t kernel_size;
    float *kernel;
    int width = 0, height = 0, channels = 0;
	double start, finish;

    mtx = malloc(sizeof(Matrix));
    Matrix_init(mtx);
    local_mtx = malloc(sizeof(Matrix));
    Matrix_init(local_mtx);

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    /* get matrix dimension */
    if(argc == 4){
        printf("arguments supplied are: filename = %s kernel dimension = %s, kernel sigma = %s\n", argv[1], argv[2], argv[3]);
        kernel_dim = atoi(argv[2]);
        kernel_sigma = atof(argv[3]);
        if (kernel_dim == 0){
        	printf("Please enter a valid integer for kernel dimension between 5 and 100 as an argument\n"); 
            Matrix_free(mtx);
    	    Matrix_free(local_mtx);	            
            MPI_Abort(MPI_COMM_WORLD, 0);            
            return -1;
        } else if( ((int)kernel_sigma) > (kernel_dim/2) || kernel_sigma < 3.0){
            printf("Please enter a double for kernel sigma that is between 3 and %d (half of kernel dimension)\n", kernel_dim/2);      
            return -1;
        }
    } else {
        printf("Please provide filename , kernel dimension and kernel sigma\nUsage: %s filename dimension sigma ,\n \twhere dimension and sigma are integeres\n", argv[0]);
        Matrix_free(mtx);
	    Matrix_free(local_mtx);	
        MPI_Abort(MPI_COMM_WORLD, 0);
        return -1;
    }

    if (rank == 0) {
        printf("opening image\n");
        //load image from file
        if (Image_load(&img, argv[1]) != 0){
            printf("Error in loading the image\n");       
            return -1;
        }

        printf("image to matrix\n");
        // Convert the image to Matrix
        if (Image_to_Matrix(&img, mtx) != 0){
            printf("Error creating Matrix\n");       
            return -1;
        }

        
 /*       printf("\nsent\n");
        for (i=mtx->height/2; i<mtx->height; i++){
    	    for (j=0; j<mtx->width; j++){
                printf("%d ", mtx->A[i*mtx->width + j]);
    	    }
            printf("\n");
        }*/
       

	    //original image no longer needed
        Image_free(&img);

        height = mtx->height;    
        width = mtx->width;
        channels = mtx->channels;
    }

    //determine execution time
    finish = MPI_Wtime();

    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);  
    //printf("Matrix width in Process %d: %d\n", rank, width);
 
    local_mtx->width = width;
    local_mtx->height = (height/np) + kernel_dim*2;
    local_mtx->R = malloc(local_mtx->width * local_mtx->height * sizeof(MPI_UINT8_T));
	local_mtx->G = malloc(local_mtx->width * local_mtx->height * sizeof(MPI_UINT8_T));
	local_mtx->B = malloc(local_mtx->width * local_mtx->height * sizeof(MPI_UINT8_T));
	local_mtx->Gy = malloc(local_mtx->width * local_mtx->height * sizeof(MPI_UINT8_T));
    //printf("Local Matrix Size in Process %d: %d * %d\n", rank, local_mtx->width, local_mtx->height);


    for (i=0; i<kernel_dim; i++)
	    for (j=0; j<local_mtx->width; j++){
		    local_mtx->R[i*local_mtx->width+j]=0;
		    local_mtx->G[i*local_mtx->width+j]=0;
		    local_mtx->B[i*local_mtx->width+j]=0;
		    local_mtx->Gy[i*local_mtx->width+j]=0;
	    }

    for (i=(local_mtx->height-kernel_dim); i<local_mtx->height; i++)
	    for (j=0; j<local_mtx->width; j++){
		    local_mtx->R[i*local_mtx->width+j]=0;
		    local_mtx->G[i*local_mtx->width+j]=0;
		    local_mtx->B[i*local_mtx->width+j]=0;
		    local_mtx->Gy[i*local_mtx->width+j]=0;
	    } 
    
    offset = kernel_dim*width;
    MPI_Scatter(mtx->R, width * height/np, MPI_BYTE, &(local_mtx->R[offset]), width * height/np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Scatter(mtx->G, width * height/np, MPI_UINT8_T, &(local_mtx->G[offset]), width *height/np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Scatter(mtx->B, width * height/np, MPI_UINT8_T, &(local_mtx->B[offset]), width * height/np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Scatter(mtx->Gy, width * height/np, MPI_UINT8_T, &(local_mtx->Gy[offset]), width * height/np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    //printf("Scatter complete, offset = %d\n", offset);
	
	/*create an adjacent array to hold last kernel_dim rows of local_mtx*/
    adjacent_mtx = malloc(sizeof(Matrix));
    Matrix_init(adjacent_mtx);
    adjacent_mtx->width = width;
    adjacent_mtx->height = kernel_dim;
    adjacent_mtx->R = malloc(adjacent_mtx->width * adjacent_mtx->height * sizeof(uint8_t));
    adjacent_mtx->G = malloc(adjacent_mtx->width * adjacent_mtx->height * sizeof(uint8_t));
    adjacent_mtx->B = malloc(adjacent_mtx->width * adjacent_mtx->height * sizeof(uint8_t));
    adjacent_mtx->Gy = malloc(adjacent_mtx->width * adjacent_mtx->height * sizeof(uint8_t));
    //printf("Adjacent Matrix in Process %d: %d * %d\n", rank, adjacent_mtx->width, adjacent_mtx->height);

	/*copy the last kernel_dim rows from local_mtx to adjacent mtx*/  
	for (i=0; i<kernel_dim; i++)
		for (j=0; j<local_mtx->width; j++){
			adjacent_mtx->R[i*local_mtx->width+j] = local_mtx->R[(local_mtx->height-2*kernel_dim+i)*local_mtx->width+j];
			adjacent_mtx->G[i*local_mtx->width+j] = local_mtx->G[(local_mtx->height-2*kernel_dim+i)*local_mtx->width+j];
			adjacent_mtx->B[i*local_mtx->width+j] = local_mtx->B[(local_mtx->height-2*kernel_dim+i)*local_mtx->width+j];
			adjacent_mtx->Gy[i*local_mtx->width+j] = local_mtx->Gy[(local_mtx->height-2*kernel_dim+i)*local_mtx->width+j];
		}	
    //printf("Process %d copied last rows\n", rank);
		
	/*send  adjacent matrix to next process*/
	if(rank < (np-1)){
		MPI_Send(adjacent_mtx->R, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 0, MPI_COMM_WORLD);
		MPI_Send(adjacent_mtx->G, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 1, MPI_COMM_WORLD);
		MPI_Send(adjacent_mtx->B, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 2, MPI_COMM_WORLD);
		MPI_Send(adjacent_mtx->Gy, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 3, MPI_COMM_WORLD);
        //printf("Process %d, sent#1 adjacent matrix\n", rank);
	}
	if (rank > 0){
		MPI_Recv(local_mtx->R, adjacent_mtx->width*adjacent_mtx->height, MPI_INT8_T, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(local_mtx->G, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(local_mtx->B, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank-1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(local_mtx->Gy, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank-1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("Process %d, received#1 adjacent matrix\n", rank);
	}
	
	/*copy the first kernel_dim rows from local_mtx to adjacent mtx*/ 
    for (i=0; i<kernel_dim; i++){
	    for (j=0; j<local_mtx->width; j++){
		    adjacent_mtx->R[i*adjacent_mtx->width+j] = local_mtx->R[(i+kernel_dim)*adjacent_mtx->width+j];
		    adjacent_mtx->G[i*adjacent_mtx->width+j] = local_mtx->G[(i+kernel_dim)*adjacent_mtx->width+j];
		    adjacent_mtx->B[i*adjacent_mtx->width+j] = local_mtx->B[(i+kernel_dim)*adjacent_mtx->width+j];
		    adjacent_mtx->Gy[i*adjacent_mtx->width+j] = local_mtx->Gy[(i+kernel_dim)*adjacent_mtx->width+j];
            //printf("%d ", adjacent_mtx->R[i*adjacent_mtx->width+j]);
	    }
        //printf("\nsent\n");
    }
		
	/*send  adjacent matrix to previous process*/
	if(rank > 0){
		MPI_Send(adjacent_mtx->R, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank-1, 4, MPI_COMM_WORLD);
		MPI_Send(adjacent_mtx->G, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank-1, 5, MPI_COMM_WORLD);
		MPI_Send(adjacent_mtx->B, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank-1, 6, MPI_COMM_WORLD);
		MPI_Send(adjacent_mtx->Gy, adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank-1, 7, MPI_COMM_WORLD);
        //printf("Process %d, sent#2 adjacent matrix\n", rank);
	}
	/*receive adjacent array from next process*/ 
	if (rank < (np-1)){
        offset = (local_mtx->height - kernel_dim)*local_mtx->width;
		MPI_Recv(&local_mtx->R[offset], adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&local_mtx->G[offset], adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&local_mtx->B[offset], adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&local_mtx->Gy[offset], adjacent_mtx->width*adjacent_mtx->height, MPI_UINT8_T, rank+1, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("Process %d, received#2 adjacent matrix, offset = %d\n", rank, offset);
    }
	
	//record starting time for measurement of execution time        
    start = MPI_Wtime(); 
	
	kernel_size = (kernel_dim*2+1)*(kernel_dim*2+1);
    kernel = (float *) malloc(kernel_size*sizeof(float));

    if (rank == 0)
        Get_Gaussian_Kernel(kernel, kernel_dim, kernel_sigma);

    MPI_Bcast(kernel, kernel_size, MPI_FLOAT, 0, MPI_COMM_WORLD); 

	Apply_Gaussian_Blur_Filter(kernel, kernel_dim, local_mtx);

    free(kernel);
    
    offset = (kernel_dim)*(local_mtx->width);
    MPI_Gather(&(local_mtx->R[offset]), width * height / np, MPI_UINT8_T, mtx->R, width * height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gather(&(local_mtx->G[offset]), width * height / np, MPI_UINT8_T, mtx->G, width * height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gather(&(local_mtx->B[offset]), width * height / np, MPI_UINT8_T, mtx->B, width * height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gather(&(local_mtx->Gy[offset]), width * height / np, MPI_UINT8_T, mtx->Gy, width * height / np, MPI_UINT8_T, 0, MPI_COMM_WORLD);

    //determine execution time
    finish = MPI_Wtime();

    if (rank == 0) {
        printf("finished in %3.7fs\n", finish-start);

        printf("matrix to RGB image\n");
        //Convert RGB matrices to image
        if (Matrix_to_RGB_Image(mtx, &img_RGB) != 0){
            printf("Error creating RGB image from matrix\n");       
            return -1;
        }
 
        printf("matrix to grayscale image\n");
        // Convert Gray matrix to image
        if (Matrix_to_Gray_Image(mtx, &img_Gray) != 0){
            printf("Error creating grayscale image from matrix\n");       
            return -1;
        }

        // Save images
        Image_save(&img_Gray, "cube_gray.png");
        Image_save(&img_RGB, "cube_RGB.png");
        Image_free(&img_Gray);
        Image_free(&img_RGB);
		
    }

    Matrix_free(mtx);
    Matrix_free(local_mtx);
    Matrix_free(adjacent_mtx);
	
	
    MPI_Finalize();

    return 0;
}
