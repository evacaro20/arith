/*************************************************************************
 *
 *                     int.c
 *
 *     Assignment: arith
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *     Implementation of int.c, which works primarily on variables that use
 *     ints. Facilitates  conversion between RBGs and component video arrays.
 *
 *************************************************************************/
#include "int.h"
#include "a2methods.h"
#include "uarray2.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "assert.h"
#include "mem.h"

typedef A2Methods_UArray2 A2;

typedef struct array_methods {
        /* the array */
        A2 *array; 
        /* the methods client wants to use */
        A2Methods_T methods;
        /* the dimension, not used all the time */
        int value;
} array_methods;

typedef struct comp_v {
        /* float values */
        float y, pB, pR;
} comp_v;


/********** int_parent **************************************************
 *
 * This function is a parent function for everything in int.c. If we are 
 * compressing, this function will trim the given image (if width/height odd),
 * change the RGB values to comp video values and return the Pnm_ppm. If we
 * are decompressing the function will change the array values from comp 
 * video to RGB values and return the Pnm_ppm.
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      bool compress           if we are compressing or not
 *
 * Return: a Pnm_ppm with the new values in the array
 *
 * Expects: my_ppm is not NULL
 *     
 * Notes:  
 *      
 ***********************************************************************/
Pnm_ppm int_parent(Pnm_ppm my_ppm, bool compress)
{        
        assert(my_ppm != NULL);
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods != NULL);
        if (compress) {
                my_ppm = trim_ppm(my_ppm, methods);
                my_ppm = to_comp_video(my_ppm, methods);
        } else {
                my_ppm = to_rgb(my_ppm, methods);
        }
        
        return my_ppm;
}

/********** trim_ppm ****************************************************
 *
 * This function checks the width and the height of the image and if they are
 * not even, it trims it by one. It creates a new array and calls on a mapping
 * function and places all the values from the old array into the new trimmed
 * array and then returns the Pnm_ppm with the new array.
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      A2Methods_T methods     methods given
 *
 * Return: a Pnm_ppm with trimmed array
 *
 * Expects: my_ppm is not NULL
 *     
 * Notes: if the image needs to be trimmed then we create a new array and free
 *        the old one, compression
 *      
 ***********************************************************************/
Pnm_ppm trim_ppm(Pnm_ppm my_ppm, A2Methods_T methods)
{
        assert(my_ppm->height - 1 != 0 && my_ppm->width - 1 != 0);
        if (my_ppm->height % 2 != 0) {
                my_ppm->height = my_ppm->height - 1;
                A2 array = my_ppm->pixels;
                A2 trimmed_array = methods->new(my_ppm->width, my_ppm->height, 
                                                sizeof(struct Pnm_rgb));
                array_methods cl;
                cl.array = &trimmed_array;
                cl.methods = methods;
                cl.value = my_ppm->height;
                methods->map_default(array, trim_height, (void *) &cl);
                methods->free(&array);
                my_ppm->pixels = trimmed_array;
                
        }
        if (my_ppm->width % 2 != 0) {
                my_ppm->width = my_ppm->width - 1;
                A2 array = my_ppm->pixels;
                A2 trimmed_array = methods->new(my_ppm->width, my_ppm->height,
                                                sizeof(struct Pnm_rgb));
                array_methods cl;
                cl.array = &trimmed_array;
                cl.methods = methods;
                cl.value = my_ppm->width;
                methods->map_default(array, trim_width, (void *) &cl);
                methods->free(&array);
                my_ppm->pixels = trimmed_array;
        }
        return my_ppm;
}

/********** trim_height ****************************************************
 *
 * This function is the apply function for our trimming function. It checks
 * that the current column index is less than the new array's height, then it
 * places the current element into the new array.
 *
 * Parameters:
 *      int col                          column
 *      int row                          row
 *      A2 array                         the array
 *      void *elem                       elem at that position
 *      void *cl                         closure struct
 *
 * Return: void
 *
 * Expects: closure is not NULL, elem is not NULL
 *     
 * Notes: Compression
 *      
 *************************************************************************/

void trim_height(int col, int row, A2 array, void *elem, void *cl)
{
        assert(cl != NULL);
        assert(elem != NULL);
        (void) array;
        array_methods *cl_p = cl;
        A2Methods_T methods = cl_p->methods;
        A2 *trim_array = cl_p->array;
        int height = cl_p->value;
        struct Pnm_rgb *ep = elem;
        if (row < height) {
                *(struct Pnm_rgb *)methods->at(*trim_array, col, row) = *ep;
        }
}

/********** trim_width ****************************************************
 *
 * This function is the apply function for our trimming function. It checks
 * that the current row index is less than the new array's width, then it
 * places the current element into the new array.
 *
 * Parameters:
 *      int col                          column
 *      int row                          row
 *      A2 array                         the array
 *      void *elem                       elem at that position
 *      void *cl                         closure struct
 *
 * Return: void
 *
 * Expects: closure is not NULL, elem is not NULL
 *     
 * Notes: Compression
 *      
 *************************************************************************/
void trim_width(int col, int row, A2 array, void *elem, void *cl)
{
        (void) array;
        array_methods *cl_p = cl;
        A2Methods_T methods = cl_p->methods;
        A2 *trim_array = cl_p->array;
        int width = cl_p->value;
        struct Pnm_rgb *ep = elem;
        if (col < width) {
                *(struct Pnm_rgb *) methods->at(*trim_array, col, row) = *ep;
        }
}

/********** to_comp_video *************************************************
 *
 * This function creates a new array that is type struct comp_v. It then 
 * calls on our mapping function (apply_comp_vid) to place everything in the
 * new array of the different type. It returns Pnm_ppm with the new array.
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      A2Methods_T methods     methods given
 *
 * Return: Pnm_ppm
 *
 * Expects: my_ppm is not NULL, methods is not NULL
 *     
 * Notes: We free our old array here! We ALLOC new array using methods->new
 *        but we do not free it. Compression
 *      
 *************************************************************************/
Pnm_ppm to_comp_video(Pnm_ppm my_ppm, A2Methods_T methods)
{
        assert(my_ppm != NULL);
        assert(methods != NULL);

        A2 new_array = methods->new(my_ppm->width, my_ppm->height, 
                                    sizeof(struct comp_v));
        array_methods a_m;
        a_m.array = new_array;
        a_m.methods = methods;  
        a_m.value = my_ppm->denominator;                         
        void *cl = &a_m;
        methods->map_default(my_ppm->pixels, apply_comp_vid, cl);
        methods->free(&my_ppm->pixels);
        my_ppm->pixels = new_array;
        return my_ppm;
}

/********** apply_comp_vid **************************************************
 *
 * This function is the apply function for when we change the values from 
 * RGB to component video values. After we change to comp vid, we place the 
 * new elem of type comp_v into the new array. 
 *
 * Parameters:
 *      int col                          column
 *      int row                          row
 *      A2 array                         the array
 *      void *elem                       elem at that position
 *      void *cl                         closure struct
 *
 * Return: void
 *
 * Expects: closure is not NULL, elem is not NULL, assert that a is between 
 *          0 and 1.
 *     
 * Notes: We void col, row, and the array, Compression
 *      
 *************************************************************************/
void apply_comp_vid(int col, int row, A2 array, void *elem, void *cl)
{
        (void) col;
        (void) row;
        (void) array;
        assert(cl != NULL);
        assert(elem != NULL);

        array_methods *a_m = cl;
        A2 new_array = a_m->array;
        A2Methods_T methods = a_m->methods;
        int denominator = a_m->value;
        struct Pnm_rgb *rgb = elem;

        /* Get the RGB values as floats */
        float r = (float)rgb->red / denominator;
        float g = (float)rgb->green / denominator;
        float b = (float)rgb->blue / denominator;
        
        /* Find y, pB, pR */
        comp_v comp_vid_elem;
        comp_vid_elem.y = 0.299 * r + 0.587 * g + 0.114 * b;
        assert(comp_vid_elem.y >= 0 && comp_vid_elem.y <= 1);
        comp_vid_elem.pB = -0.168736 * r - 0.33125 * g + 0.5 * b;
        comp_vid_elem.pR = 0.5 * r - 0.418688 * g - 0.081312 * b;

        /* Place in the array */
        *(struct comp_v *)methods->at(new_array, col, row) = comp_vid_elem;
}


/********** to_rgb *******************************************************
 *
 * This function creates a new array that is type struct Pnm_rgb. It then 
 * calls on our mapping function (apply_to_rgb) to place everything in the
 * new array of the different type. It returns Pnm_ppm with the new array.
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      A2Methods_T methods     methods given
 *
 * Return: Pnm_ppm
 *
 * Expects: my_ppm is not NULL, methods is not NULL
 *     
 * Notes: We free our old array here! We ALLOC new array using methods->new
 *        but we do not free it. Decompression
 *      
 *************************************************************************/
Pnm_ppm to_rgb(Pnm_ppm my_ppm, A2Methods_T methods)
{
        assert(my_ppm != NULL);
        assert(methods != NULL);

        A2 new_array = methods->new(my_ppm->width, my_ppm->height, 
                                    sizeof(struct Pnm_rgb));
        array_methods a_m;
        a_m.array = new_array;
        a_m.methods = methods;  
        a_m.value = my_ppm->denominator;                         
        void *cl = &a_m;
        methods->map_default(my_ppm->pixels, apply_to_rgb, cl);
        methods->free(&my_ppm->pixels);
        my_ppm->pixels = new_array;
        return my_ppm;
}

/********** apply_comp_vid ************************************************
 *
 * This function is the apply function for when we change the values from 
 * comp video values to RGB values. After it changes the value, it places it
 * into the new array (given by the closure). 
 *
 * Parameters:
 *      int col                          column
 *      int row                          row
 *      A2 array                         the array
 *      void *elem                       elem at that position
 *      void *cl                         closure struct
 *
 * Return: void
 *
 * Expects: closure is not NULL, elem is not NULL
 *     
 * Notes: We void array, calls on helper function rgb_help to get values. 
 *        Decompression
 *      
 *************************************************************************/
void apply_to_rgb(int col, int row, A2 array, void *elem, void *cl)
{
        (void) array;

        assert(elem != NULL);
        assert(cl != NULL);

        struct comp_v *comp_vid_elem = elem;
        array_methods *a_m = cl;
        A2 new_array = a_m->array;
        A2Methods_T methods = a_m->methods;
        float denom = (float)a_m->value;
        
        float y = comp_vid_elem->y;
        float pB = comp_vid_elem->pB;
        float pR = comp_vid_elem->pR;

        struct Pnm_rgb rgb;

        /* Get rgb values */
        rgb.red = rgb_help(((1.0 * y) + (0.0 * pB) + (1.402 * pR)), denom);
        rgb.green = rgb_help(((1.0 * y) - (0.344136 * pB) - (0.714136 * pR)),
                              denom);
        rgb.blue = rgb_help(((1.0 * y) + (1.772 * pB) + (0.0 * pR)), denom);

        *(struct Pnm_rgb *)methods->at(new_array, col, row) = rgb;
}

/********** rgb_help **************************************************
 *
 * This function is a helper function that checks that the rgb value is not 
 * less than 0 or greater than 1. If it is it changes the value to 0 or 1, 
 * respectively. This is because if we have a value that is slightly negative
 * it will be a very high positive value as a unsigned and skew the image. 
 * This way we assure that will not happen.
 *
 * Parameters:
 *      float num       RBG equation in terms of y, pR, and pB
 *      float denom     denominator
 *
 * Return: float rgb value
 *
 * Expects: number and denom
 *     
 * Notes: We are losing some information here if the num is < 0 or > 1. 
 *        However, this helps the overall image!
 *      
 *************************************************************************/
float rgb_help(float num, float denom)
{
        assert(denom != 0);
        if (num < 0) {
                num = 0;
        } else if (num > 1) {
                num = 1;
        } else {
                num = num;
        }   
        return num * denom; 
}

#undef A2