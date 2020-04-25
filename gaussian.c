/* 
 * CP631 - Group Project
 * Adam Fortier  
 * Ashwini Choudhari
 * Ning Ma
 * 
 * gaussian.c
 * 	serial implementation of gaussian blur functions
 *  
*/
#include <stdio.h>
#include <math.h>
#include "gaussian.h"

void Get_Gaussian_Kernel(float* kernel, int dim, double sigma) {
	double value = 0.0;
	int i, j, ii, jj = 0;
    int idx;

	for (i = -dim; i <= dim; i++) {
		ii = i + dim;
		for (j = -dim; j <= dim; j++) {
			jj = j + dim;
			value = (exp(-((i*i) + (j*j)) / (2 * (sigma*sigma)))) / (2 * M_PI*sigma*sigma);
            idx = ii*(dim*2 + 1) + jj;

            //printf("Kernel[%d] = %lf, for x = %d, y = %d \n", idx, value, i, j);
			kernel[idx] = value ;
		}
	}

	printf("Kernel generated successfully\n");
}

int getImgIndex(int width, int height, int x, int y) {
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	return y*width + x;
}

void Apply_Gaussian_Blur_Filter(float* kernel, int dim, Matrix *mtx) {
    float new_R = 0.0f;
    float new_G = 0.0f;
    float new_B = 0.0f;
    float new_Gy = 0.0f;

	int i, j, k, ii, jj = 0;
    int kernel_idx, img_idx;

	for (k = 0; k < (mtx->width*mtx->height); k++) {
        new_R = 0.0f;
        new_G = 0.0f;
        new_B = 0.0f;
        new_Gy = 0.0f;

		// run through the kernel matrix
		for (i = -dim; i < dim; i++) {
			// get real kernel index 
			ii = i + dim;
        	for (j = -dim; j < dim; j++) {
			    jj = j + dim;
                kernel_idx = ii*(dim*2 + 1) + jj;

				// get index image index
				img_idx = getImgIndex(mtx->width, mtx->height, (k % mtx->width) + i, (k / mtx->width) + j);

				// work out new values by multiplying kernel value by pixel value
                //printf("Kernel value: %lf\n", kernel[kernel_idx] * mtx->R[img_idx]);

				new_R += kernel[kernel_idx] * mtx->R[img_idx];
                new_G += kernel[kernel_idx] * mtx->G[img_idx];
                new_B += kernel[kernel_idx] * mtx->B[img_idx];
                new_Gy += kernel[kernel_idx] * mtx->Gy[img_idx];
			}
		}

        //printf("R: %d  New R: %lf;  G: %d  New G: %lf;  B: %d  New B: %lf;  Gy: %d  New Gy: %lf;\n", mtx->R[k], new_R, mtx->G[k], new_G, mtx->B[k], new_B, mtx->Gy[k], new_Gy);

		// set new values to output array
        mtx->R[k] = (uint8_t) new_R;
        mtx->G[k] = (uint8_t) new_G;
        mtx->B[k] = (uint8_t) new_B;
        mtx->Gy[k] = (uint8_t) new_Gy;
	}
}