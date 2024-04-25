/*************************************************************************
 *
 *                     compress40.c
 *
 *     Assignment: arith
 *     Author:   Eva Caro 
 *     Date:     3/6/24
 *
 *     Handles compressing and decompressing ppm and binary-codeword files.
 *     Results in some loss of information in the process.
 *
 *************************************************************************/

#include "compress40.h"
#include "int.h"
#include "float.h"
#include "codewords.h"
#include "a2methods.h"
#include "uarray2.h"
#include "a2plain.h"
#include <stdbool.h>
#include "assert.h"
#include "arith40.h"
#include "except.h"
#include "mem.h"


const unsigned DENOM = 255;
const int HALF = 2;

typedef A2Methods_UArray2 A2;

/* structure containing scaled DCT values */
struct scaled_dct {
        unsigned a, pB, pR;
        signed b, c, d;
};

/********** compress40 ****************************************************
 *
 * This function handles compression. It calls on functions in external helper 
 * files (int, float, and codewords), which turn a ppm from rgb values to 
 * packed binary codewords, and prints the codewords to standard output.
 *
 * Parameters:
 *      FILE *input             a file pointer to read the ppm from
 *
 * Return: N/A
 *
 * Expects: input is not null and contains information for a valid ppm file
 *     
 * Notes: resulting compressed file is printed to standard output.
 *        Calls on functions in int.h, float.h, and codewords.h, which could 
 *    -   result in exceptions (see function contracts in those three files for
 *        more info).
 *    -   The height and width in the header of the printed information 
 *        reflects that of the original file, not the compressed one (which is
 *        about 1/4 the size).
 *      
 ***********************************************************************/
extern void compress40(FILE *input) 
{
        A2Methods_T methods = uarray2_methods_plain;
        Pnm_ppm my_ppm = Pnm_ppmread(input, methods);

        my_ppm = int_parent(my_ppm, true);
        my_ppm = float_parent(my_ppm, true);
        my_ppm = codewords_parent(my_ppm, true, input);

        Pnm_ppmfree(&my_ppm);

}

/********** decompress40 ****************************************************
 *
 * This function handles decompression. It calls on functions in external 
 * helper files (int, float, and codewords), which read in a file containing 
 * binary codewords and turn it into a ppm with rgb values. Resulting 
 * decompressed file is printed to standard output.
 *
 * Parameters:
 *      FILE *input             a file pointer to read the information from
 *
 * Return: N/A
 *
 * Expects: input is not null and contains information for a valid ppm file
 *     
 * Notes: resulting compressed file is printed to standard output.
 *    -   Calls on functions in int.h, float.h, and codewords.h, which could 
 *        result in exceptions (see function contracts in those three files for
 *        more info).
 *      
 ***********************************************************************/
extern void decompress40(FILE *input) 
{
        A2Methods_T methods = uarray2_methods_plain;
        /* getting header information from input */
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == HALF);
        int c = getc(input);
        assert(c == '\n');

        /* intiialize ppm with empty array filled in decompression functions */
        A2 new_array = methods->new(width / HALF, height / HALF, 
                                    sizeof(uint64_t));

        Pnm_ppm my_ppm = ALLOC(sizeof(struct Pnm_ppm));
        my_ppm->width = width / HALF;
        my_ppm->height = height / HALF;
        my_ppm->denominator = DENOM;
        my_ppm->pixels = new_array;
        my_ppm->methods = methods;

        my_ppm = codewords_parent(my_ppm, false, input);
        my_ppm = float_parent(my_ppm, false);
        my_ppm = int_parent(my_ppm, false);

        Pnm_ppmwrite(stdout, my_ppm);
        Pnm_ppmfree(&my_ppm);
}

#undef A2