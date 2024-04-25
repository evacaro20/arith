/*************************************************************************
 *
 *                     arith40.h
 *
 *     Assignment: arith
 *     Authors:  CS40 Tufts Course Staff
 *     Date:     3/6/24
 *
 *     Functions that convert between index and chroma values for pixels.
 *
 *************************************************************************/

#ifndef ARITH40_INCLUDED
#define ARITH40_INCLUDED

extern unsigned Arith40_index_of_chroma(float chroma);
extern float    Arith40_chroma_of_index(unsigned n);

extern void Arith40_test_chroma_map(float chroma);

#endif
