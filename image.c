#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

/* load image from file 
 *	Inputs:	pointer to Image structure (output)
 *			filename of image
 * 	Output: 0 - success, -1 - error
*/
int Image_load(Image *img, const char *fname) {
    if((img->data = stbi_load(fname, &img->width, &img->height, &img->channels, 0)) != NULL) {
        img->size = img->width * img->height * img->channels;
        img->allocation_ = ALLOCATED;
		printf("channels=%d\n", img->channels);
        return 0;
    }else{
        return -1;    
    }
}
/*  transfer data from Image structure to Matrix structure 
 *	Inputs:	pointer to Image structure
 *			pointer to Matrix structure (output)
 * 	Output: 0 - success, -1 - error
*/
int Image_to_Matrix(const Image *img, Matrix *mtx) {
	int i=0;

    if (!(img->allocation_ != NO_ALLOCATION && img->channels >= 3)){
        printf("The input image must have at least 3 channels->");
        return -1;
    }

    if(img->data != NULL) {
        mtx->width = img->width;
        mtx->height = img->height;
		mtx->channels = img->channels;
    }
    
    printf("height=%d, width=%d\n", mtx->height, mtx->width); 
    mtx->R = malloc(mtx->width * mtx->height);
	mtx->G = malloc(mtx->width * mtx->height);
	mtx->B = malloc(mtx->width * mtx->height);
	mtx->A = malloc(mtx->width * mtx->height);
	mtx->Gy = malloc(mtx->width * mtx->height);

    printf("malloc ok\n");
    if(mtx->R == NULL || mtx->G == NULL || mtx->B == NULL || mtx->A == NULL || mtx->Gy == NULL) {
        mtx->allocation_ = NO_ALLOCATION;
        printf("Error in creating matrix\n");
        return -1;
    }

    mtx->allocation_ = ALLOCATED;
    
    for(uint8_t *p = img->data; p != img->data + img->size; p += img->channels) {	
        *(mtx->R+i) = (uint8_t) *p;  					//R
        *(mtx->G+i) = (uint8_t) *(p+1);  				//B
        *(mtx->B+i) = (uint8_t) *(p+2);  				//G
		*(mtx->Gy+i) = (uint8_t) ((*p + *(p + 1) + *(p + 2))/3);	//Gray
		if(img->channels == 4) *(mtx->A+i) = (uint8_t) *(p+3);  				//A - transparency
		i++;
    }
	printf("i=%d\n", i);

    return 0;

}

/*  transfer data from Matrix to RGB Image structure 
 *	Inputs:	pointer to Matrix structure
 *			pointer to Image structure (output)
 * 	Output: 0 - success, -1 - error
*/
int Matrix_to_RGB_Image(Matrix *mtx, Image *img){
	int i=0;

    size_t size = mtx->width * mtx->height * mtx->channels;
    img->data = malloc(size);
    if(img->data == NULL) {
        printf("Error in creating RGB image\n");
        img->allocation_ = NO_ALLOCATION;
        return -1;
    }

    if(img->data != NULL) {
        img->width = mtx->width;
        img->height = mtx->height;
        img->size = size;
        img->channels = mtx->channels;
        img->allocation_ = ALLOCATED;
    }

    for(uint8_t *p = img->data; p != img->data + img->size; p += img->channels) {
        *p = (uint8_t) *(mtx->R+i);  					//R
        *(p+1) = (uint8_t) *(mtx->G+i);  				//G
        *(p+2) = (uint8_t) *(mtx->B+i);  				//B
		if (img->channels == 4) *(p+3) = (uint8_t) *(mtx->A+i);  				//A - transparency
		i++;
    }

    return 0;

}

/*  transfer data from Matrix to grayscale Image structure 
 *	Inputs:	pointer to Matrix structure
 *			pointer to Image structure (output)
 * 	Output: 0 - success, -1 - error
*/
int Matrix_to_Gray_Image(Matrix *mtx, Image *img) {
	int i=0;

    size_t size = mtx->width * mtx->height * mtx->channels;
    img->data = malloc(size);
    if(img->data == NULL) {
        printf("Error in creating gray image\n");
        img->allocation_ = NO_ALLOCATION;
        return -1;
    }

    if(img->data != NULL) {
        img->width = mtx->width;
        img->height = mtx->height;
        img->size = size;
		img->channels = mtx->channels;
        img->allocation_ = ALLOCATED;
    }
    
    for(uint8_t *p = img->data; p != img->data + img->size; p += img->channels) {
        *p = *(p+1) = *(p+2) = (uint8_t) *(mtx->Gy+i);  	//gray
		if (img->channels==4) *(p+3) = (uint8_t) *(mtx->A+i);
		i++;
    }
    return 0;

}

/* save image to file 
 *	Inputs:	pointer to Image structure
 *			filename of image
 * 	Output: 0 - success, -1 - error
*/
void Image_save(const Image *img, const char *fname) { 
    stbi_write_png(fname, img->width, img->height, img->channels, img->data, img->width * img->channels);
}

/* release memory allocated for image
 *	Inputs:	pointer to Image structure 
 * 	
*/
void Image_free(Image *img) {
    if(img->allocation_ != NO_ALLOCATION && img->data != NULL) {
        stbi_image_free(img->data);
        img->data = NULL;
        img->width = 0;
        img->height = 0;
        img->size = 0;
        img->allocation_ = NO_ALLOCATION;
    }

}

/* release memory allocated for matrix
 *	Inputs:	pointer to Matrix structure 
 * 	
*/
void Matrix_free(Matrix *matrix) {
    if(matrix->allocation_ != NO_ALLOCATION && (matrix->R != NULL || matrix->G != NULL || matrix->B != NULL || matrix->Gy != NULL)) {
        free(matrix->R);
        free(matrix->G);
        free(matrix->B);
        free(matrix->A);
        free(matrix->Gy);
        matrix->R = NULL;
		matrix->G = NULL;		
		matrix->B = NULL;
		matrix->Gy = NULL;
		matrix->A = NULL;
        matrix->width = 0;
        matrix->height = 0;
		matrix->channels = 0;
        matrix->allocation_ = NO_ALLOCATION;
    }

}

