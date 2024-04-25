/*************************************************************************
 *
 *                     int.h
 *
 *     Assignment: arith
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *     Interface of int.h.
 *
 *************************************************************************/
#include "pnm.h"
#include <stdbool.h>

Pnm_ppm int_parent(Pnm_ppm my_ppm, bool compress);

/* Compress */
Pnm_ppm trim_ppm(Pnm_ppm my_ppm, A2Methods_T methods);
void trim_height(int col, int row, A2Methods_UArray2 array, void *elem, 
                 void *cl);
void trim_width(int col, int row, A2Methods_UArray2 array, void *elem,
                void *cl);
Pnm_ppm to_comp_video(Pnm_ppm my_ppm, A2Methods_T methods);
void apply_comp_vid(int col, int row, A2Methods_UArray2 array, void *elem, 
                    void *cl);

/* Decompress */
Pnm_ppm to_rgb(Pnm_ppm my_ppm, A2Methods_T methods);
void apply_to_rgb(int col, int row, A2Methods_UArray2 array, void *elem, 
                  void *cl);
float rgb_help(float num, float denom);
#undef A2