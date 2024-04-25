/*************************************************************************
 *
 *                     ppmdiff.c
 *
 *     Assignment: arith
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *     Implementation of ppmdiff.c, which quantizes the difference between
 *     two files (one must be the compressed and decompressed version of the
 *     other).
 *
 *************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "pnm.h"
#include <a2methods.h>
#include <math.h>
#include "a2blocked.h"
#include "uarray2b.h"

/* Function Declarations */
void ppmdiff(FILE *file1, FILE *file2);


int main(int argc, char *argv[]) 
{
        FILE *file1, *file2;
        (void) argc;
        int count = 0;

        if (argc != 3) {
                fprintf(stderr, "Must provide 2 arguments\n");
                exit(EXIT_FAILURE);
        } else {
                for (int i = 1; i < argc; i++) {
                        if (strcmp(argv[i], "-") == 0) {
                                count++;
                                if (count > 1) {
                                        fprintf(stderr, "both files are -\n");
                                        exit(EXIT_FAILURE);
                                }
                                if (i == 1) {
                                        file1 = stdin;
                                } else {
                                        file2 = stdin;
                                }
                        } else {
                                if (i == 1) {
                                        file1 = fopen(argv[i], "r");
                                } else {
                                        file2 = fopen(argv[i], "r");
                                }     
                        }
                }
                assert(file1 != NULL);
                assert(file2 != NULL); 

                ppmdiff(file1, file2);  
                if (file1 == stdin) {
                        fclose(file2);
                } else if (file2 == stdin) {
                        fclose(file1);
                } else {
                        fclose(file1);
                        fclose(file2);
                }
        }
        return 0;
}


void ppmdiff(FILE *file1, FILE *file2)
{
        A2Methods_T methods = uarray2_methods_blocked;
        Pnm_ppm ppm_1 = Pnm_ppmread(file1, methods);
        Pnm_ppm ppm_2 = Pnm_ppmread(file2, methods);
        A2Methods_UArray2 pixels1 = ppm_1->pixels;
        A2Methods_UArray2 pixels2 = ppm_2->pixels;
        double finalSum = 0;
        if ((ppm_1->height - ppm_2->height) > 1 || 
            (ppm_1->width - ppm_2->width) > 1) { 
                fprintf(stderr, "Height and width differ\n");
                printf("1.0\n");
                exit(EXIT_FAILURE);
        } else {
                int height, width;
                if (ppm_1->height < ppm_2->height) {
                        height = ppm_1->height;
                } else {
                        height = ppm_2->height;
                }

                if (ppm_1->width < ppm_2->width) {
                        width = ppm_1->width;
                } else {
                        width = ppm_2->width;
                }
                float d1 = ppm_1->denominator;
                float d2 = ppm_2->denominator;
                float red1, red2, green1, green2, blue1, blue2;
                float sumRed, sumGre, sumBlue;
                
                for (int i = 0; i < width; i++) {
                        for (int j = 0; j < height; j++) {
                                struct Pnm_rgb *rgb1 = methods->at(pixels1,
                                                                   i, j);
                                struct Pnm_rgb *rgb2 = methods->at(pixels2, 
                                                                   i, j);
                                red1 = (float)rgb1->red / (float)d1;
                                red2 = (float)rgb2->red / (float)d2;
                                green1 = (float)rgb1->green / (float)d1;
                                green2 = (float)rgb2->green / (float)d2;
                                blue1 = (float)rgb1->blue / (float)d1;
                                blue2 = (float)rgb2->blue / (float)d2;
                                
                                sumRed = (red1 - red2) * (red1 - red2);
                                sumGre = (green1 - green2) * (green1 - green2);
                                sumBlue = (blue1 - blue2) * (blue1 - blue2);

                                finalSum += (sumRed + sumGre + sumBlue) / 
                                            (3 * width * height);
                        }
                }
                finalSum = sqrt(finalSum);
                
        }

        printf("E: %.4f\n", finalSum);
        Pnm_ppmfree(&ppm_1);
        Pnm_ppmfree(&ppm_2);
}
