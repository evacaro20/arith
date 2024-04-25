/*************************************************************************
 *
 *                     uarray2.c
 *
 *     Assignment: iii
 *     Authors:  Kevin Yu and Ava Sim 
 *     Date:     2/3/24
 *
 *
 *     Implemention of UArray2_T.h. Here are all of the member functions of our
 *     UArray2_T. If the client wants to use a UArray2_T, they should include 
 *     uarray2.h in their files. 
 *
 *
 *************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "uarray.h"
#include "mem.h"
#include "assert.h"
#include "uarray2.h"

struct UArray2_T_struct {
        int width;
        int height;
        int size;
        UArray_T data;
};

/********** UArray2_new ****************************************************
 *
 * This function creates a new UArray2 given the width, height, and size of
 * every element.
 *
 * Parameters:
 *      int width               width of the array  
 *      int height              height of the array
 *      int size                size of each element in the array
 *
 * Return: the allocated UArray2
 *
 * Expects: width and height are >= 0, and size is the amount of memory
 *          each element requires (and is greater than 0)
 *    
 * Notes: this function allocates memory
 *    
 *************************************************************************/
UArray2_T UArray2_new(int width, int height, int size)
{
        /* check that inputs are valid */
        assert(width >= 0);
        assert(height >= 0);
        assert(size > 0);
        /* Create the struct and fill it out */
        UArray2_T new_UArray2 = ALLOC(sizeof(struct UArray2_T_struct));
        new_UArray2->width = width;
        new_UArray2->height = height;
        new_UArray2->size = size;
        /* The length of this 1D UArray is width * height */
        new_UArray2->data = UArray_new(width * height, size);

        return new_UArray2;
}

/********** UArray2_free ****************************************************
 *
 * This function frees memory associated with the given UArray2, clears it.
 * 
 * Parameters:
 *     UArray2_T *uarray2_p                given array to free
 *
 * Return: none
 *
 * Expects: the array is not already freed, CRE for the pointer to be NULL,
 *          and that the pointer that the input pointer is pointing to 
 *          cannot be NULL
 *     
 * Notes: this function frees memory
 *     
 *************************************************************************/
void UArray2_free(UArray2_T *uarray2_p) 
{
        assert(uarray2_p != NULL && *uarray2_p != NULL);
        UArray_free(&((*uarray2_p)->data));

        FREE(*uarray2_p);
}

/********** UArray2_width ****************************************************
 *
 * This function returns the width of a given array.
 * 
 * Parameters:
 *      UArray2_T array2                given array
 *
 * Return: an integer that is the width of the array
 *
 * Expects: width is positive (CRE if not), that the uarray is not NULL (CRE
 *          if not)
 *     
 * Notes: 
 *     
 *************************************************************************/
int UArray2_width(UArray2_T uarray2)
{
        assert(uarray2 != NULL);
        assert(uarray2->width >= 0);
        return uarray2->width;
}

/********** UArray2_height **************************************************
 *
 * This function returns the height of a given array.
 * 
 * Parameters:
 *      UArray2_T array2                given array
 *
 * Return: an integer that is the height of the array
 *
 * Expects: height is positive (CRE if not), that the uarray is not NULL (CRE
 *          if not)
 *     
 * Notes: 
 *     
 *************************************************************************/
int UArray2_height(UArray2_T uarray2)
{
        assert(uarray2 != NULL);
        assert(uarray2->height >= 0);
        return uarray2->height;
}


/********** UArray2_size ****************************************************
 *
 * This function returns the size of the elements of a uarray2.
 * 
 * Parameters:
 *      UArray2_T array2                given array
 *
 * Return: an integer that is the size of the array
 *
 * Expects: size is positive (CRE if not), that the uarray is not NULL (CRE
 *          if not)
 *     
 * Notes: 
 *     
 *************************************************************************/
int UArray2_size(UArray2_T uarray2)
{
        assert(uarray2 != NULL);
        assert(uarray2->size > 0);
        return uarray2->size;
}

/********** UArray2_at ****************************************************
 *
 * This function gets an element at the specified position (i, j) from the 
 * specified UArray2
 * 
 * Parameters:
 *      UArray2_T uarray2               given array
 *      int column                      position i 
 *      int row                         position j 
 *
 * Return: a void pointer pointing to the element at the specified position
 *
 * Expects: column and row are in range (CRE if not), uarray is not NULL
 *     
 * Notes:
 *     
 *************************************************************************/
void *UArray2_at(UArray2_T uarray2, int column, int row)
{
        assert(uarray2 != NULL);
        assert(0 <= column && column < uarray2->width);
        assert(0 <= row && row < uarray2->height);
        return UArray_at(uarray2->data, column * uarray2->height + row);
}

/********** UArray2_map_col_major *****************************************
 *
 * This function iterates through the UArray2 in a column major format, meaning
 * it goes down columns before rows (the row changes faster). This function
 * calls the apply function on each element. Includes a closure parameter
 * if needed, which is something the client can use to keep track of values.
 *
 * Parameters:
 *      UArray2 uarray2                   given array
 *      void apply ()                     the pointer function
 *              apply(int column)         column position if needed by client
 *              apply(int row)            row position if needed by client
 *              apply(UArray2_T)          the original UArray if needed by 
 *                                        client
 *              apply(void *val)          element in UArray if needed by client
 *              apply(void *cl)           closure if needed by client  
 *     void *cl                           closure, passed to apply
 *        
 *
 * Return: None
 *
 * Expects: uarray2 is not NULL (CRE if it is), that the apply function is
 *          valid
 *    
 * Notes: Apply function supplies client with anything they need
 *    
 *************************************************************************/

 void UArray2_map_col_major(UArray2_T uarray2, void apply(int column, int row, 
                            UArray2_T uarray2, void *val, void *cl), void *cl)
{
        assert(uarray2 != NULL);
        for (int column = 0; column < uarray2->width; column++) {
                for (int row = 0; row < uarray2->height; row++) {
                        apply(column, row, uarray2, UArray2_at(uarray2, column, 
                                                               row), cl);
                }
        }
}


/********** UArray2_map_row_major ****************************************
 *
 * This function iterates through the UArray2 in a row major format, meaning
 * it goes across rows then columns (the columns changes faster). This function
 * calls the apply function on each element. Includes a closure parameter
 * if needed, which is something the client can use to keep track of values.
 *
 * Parameters:
 *      UArray2 uarray2                    given array
 *      void apply ()                      the pointer function
 *              apply(int column)          column position if needed by client
 *              apply(int row)             row position if needed by client
 *              apply(UArray2_T)           the original UArray if needed by 
 *                                         client
 *              apply(void *val)           element in UArray2 if needed by 
 *                                         client
 *              apply(void *cl)            closure if needed by client  
 *     void *cl                            closure, passed to apply
 *        
 *
 * Return: None
 *
 * Expects: uarray2 is not NULL (CRE if it is), that the apply function is
 *          valid
 *    
 * Notes: Apply function supplies client with anything they need
 *    
 *************************************************************************/

void UArray2_map_row_major(UArray2_T uarray2, void apply(int column, int row, 
                            UArray2_T uarray2, void *val, void *cl), void *cl)
{      
        assert(uarray2 != NULL);
        for (int row = 0; row < uarray2->height; row++) {
                for (int column = 0; column < uarray2->width; column++) {
                        apply(column, row, uarray2, UArray2_at(uarray2, column,
                                                               row), cl);
                }
        }
}

