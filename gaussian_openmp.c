/* 
 * CP631 - Group Project
 * Adam Fortier  
 * Ashwini Choudhari
 * Ning Ma
 * 
 * gaussian_openmp.h
 * 	implementation of gaussian blur functions
 *  using OpenMP
*/

#include <stdio.h>
#include <math.h>
#include "gaussian.h"
#include "omp.h"

/* get gaussian kernel
 *	Inputs:	pointer to kernel
 *			dimension of kernel
 *			sigma value for kernel
 * 
*/
void Get_Gaussian_Kernel(float* kernel, int dim, double sigma) {
	double value = 0.0;
	int i, j = 0;
    int idx;

    #pragma omp parallel for collapse(2) private(i,j, value, idx) schedule (dynamic) 
	for (i = -dim; i <= dim; i++) {
		for (j = -dim; j <= dim; j++) {
			value = (exp(-((i*i) + (j*j)) / (2 * (sigma*sigma)))) / (2 * M_PI*sigma*sigma);
            idx = (i+dim)*(dim*2 + 1) + (j+dim);
            
			kernel[idx] = value ;
		}
	}

	printf("Kernel generated successfully\n");
}

/* apply guassian blur filter
 *	Inputs:	pointer to kernel
 *			dimension of kernel
 *			matrix with image data (output)
 * 
*/
void Apply_Gaussian_Blur_Filter(float* kernel, int dim, Matrix *mtx) {
    int i, j, ii, jj = 0;
    int kernel_width = (dim*2) +1;
    int RGBGY[4] = { 0 };

	for (i = 0; i < mtx->height; i++) {
        //launch parallel threads to calculate RGB and Gray pixel values
        #pragma omp parallel for private(j, ii, jj) schedule(dynamic) reduction(+:RGBGY[:4])
    	for (j = 0; j < mtx->width; j++) {            
            float new_RGBGY[4] = { 0.0f };
            float weights = 0.0f;

	        for (ii = -dim; ii < dim; ii++) {
		        for (jj = -dim; jj < dim; jj++) {
                    int k_x, k_y, m_x, m_y = 0;
                    
                    //calculate kernel coordinates
                    k_y = ((i+ii) >= 0) ? (((i+ii) <= mtx->height) ? (ii+dim): dim*2): 0;
                    k_x = ((j+jj) >= 0) ? (((j+jj) <= mtx->width) ? (jj+dim): dim*2):0; 
                    //calculate matrix coordinates
                    m_y = ((i+ii) >= 0) ? (((i+ii) <= mtx->height) ? i+ii: mtx->height-1): 0;
                    m_x = ((j+jj) >= 0) ? (((j+jj) <= mtx->width) ? j+jj: mtx->width-1):0;   
       
                    //convolution
				    new_RGBGY[0] += kernel[k_y*kernel_width + k_x] * (*(mtx->R + m_y*mtx->width+m_x));
	    		    new_RGBGY[1] += kernel[k_y*kernel_width + k_x] * (*(mtx->G + m_y*mtx->width+m_x));
				    new_RGBGY[2] += kernel[k_y*kernel_width + k_x] * (*(mtx->B + m_y*mtx->width+m_x));
	    		    new_RGBGY[3] += kernel[k_y*kernel_width + k_x] * (*(mtx->Gy + m_y*mtx->width+m_x));
                    weights += kernel[k_y*kernel_width + k_x];
		        }
	        }
            		
    	    //set values for pixels at (i,j)
    	    RGBGY[0] = (uint8_t) new_RGBGY[0]/weights;
    	    RGBGY[1] = (uint8_t) new_RGBGY[1]/weights;
    	    RGBGY[2] = (uint8_t) new_RGBGY[2]/weights;
    	    RGBGY[3] = (uint8_t) new_RGBGY[3]/weights;

            //copy values to output matrix
            mtx->R[i*mtx->width+j] = RGBGY[0];
            mtx->G[i*mtx->width+j] = RGBGY[1];
    	    mtx->B[i*mtx->width+j] = RGBGY[2];
    	    mtx->Gy[i*mtx->width+j] = RGBGY[3];
	    }
    }
	
}   
