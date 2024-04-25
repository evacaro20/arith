/*************************************************************************
 *
 *                     compress40.h
 *
 *     Assignment: arith
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *     Interface of compress40, which de/compresses ppms using helper files
 *     int.h, float.h, and codewords.h
 *
 *************************************************************************/

#include <stdio.h>

extern void compress40  (FILE *input);  /* reads PPM, writes compressed image */
extern void decompress40(FILE *input);  /* reads compressed image, writes PPM */