#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "image.h"

int main(int argc, char* argv[]) {
    Image img, img_RGB, img_Gray;
    Matrix *mtx;
	int scaled;
    
    printf("opening image\n");
    //load image from file
    if (Image_load(&img, "cube.jpg") != 0){
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


	/*******************sample code for brightening image*************************/
	for (int i = 0; i < mtx->height; i++) {
    	for (int j = 0; j < mtx->width; j++) {
			
			scaled = ceil(*(mtx->R + i*mtx->width + j)*1.2);
      		*(mtx->R + i*mtx->width + j) = (uint8_t)(scaled>255)?255:scaled;
			scaled = ceil(*(mtx->G + i*mtx->width + j)*1.2);
      		*(mtx->G + i*mtx->width + j) = (uint8_t)(scaled>255)?255:scaled;
			scaled = ceil(*(mtx->B + i*mtx->width + j)*1.2);
      		*(mtx->B + i*mtx->width + j) = (uint8_t)(scaled>255)?255:scaled;
			scaled = ceil(*(mtx->Gy + i*mtx->width + j)*1.2);
      		*(mtx->Gy + i*mtx->width + j) = (uint8_t)(scaled>255)?255:scaled;
    	}
  	}
	/******************************************************************************/



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


    // Save images
    Image_save(&img_Gray, "cube_gray.png");
    Image_save(&img_RGB, "cube_RGB.png");
 

    // Release memory
    Image_free(&img_Gray);
    Image_free(&img_RGB);
 
    Matrix_free(mtx);

    return 0;
}
