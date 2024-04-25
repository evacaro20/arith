/*************************************************************************
 *
 *                     float.h
 *
 *     Assignment: arith
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *     Interface of float.c.
 *
 *************************************************************************/

#include "pnm.h"
#include <stdbool.h>


Pnm_ppm float_parent(Pnm_ppm my_ppm, bool compress);

/* Compression */
A2Methods_UArray2 DCT(Pnm_ppm my_ppm, A2Methods_T methods, int width, 
                      int height);

/* Decompression */
A2Methods_UArray2 inverse_DCT(Pnm_ppm my_ppm, A2Methods_T methods, int width, 
                              int height);

/* Helper Functions */
void apply_scale(int col, int row, A2Methods_UArray2 array, void *elem, 
                 void *cl);
Pnm_ppm change_scale(Pnm_ppm my_ppm, A2Methods_T methods);
signed scale_helper(float num);