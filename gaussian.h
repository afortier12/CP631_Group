#pragma once
#include "image.h"

/*  Generate Gaussian kernel values and store them in the kernel array 
 *	Inputs:	pointer to kernel array
 *			kernel dimension
 *          sigma value for Gaussian function
*/
void Get_Gaussian_Kernel(float* kernel, int dim, double sigma);

/*  Apply Gaussian Blur Filter by updating image matrix using the Gaussian Kernel 
 *	Inputs:	Gaussian kernel
 *          kernel dimension
 *			image matrix
*/
void Apply_Gaussian_Blur_Filter(float* kernel, int dim, Matrix *mtx);