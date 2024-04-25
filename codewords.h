/*************************************************************************
 *
 *                     codewords.h
 *
 *     Assignment: arith
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *     Interface of codewords.h. 
 *
 *************************************************************************/
#include <stdbool.h>
#include "pnm.h"


Pnm_ppm codewords_parent(Pnm_ppm my_ppm, bool compress, FILE *input);

/* Compress */
A2Methods_UArray2 pack(A2Methods_UArray2 array, A2Methods_T methods);
void apply_pack(int col, int row, A2Methods_UArray2 array, void *elem, 
                void *cl);
void apply_print(int col, int row, A2Methods_UArray2 array, void *elem, 
                 void *cl);

/* Decompress */
void code_apply(int col, int row, A2Methods_UArray2 array, void *elem, 
                void *cl);
A2Methods_UArray2 unpack(A2Methods_UArray2 array, A2Methods_T methods, 
                         int width, int height);
void apply_unpack(int col, int row, A2Methods_UArray2 array, void *elem, 
                  void *cl);