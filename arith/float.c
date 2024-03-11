/*************************************************************************
 *
 *                     float.c
 *
 *     Assignment: arith
 *     Authors:  Eva Caro and Ava Sim
 *     Date:     3/6/24
 *
 *     Implementation of our float.c. Facilitates conversion between ppm's with
 *     values in the component video format (Y, pB, and pR) and ppm's with 
 *     scaled DCT values (a, b, c, d, avg pB, and avg pR). Significant loss of
 *     information results from the packing of 2-by-2 blocks of pixels into 1.
 *
 *************************************************************************/
#include "float.h"
#include "a2methods.h"
#include "uarray2.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "assert.h"
#include "arith40.h"
#include <math.h>

typedef A2Methods_UArray2 A2;

const float BLK = 4.0;
const float HFBLK = 2.0;
const int SFBCD = 50;
const int SFA = 511;
const float UPBND = .3;
const float LWBND = -.3;

/* structure that holds the component video values in float form */
typedef struct comp_v {
        float y, pB, pR;
} comp_v;

/* structure used to pass second array and methods to apply functions */
typedef struct array_methods {
        /* (usually) new array to write to */
        A2 *array;
        /* methods */
        A2Methods_T methods;
        /* value needed for apply function (like a dimension or denominator) */
        int value;
} array_methods;

/* holds the original DCT values before scaling */
typedef struct dct_values {
        /* unscaled cosine coefficients */
        float a, b, c, d;
        /* quantized pB and pR values */
        unsigned pB, pR;
} dct_values;

typedef struct scaled_dct {
        /* scaled a and quantized pB and pR values */
        unsigned a, pB, pR;
        /* scaled cosine coefficients */
        signed b, c, d;
} scaled_dct;

/********** float_parent **************************************************
 *
 * This function is a parent function for everything in float.c. If we are 
 * compressing, this function will convert the ppm's comp video values into 
 * discrete cosine transform values (which reduces the amount of pixels by a 
 * factor of 4). It quantizes pB and pR, then scales the a, b, c, and d values 
 * onto more precise scales (from 0 to 511 for a and -15 to 15 for b, c, and 
 * d). If we are decompressing the function will undo the aforementioned 
 * scaling, un-quantize pB and pR, and conduct an inverse discrete cosine 
 * transform to return to comp video values. 
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      bool compress           if we are compressing or not
 *
 * Return: a Pnm_ppm with the comp vid values in the array
 *
 * Expects: my_ppm is not NULL
 *     
 * Notes:  
 *      Because pB's and pR's within 2-by-2 blocks of pixels were averaged,
 *      the original pB and pR values cannot be recovered during decompression.
 *      
 ***********************************************************************/
Pnm_ppm float_parent(Pnm_ppm my_ppm, bool compress)
{
        assert(my_ppm != NULL);
        A2Methods_T methods = uarray2_methods_plain;
        if (compress) {
                A2 comp_a = DCT(my_ppm, methods, my_ppm->width,
                                my_ppm->height);
                my_ppm->pixels = comp_a;
                my_ppm->height = my_ppm->height / HFBLK;
                my_ppm->width = my_ppm->width / HFBLK;
                my_ppm = change_scale(my_ppm, methods);
        } else {
                A2 decomp_a = inverse_DCT(my_ppm, methods, my_ppm->width, 
                                          my_ppm->height);
                my_ppm->pixels = decomp_a;
                my_ppm->width = my_ppm->width * HFBLK;
                my_ppm->height = my_ppm->height * HFBLK;
        }
        return my_ppm;
}

/********** DCT **************************************************
 *
 * This function converts an array containing comp video values into one that 
 * is 1/4 the size with DCT values (a, b, c, d, pB, and pR). It quantizes pB 
 * and pR values.
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      A2Methods_T methods     the methods suite for plain uarray2's
 *      int width               the width of the original array
 *      int height              the height of the original array
 *
 * Return: a uarray2 containing the DCT values
 *
 * Expects: my_ppm and methods are not NULL
 *     
 * Notes:  
 *     Because pB's and pR's within 2-by-2 blocks of pixels were averaged,
 *     the original pB and pR values cannot be recovered during decompression.
 *      
 ***********************************************************************/
A2 DCT(Pnm_ppm my_ppm, A2Methods_T methods, int width, int height)
{
        assert(my_ppm != NULL);
        assert(methods != NULL);
        A2 array = my_ppm->pixels;
        A2 *new_array = methods->new(width / HFBLK, height / HFBLK, 
                                     sizeof(struct dct_values));
        float avg_pB, avg_pR;
        for (int j = 0; j < height; j += HFBLK) {
                for (int i = 0; i < width; i += HFBLK) {
                        comp_v *e1, *e2, *e3, *e4;
                        /* getting 2-by-2 block of pixels */
                        e1 = methods->at(array, i, j);
                        e2 = methods->at(array, i + 1, j);
                        e3 = methods->at(array, i, j + 1);
                        e4 = methods->at(array, i + 1, j + 1);
                                   
                        dct_values el;
                        el.a = (float)((e4->y + e3->y + e2->y + e1->y) / BLK);
                        el.b = (float)((e4->y + e3->y - e2->y - e1->y) / BLK);
                        el.c = (float)((e4->y - e3->y + e2->y - e1->y) / BLK);
                        el.d = (float)((e4->y - e3->y - e2->y + e1->y) / BLK);
                        avg_pB = (float)((e1->pB + e2->pB + e3->pB + e4->pB) 
                                          / BLK);
                        avg_pR = (float)((e1->pR + e2->pR + e3->pR + e4->pR) 
                                          / BLK);
                        el.pB = Arith40_index_of_chroma(avg_pB);
                        el.pR = Arith40_index_of_chroma(avg_pR);
                        
                        *(dct_values *)methods->at(new_array, (i / 2), 
                                                         (j / 2)) = el;
                }
        }
        methods->free(&array);
        return new_array;
}

/********** change_scale **************************************************
 *
 * This function scales the DCT values in a provided ppm pixels array. a's 
 * go from [0, 1] to [0, 511], while b, c, and d values go from [-0.5, 0.5]
 * to [-15, 15].
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      A2Methods_T methods     the methods suite for plain uarray2's
 *
 * Return: the same ppm
 *
 * Expects: my_ppm and methods are not NULL
 *     
 * Notes:  b, c, and d values between [0.3, 0.5] and [-0.5, -0.3] are all 
 *         turned into 15 and -15, respectively. Also, the original array is 
 *         freed and the "pixels" of the ppm is set to the new array with the 
 *         scaled values. Mapping is done in row-major order by default.
 *      
 ***********************************************************************/
Pnm_ppm change_scale(Pnm_ppm my_ppm, A2Methods_T methods)
{
        assert(my_ppm != NULL);
        assert(methods != NULL);
        A2 array = my_ppm->pixels;
        A2 *new_array = methods->new(my_ppm->width, my_ppm->height, 
                                     sizeof(struct scaled_dct));
        array_methods a_m;
        a_m.array = new_array;
        a_m.methods = methods;
        void *cl = &a_m;
        methods->map_default(array, apply_scale, cl);
        methods->free(&my_ppm->pixels);
        my_ppm->pixels = new_array;
        return my_ppm;

} 

/********** apply_scale **************************************************
 *
 * This is the apply function to change the scale of the DCT values. For every
 * element in the original array, it creates a new element with the scaled DCT
 * values and adds that onto the new array at the same position.
 *
 * Parameters:
 *      int col                 the current element's width-position from left
 *      int row                 the current element's height-position from top
 *      A2 array                the original unscaled array
 *      void *elem              a pointer to the current element to scale
 *      void *cl                a pointer to the array_methods struct that 
 *                              holds the new array to store the scaled 
 *                              elements in and the methods
 * Return: N/A
 *
 * Expects: cl not NULL, new_array passed in through cl is empty and has the
 *          same width and height as the original array
 *     
 * Notes:  calls scale_helper function, a goes from float to unsigned
 *      
 ***********************************************************************/
void apply_scale(int col, int row, A2 array, void *elem, void *cl)
{
        (void) array;
        assert(cl != NULL);
        assert(elem != NULL);

        dct_values *og_elem = elem;
        scaled_dct new_elem;
        array_methods *a_m = cl;
        A2 new_array = a_m->array;
        A2Methods_T methods = a_m->methods;
        new_elem.a = (unsigned)(floorf(og_elem->a * SFA));
        new_elem.b = scale_helper(og_elem->b);
        new_elem.c = scale_helper(og_elem->c);
        new_elem.d = scale_helper(og_elem->d);
        new_elem.pB = og_elem->pB;
        new_elem.pR = og_elem->pR;

        *(scaled_dct *)methods->at(new_array, col, row) = new_elem;
   
}

/********** scale_helper **************************************************
 *
 * This is a helper function that does the scaling for b, c, and d by 
 * multiplying them by a scalefactor (set to 15). 
 *
 * Parameters:
 *      float num               the number to convert
 * 
 * Return: a signed int holding the scaled version of the provided float
 *
 * Expects: N/A
 *     
 * Notes:  Very high and very low values (between -0.3 and -0.5 and 0.3 and 
 *         0.5) are treated as the same value.
 *      
 ***********************************************************************/
signed scale_helper(float num)
{
        if (num >= UPBND) {
                return UPBND * SFBCD;
        } else if (num <= LWBND) {
                return -15;
        } else {
                num *= (float)SFBCD;
                return (signed)num;
        } 
}

/********** inverse_DCT **************************************************
 *
 * This undoes the DCT for a given ppm, converting scaled DCT values into comp
 * video values. This involves unwauntizing pB and pR values.
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      A2Methods_T methods     the methods suite for plain uarray2's
 *      int width               the width of the original array
 *      int height              the height of the original array
 * 
 * Return: an A2 with the new array with comp video elements
 *
 * Expects: my_ppm and methods not NULL
 *     
 * Notes:  Allocates space on the heap for a new array and frees the original
 *         array. We need to free this new array later on.
 *      
 ***********************************************************************/
A2 inverse_DCT(Pnm_ppm my_ppm, A2Methods_T methods, int width, int height)
{
        assert(my_ppm != NULL);
        assert(methods != NULL);
        A2 array = my_ppm->pixels;
        A2 *new_arr = methods->new(width * HFBLK, height * HFBLK, 
                                   sizeof(struct comp_v));
        float y1, y2, y3, y4, a, b, c, d;
        unsigned pB, pR;
        for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                        scaled_dct *og_elem = methods->at(array, i, j);
                        a = (float)((double)og_elem->a / (double)SFA);
                        b = (float)((double)og_elem->b / (double)SFBCD);
                        c = (float)((double)og_elem->c / (double)SFBCD);
                        d = (float)((double)og_elem->d / (double)SFBCD);
                        pB = og_elem->pB;
                        pR = og_elem->pR;  
                        y1 = a - b - c + d;
                        y2 = a - b + c - d;
                        y3 = a + b - c - d;
                        y4 = a + b + c + d;  
                        float new_pB = Arith40_chroma_of_index(pB);
                        float new_pR = Arith40_chroma_of_index(pR);
                        comp_v elem1 = {y1, new_pB, new_pR};
                        comp_v elem2 = {y2, new_pB, new_pR};
                        comp_v elem3 = {y3, new_pB, new_pR};
                        comp_v elem4 = {y4, new_pB, new_pR};
                        *(comp_v *)methods->at(new_arr, i * HFBLK, 
                                                j * HFBLK) = elem1;
                        *(comp_v *)methods->at(new_arr, (i * HFBLK) + 1,
                                                j * HFBLK) = elem2;
                        *(comp_v *)methods->at(new_arr, i * HFBLK, 
                                                (j * HFBLK) + 1) = elem3;
                        *(comp_v *)methods->at(new_arr, (i * HFBLK) + 1, 
                                                (j * HFBLK) + 1) = elem4;
                }
        }
        methods->free(&array);
        return new_arr;
}

#undef A2