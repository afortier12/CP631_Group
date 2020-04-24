#pragma once 
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

enum memory_check{
    NO_ALLOCATION, ALLOCATED
};

/*structure to hold image information 
 * returned from call to stbi_load
 * data holds pixel information 
*/ 
typedef struct {
    int width;
    int height;						
    int channels;					//number of channels read
    size_t size;					//width*height*channels
    uint8_t *data;					//RGBA data (1 byte each)
    enum memory_check allocation_;
} Image;

/*structure to hold matrix information 
 * returned from call to stbi_load
 * data holds pixel information 
*/ 
typedef struct {
    int width;
    int height;
	int channels;					//number of channels from original image
    uint8_t *R;						//array of Red values
    uint8_t *G;						//array of Green values
    uint8_t *B;						//array of Blue values
	uint8_t *A;						//array of Alpha (transparency level) values
    uint8_t *Gy;					//array of Gray values
    enum memory_check allocation_;	//flag if memory was allocated
} Matrix;

/* load image from file 
 *	Inputs:	pointer to Image structure (output)
 *			filename of image
 * 	Output: 0 - success, -1 - error
*/
int Image_load(Image *img, const char *fname);

/*  transfer data from Image structure to Matrix structure 
 *	Inputs:	pointer to Image structure
 *			pointer to Matrix structure (output)
 * 	Output: 0 - success, -1 - error
*/
int Image_to_Matrix(const Image *img, Matrix *mtx);

/*  transfer data from Matrix to RGB Image structure 
 *	Inputs:	pointer to Matrix structure
 *			pointer to Image structure (output)
 * 	Output: 0 - success, -1 - error
*/
int Matrix_to_RGB_Image(Matrix *mtx, Image *img);

/*  transfer data from Matrix to grayscale Image structure 
 *	Inputs:	pointer to Matrix structure
 *			pointer to Image structure (output)
 * 	Output: 0 - success, -1 - error
*/
int Matrix_to_Gray_Image(Matrix *mtx, Image *img);

/* save image to file 
 *	Inputs:	pointer to Image structure
 *			filename of image
 * 	Output: 0 - success, -1 - error
*/
void Image_save(const Image *img, const char *fname);

/* release memory allocated for image
 *	Inputs:	pointer to Image structure 
 * 	
*/
void Image_free(Image *img);

/* initialize memory allocated for matrix
 * 	
*/
void Matrix_init(Matrix *matrix);

/* release memory allocated for matrix
 *	Inputs:	pointer to Matrix structure 
 * 	
*/
void Matrix_free(Matrix *mtx);


