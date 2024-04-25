/*************************************************************************
 *
 *                     codewords.c
 *
 *     Assignment: arith
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *     Implementation of codewords. codewords.c works primarily on variables 
 *     that use 32-bit words. Facilitates conversion between 6 comp vid and 
 *     codewords. We call on Bitpack to help with these conversions.
 *
 *************************************************************************/
#include "codewords.h"
#include "a2methods.h"
#include "uarray2.h"
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "bitpack.h"

typedef A2Methods_UArray2 A2;

const int BYTE = 8;
const int SCALEDBCD = 5;
const int AFACTOR = 9;
const int ALSB = 23;
const int BLSB = 18;
const int CLSB = 13;
const int SCALEPBPR = 4;

/* Struct to help with closures */
typedef struct array_methods {
        /* array */
        A2 *array;
        /* method client wants to use */
        A2Methods_T methods;
} array_methods;

/* Struct of scaled DCT values with unsigned and signed values */
typedef struct scaled_dct {
        /* unsigned values for a, pB, and pR */
        unsigned a, pB, pR;
        /* signed b, c, and d components */
        signed b, c, d;
} scaled_dct;

/* Struct to help with reading in the compressed file and unpacking it */
typedef struct unpack_cl {
        /* file/input given */
        FILE *input;
        /* counter to check that num of bits is the same as width * height */
        int *counter;
} unpack_cl;

/* Exceptions to raise */
Except_T File_Too_Short = { "Supplied input does not match width and height" };

/********** codewords_parent ***********************************************
 *
 * This function is a parent function for everything in codeword.c. If client 
 * is compressing, function will pack the 6 components into 32-bit codewords.
 * It will then print out the compressed image. If client is decompressiong,
 * it will read in from the file and unpack the codeword into its 6 component
 * values.
 *
 * Parameters:
 *      Pnm_ppm my_ppm          the Pnm_ppm
 *      bool compress           if we are compressing or not
 *      FILE *input             file given for decompression
 *
 * Return: a Pnm_ppm with the new values in the array
 *
 * Expects: my_ppm is not NULL
 *     
 * Notes: Only use file if decompressing
 *      
 ***********************************************************************/
Pnm_ppm codewords_parent(Pnm_ppm my_ppm, bool compress, FILE *input)
{
        assert(my_ppm != NULL);
        A2 array = my_ppm->pixels;
        A2Methods_T methods = uarray2_methods_plain;
        if (compress) {
                my_ppm->pixels = pack(array, methods);
                printf("COMP40 Compressed image format 2\n%u %u\n", 
                        methods->width(my_ppm->pixels) * 2, 
                        methods->height(my_ppm->pixels) * 2);
                methods->map_row_major(my_ppm->pixels, apply_print, NULL);
        } else {
                unpack_cl u_c;
                u_c.input = input;
                int counter = 0;
                u_c.counter = &counter;
                void *cl = &u_c;
                methods->map_row_major(array, code_apply, cl);
                if (counter != (int)(my_ppm->width * my_ppm->height)) {
                        RAISE(File_Too_Short);
                }
                my_ppm->pixels = unpack(array, methods, my_ppm->width, 
                                        my_ppm->height);
        }
        return my_ppm;
}


/********** pack ********************************************************
 *
 * This function is a mapping function that creates a closure and then calls
 * on an apply function to change from the 6 components (a, b, c, d, pB, pR) 
 * to the 32-bit words. 
 *
 * Parameters:
 *      A2 array                the array given
 *      A2Methods_T             methods given
 *
 * Return: an array with packed codewords from 6 comps
 *
 * Expects: array is not NULL, methods not NULL
 *     
 * Notes: creating a new array that is not freed here, frees the old array,
 *        Compression
 *      
 ***********************************************************************/
A2 pack(A2 array, A2Methods_T methods)
{
        assert(array != NULL);
        assert(methods != NULL);
        A2 *new_array = methods->new(methods->width(array), 
                                     methods->height(array), 
                                     sizeof(uint64_t));
        array_methods a_m;
        a_m.array = new_array;
        a_m.methods = methods;
        void *cl = &a_m;
        methods->map_default(array, apply_pack, cl);
        /* frees the old array */
        methods->free(&array);
        return new_array;
}

/********** apply_pack ****************************************************
 *
 * This function is the apply function for our packing function. It takes the 
 * current element's a, b, c, d, pR, and pB values and uses Bitpack to convert
 * the element into a 32-bit word. It then places the word into the new array.
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
 * Expects: closure is not NULL, elem is not NULL, a, b, c, d, pR, and pB can
 *          fit into their corresponding bit values 
 *     
 * Notes: Compression
 *      
 *************************************************************************/
void apply_pack(int col, int row, A2 array, void *elem, void *cl)
{
        assert(elem != NULL);
        assert(cl != NULL);

        (void) array;
        array_methods *a_m = cl;
        A2 *new_array = a_m->array;
        A2Methods_T methods = a_m->methods;
        scaled_dct *elem_p = elem;
        
        assert(Bitpack_fitsu(elem_p->a, AFACTOR)); 
        assert(Bitpack_fitss(elem_p->b, SCALEDBCD));
        assert(Bitpack_fitss(elem_p->c, SCALEDBCD));
        assert(Bitpack_fitss(elem_p->d, SCALEDBCD));
        assert(Bitpack_fitsu(elem_p->pB, SCALEPBPR));
        assert(Bitpack_fitsu(elem_p->pR, SCALEPBPR));

        uint64_t codeword = Bitpack_newu(0, SCALEPBPR, 0, elem_p->pR); 

        codeword = Bitpack_newu(codeword, SCALEPBPR, SCALEPBPR, elem_p->pB);
        codeword = Bitpack_news(codeword, SCALEDBCD, BYTE, elem_p->d);
        codeword = Bitpack_news(codeword, SCALEDBCD, CLSB, elem_p->c);
        codeword = Bitpack_news(codeword, SCALEDBCD, BLSB, elem_p->b);
        codeword = Bitpack_newu(codeword, AFACTOR, ALSB, elem_p->a);

        *(uint64_t *)methods->at(new_array, col, row) = codeword;
}

/********** apply_print ****************************************************
 *
 * This function is the apply function for our packing function. It turns each
 * codeword into 4 bytes and uses putchar to print out the bytes accordingly.
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
 * Expects: elem is not NULL
 *     
 * Notes: Compression
 *      
 *************************************************************************/
void apply_print(int col, int row, A2 array, void *elem, void *cl)
{
        assert(elem != NULL);
        (void) cl;
        (void) col;
        (void) row;
        (void) array;
        uint64_t *elem_p = elem;
        uint64_t byte;
        byte = Bitpack_getu(*elem_p, BYTE, BYTE * 3);
        putchar(byte);
        byte = Bitpack_getu(*elem_p, BYTE, BYTE * 2);
        putchar(byte);
        byte = Bitpack_getu(*elem_p, BYTE, BYTE);
        putchar(byte);
        byte = Bitpack_getu(*elem_p, BYTE, 0);
        putchar(byte);

}


/********** code_apply ****************************************************
 *
 * This function is the apply function to get the codewords. This function 
 * reads in from the file (or stdin) byte by byte. Once it has
 * four bytes it will pack them as a codeword (4 byes is 32-bit word). It then
 * places them into the array given. 
 * 
 *
 * Parameters:
 *      int col                          column
 *      int row                          row
 *      A2 array                         the array
 *      void *elem                       elem at that position
 *      void *cl                         FILE INPUT
 *
 * Return: void
 *
 * Expects: closure is not NULL, elem is not NULL
 *     
 * Notes: Decompression
 *      
 *************************************************************************/
void code_apply(int col, int row, A2 array, void *elem, void *cl)
{
        assert(array != NULL);
        assert(cl != NULL);
        (void) elem;
        unpack_cl *u_cl = cl;
        FILE *input = u_cl->input;
        int *counter = u_cl->counter;
        uint64_t byte1, byte2, byte3, byte4, codeword;
        A2Methods_T methods = uarray2_methods_plain;

        byte1 = fgetc(input);
        byte2 = fgetc(input);
        byte3 = fgetc(input);
        byte4 = fgetc(input);
        codeword = Bitpack_newu(0, BYTE, 0, byte4);
        codeword = Bitpack_newu(codeword, BYTE, BYTE, byte3);
        codeword = Bitpack_newu(codeword, BYTE, BYTE * 2, byte2);
        codeword = Bitpack_newu(codeword, BYTE, BYTE * 3, byte1);
        *(uint64_t *)methods->at(array, col, row) = codeword;
        (*counter)++;
}

/********** unpack ********************************************************
 *
 * This function is a mapping function that creates a closure and then calls
 * on an apply function to change from the given codewords to 6 component 
 * video values. 
 *
 * Parameters:
 *      A2 array                the array given
 *      A2Methods_T             methods given
 *      int width               width of the array
 *      int height              height of the array
 *
 * Return: an array with 6 comps from given codewords
 *
 * Expects: array is not NULL, methods not NULL
 *     
 * Notes: creating a new array that is not freed here, frees the old array,
 *        new array is of type scaled dct, Decompression
 *      
 ***********************************************************************/
A2 unpack(A2 array, A2Methods_T methods, int width, int height)
{
        assert(array != NULL);
        assert(methods != NULL);

        A2 *new_array = methods->new(width, height, sizeof(struct scaled_dct));
        array_methods a_m;
        a_m.array = new_array;
        a_m.methods = methods;
        void *cl = &a_m;
        methods->map_row_major(array, apply_unpack, cl);
        methods->free(&array);
        return new_array;
}

/********** apply_unpack ****************************************************
 *
 * This function is the apply function for our unpacking function. It takes 
 * current element's codeword and uses Bitpack to unpack the codeword into its
 * a, b, c, d, pR, and pB values. It then places the word into the new array.
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
 * Notes: Decompression
 *      
 *************************************************************************/
void apply_unpack(int col, int row, A2 array, void *elem, void *cl)
{
        (void) array;
        assert(cl != NULL);
        assert(elem != NULL);
        uint64_t *elem_p = elem;
        scaled_dct new_elem;
        array_methods *a_m = cl;
        A2 new_array = a_m->array;
        A2Methods_T methods = a_m->methods;

        new_elem.a = Bitpack_getu(*elem_p, AFACTOR, ALSB);
        new_elem.b = Bitpack_gets(*elem_p, SCALEDBCD, BLSB);
        new_elem.c = Bitpack_gets(*elem_p, SCALEDBCD, CLSB);
        new_elem.d = Bitpack_gets(*elem_p, SCALEDBCD, BYTE);
        new_elem.pB = Bitpack_getu(*elem_p, SCALEPBPR, SCALEPBPR);
        new_elem.pR = Bitpack_getu(*elem_p, SCALEPBPR, 0);
       
        *(scaled_dct *)methods->at(new_array, col, row) = new_elem;
}

#undef A2 