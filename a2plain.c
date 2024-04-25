/*************************************************************************
 *
 *                     a2plain.c
 *
 *     Assignment: locality
 *     Author:   Eva Caro
 *     Date:     3/6/24
 *
 *
 *     Implementation of a2plain.c, a suite of methods that use the
 *     A2 methods to create and use an array using the UArray2 implementation
 *
 *
 ***************************************************************************/
#include <string.h>
#include <a2plain.h>
#include "uarray2.h"
#include "assert.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/
typedef A2Methods_UArray2 A2; 

/* Struct */
struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

/****************** new *************************************************
 *
 * calls UArray2_new which returns a new Uarray2_T
 *
 * Parameters:
 *      int width: number of columns for this new Uarray2_T
 *      int height: number of rows in our new Uarray2_T
 *      int size: the amount of memory needed for each index
 *
 * Return: UArray2_T that has been created as an A2Methods_UArray2
 *
 * Expects
 *      width, height, size must be greater than zero.
 * Notes:
 *      may CRE if unable to allocate space or if expects not met
 ************************************************************************/
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/********** new_with_blocksize ******************************************
 *
 * calls UArray2_new which returns a new Uarray2_T
 *
 * Parameters:
 *      int width: number of columns for this new Uarray2_T
 *      int height: number of rows in our new Uarray2_T
 *      int size: the amount of memory needed for each index
 *      int blocksize: unused, this isn't a blocked implementation
 *
 * Return: UArray2_T that has been created as an A2Methods_UArray2
 *
 * Expects
 *      width, height, size must be greater than zero.
 * Notes:
 *      may CRE if unable to allocate space or if expects not met
 ************************************************************************/
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/****************** a2free ***********************************************
 *
 * Frees any/all allocated space used by the A2 array.
 *
 * Parameters:
 *      A2 *array2p: pointer to A2 array (UArray2_T)
 *
 * Return: none
 *
 * Expects
 *      valid, initialized UArray2_T with allocated, unfreed memory as array2p
 *
 * Notes:
 *      May CRE if given improper inputs
 ************************************************************************/
static void a2free(A2 *array2p)
{
        UArray2_free((UArray2_T *) array2p);
}

/********************* width *********************************************
 *
 * Returns the width (number of columns) of a given A2
 *
 * Parameters:
 *      A2 *array2p: pointer to A2 array (UArray2_T)
 *
 * Return: const int of the width
 *
 * Expects
 *      valid, initialized UArray2_T as array2
 *
 * Notes:
 *      may CRE if improper input given
 ************************************************************************/
static int width(A2 array2)
{
        return UArray2_width(array2);
}

/********************** height *****************************************
 *
 * Returns the height (number of rows) of a given A2
 *
 * Parameters:
 *      A2 *array2p: pointer to A2 array (UArray2_T)
 *
 * Return: const int of the height
 *
 * Expects
 *      valid, initialized UArray2_T as array2
 *
 * Notes:
 *      may CRE if improper input given
 ************************************************************************/
static int height(A2 array2)
{
        return UArray2_height(array2);
}

/****************** size ************************************************
 *
 * Returns the size (space in memory for each index) of a given A2
 *
 * Parameters:
 *      A2 *array2p: pointer to A2 array (UArray2_T)
 *
 * Return: const int of the size
 *
 * Expects
 *      valid, initialized UArray2_T as array2
 *
 * Notes:
 *      may CRE if improper input given
 ************************************************************************/
static int size(A2 array2)
{
        return UArray2_size(array2);
}

/*************** blocksize **********************************************
 *
 * Returns the blocksize of a given A2
 *
 * Parameters:
 *      A2 *array2p: pointer to A2 array (UArray2_T)
 *
 * Return: 1 (always 1 since this is not a blocked implementation)
 *
 * Expects
 *      valid, initialized UArray2_T as array2
 *
 * Notes:
 *      may CRE if improper input given
 ************************************************************************/
static int blocksize(A2 array2)
{
        assert(array2 != NULL);
        (void) array2; 
        return 1;
}

/********************* UArray2_at **************************************
 *
 * Returns the value in a given A2 (Uarray2_T) at a given index.
 *
 * Parameters:
 *      A2 array2: pointer to some UArray2_T stored as an A2
 *      const int xVal: x value of position in array2
 *      const int yVal: y value of position in array2
 *
 * Return: pointer to data held at those indices
 *
 * Expects
 *      initialized array with xVal, yVal within width and height
 * Notes:
 *      may CRE if array is uninitialized or xVal, yVal outside bounds
 ************************************************************************/
static A2Methods_Object *at(A2 array2, int xVal, int yVal)
{
        return UArray2_at(array2, xVal, yVal);
}

typedef void applyfun(int xVal, int yVal, UArray2_T array2, void *elem, 
                                                                    void *cl);


/************** map_row_major *********************************************
 *
 * Calls an apply function for each element in the A2Methods_UArray2.
 * Row indices vary more rapidly than column indices. 
 *
 * Parameters:
 *      A2Methods_Uarray2 array2: The array to be traversed
 *      A2Methods_applyfun apply: some function that will be called on 
 *                  every value of the UArray2
 *      void *cl: a void pointer passed by reference into each function
 *                call of the apply function
 * Return: none
 *
 * Expects
 *      valid, initialized A2Methods_UArray2
 *      implemented function, apply, cl given if necessary
 *
 * Notes:
 *      May CRE if given improper inputs
 ************************************************************************/
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (applyfun *)apply, cl);
}

/******************* map_col_major **************************************
 *
 * Calls an apply function for each element in the A2Methods_UArray2.
 * Column indices vary more rapidly than row indices. 
 *
 * Parameters:
 *      A2Methods_UArray2 array: The array to be traversed
 *      void apply: some function that will be called on every value of
 *                  the A2Methods_UArray2
 *      void *cl: a void pointer that can be passed by reference into
 *                     each call of the apply function
 *
 * Return: none
 *
 * Expects
 *      valid, initialized UArray2_T
 *      implemented function, apply, closure if necessary
 *
 * Notes:
 *      may CRE if given inproper inputs
 ************************************************************************/
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (applyfun *)apply, cl);
}

/************************* apply_small ***********************************
 *
 * Calls an apply function found in the closure for a given element
 *
 * Parameters:
 *      int i: width position of element given
 *      int j: height position of element given
 *      UArray2_T array: The array at this position
 *      void *elem: the element at this position
 *      void *vcl: a void pointer containing a closure of type small_closure
 *
 * Return: none
 *
 * Expects
 *      valid, initialized UArray2_T, with i, j inside array width, height,
 *      void *elem, pointing to the element at that position, and vcl a valid
 *      pointer of type small_closure
 *
 * Notes:
 *      May CRE if given inproper inputs
 ************************************************************************/
static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        assert(uarray2 != NULL);
        assert(vcl != NULL);
        assert(elem != NULL);
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

/******************* small_map_row_major *********************************
 *
 * Calls apply_small for each element in the A2Methods_UArray2 after making
 * a closure with the original apply and closure to pass through.
 * Row indices vary more rapidly than column indices. 
 *
 * Parameters:
 *      A2Methods_Uarray2 a2: The array to be traversed
 *      A2Methods_smallapplyfun apply: some function that will be called on 
 *                  every value of the UArray2
 *      void *cl: a void pointer passed by reference into each function
 *                call of the apply function
 * Return: none
 *
 * Expects
 *      valid, initialized A2Methods_UArray2
 *      implemented function, apply, cl given if necessary
 *
 * Notes:
 *      May CRE if given improper inputs
 ************************************************************************/
static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

/***************** small_map_col_major **********************************
 *
 * Calls apply_small for each element in the A2Methods_UArray2 after making
 * a closure with the original apply and closure to pass through.
 * Column indices vary more rapidly than row indices. 
 *
 * Parameters:
 *      A2Methods_UArray2 array: The array to be traversed
 *      void apply: some function that will be called on every value of
 *                  the A2Methods_UArray2
 *      void *cl: a void pointer that can be passed by reference into
 *                     each call of the apply function
 *
 * Return: none
 *
 * Expects
 *      valid, initialized UArray2_T
 *      implemented function, apply, closure if necessary
 *
 * Notes:
 *      may CRE if given inproper inputs
 ************************************************************************/
static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,                 /* blocksize ?*/
        at,
        map_row_major,                  
        map_col_major,           
        NULL,                      /* map_block_major */
        map_row_major,             /* map default */
        small_map_row_major,
        small_map_col_major,
        NULL,                     /* small_map_block_major */
        small_map_row_major,      /* small map default */
};

/* finally the payoff: here is the exported pointer to the struct */

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
