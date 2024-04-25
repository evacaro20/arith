/*************************************************************************
 *
 *                     bitpack.c
 *
 *     Assignment: arith
 *     Author:  Eva Caro
 *     Date:     3/6/24
 *
 *     Implementation of bitpack. Here are all of the member functions.
 *     Bitpack uses uint64_T and int64_T. 
 *
 *************************************************************************/

#include "bitpack.h"
#include "assert.h"
#include "arith40.h"
#include "except.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };

const unsigned MAX = 64;

/********** Bitpack_fitsu ********************************************
 *
 * This function checks whether a given unsigned integer can be represented in
 * a specified number of bits. 
 *
 * Parameters:
 *      uint64_t n           the unsigned integer to check
 *      unsigned width       the specified value that n might fit into
 *
 * Return: a boolean value denoting whether n fits in width bits
 *
 * Expects: width is not bigger than 64
 *     
 * Notes:  N/A
 *      
 *******************************************************************/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= MAX);

        if (width <= 0) {
                return false;
        }
        if ((n >> width) > 0) {
            return false;
        } else {
            return true;
        }
}

/********** Bitpack_fitss ********************************************
 *
 * This function checks whether a given signed integer can be represented in a
 * specified number of bits. 
 *
 * Parameters:
 *      int64_t n               the signed integer to check
 *      unsigned width          the specified value that n might fit into
 *
 * Return: boolean value indicating whether n fits in width bits
 *
 * Expects: width is not greater than 64
 *     
 * Notes: N/A
 *      
 *******************************************************************/
bool Bitpack_fitss(int64_t n, unsigned width)
{
        assert(width <= MAX);

        if (width <= 0) {
                return false;
        }

        int64_t upper = (1 << (width - 1)) - 1;
        int64_t neg_1 = -1;
        int64_t lower = (neg_1 << (width - 1));

        if (n > upper || n < lower) {
                return false;
        }
        return true;
}

/********** Bitpack_getu ********************************************
 *
 * This function extracts a single width-bit value with least significant 
 * bit at lsb in a given word, which is represented as a uint64_t. 
 *
 * Parameters:
 *      uint64_t word     the unsigned integer to check
 *      unsigned width    the specified value that n might fit into
 *      unsigned lsb      the least significant bit of the value to return
 *
 * Return: a uint64_t containing the width-bit field of a word starting at lsb
 *
 * Expects: width should not be greater than 64, and width + lsb should 
 *          also be less than 64.
 *     
 * Notes:  A width of 0 means the word represents the value 0. Words are 
 *         represented in big-endian order.
 *      
 *******************************************************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb) 
{
        assert(width <= MAX);
        assert(width + lsb <= MAX);
        word = (word >> lsb);

        if (width == 0) {
                return 0;
        }

        long long n = 1;
        for (int i = 0; i < (int)width; i++) {
            n *= 2;
        }
        uint64_t cast = n - 1;
        return (word & cast);
} 

/********** Bitpack_gets ********************************************
 *
 * This function extracts a width-bit signed value from a given uint64_t with
 * least significant bit at lsb.
 *
 * Parameters:
 *      uint64_t word     the unsigned integer to check
 *      unsigned width    the specified value that n might fit into
 *      unsigned lsb      the position in word of the lsb of the value we want 
 *                        to return
 *
 * Return: an int64_t containing the width-bit field of the word, starting at 
           position lsb
 *
 * Expects: width should not be greater than 64, and width + lsb should 
 *          also be less than 64.
 *     
 * Notes:  A width of 0 means the word represents the value 0. Words are 
 *         represented in big-endian order.
 *      
 *******************************************************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= MAX);
        assert(width + lsb <= MAX);
        word = word >> lsb;
        if (width == 0) {
                return 0;
        }

        long long n = 1;
        for (int i = 0; i < (int)width - 1; i++) {
                n *= 2;
        }

        if ((n & word) == 0) {
                return Bitpack_getu(word, width, 0); /* already shifted */
        } else {
                signed new_word = Bitpack_getu(word, width, 0) - (2 * n);
                return new_word;
        }
}


/********** Bitpack_newu ********************************************
 *
 * This function returns a new word which is identical to the original word,
 * except that the field of size width with least significant bit at lsb will 
 * have been replaced by a width-bit representation of value and value is of 
 * type unsigned int.
 *
 * Parameters:
 *      uint64_t word            the word given
 *      unsigned width           the width of the new unsigned word
 *      unsigned lsb             the least significant bit
 *      uint64_t value           the new width-bit
 *
 * Return: a uint64_t where the width with the lsb have been replaced by 
 * width-bit representation of value    
 *
 * Expects: value is in width, RAISE if not, 0 <= width <= 64, CRE if not, 
 * width + lsb <= 64, CRE if not
 *     
 * Notes: 
 *      
 *******************************************************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
        assert(width <= MAX);
        assert(width + lsb <= MAX);


        value = Bitpack_getu(value, width, 0);

        if (Bitpack_fitsu(value, width) == false) {
                RAISE(Bitpack_Overflow);
        } 

        /* making the bits to the right of the field be all 1's */

        uint64_t right_mask = 1;
        right_mask = ((right_mask << lsb) - 1);
         
        /* making the bits to the left of the field be all 1's */
        uint64_t left_mask = 1;
        left_mask = (((left_mask << (MAX - width - lsb)) - 1) << 
                        (width + lsb));

        uint64_t mask = right_mask | left_mask;
        word = word & mask;

        
        value = (value << lsb);
        return (value | word);

}


/********** Bitpack_news ********************************************
 *
 * This function returns a new word which is identical to the original word,
 * except that the field of size width with least significant bit at lsb will 
 * have been replaced by a width-bit representation of value and value is 
 * of type signed int.
 *
 * Parameters:
 *      uint64_t word            the word given
 *      unsigned width           the width of the new unsigned word
 *      unsigned lsb             the least significant bit
 *      int64_t value            the new width-bit
 *
 * Return: a uint64_t where the width with the lsb have been replaced by 
 * width-bit representation of value    
 *
 * Expects: value is in width, RAISE if not, 0 <= width <= 64, CRE if not, 
 * width + lsb <= 64, CRE if not
 *     
 * Notes: Calls Bitpack_newu to do the conversion after verifying whether value
 *        is positive or negative.
 *      
 *******************************************************************/

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value) 

{
        assert(width <= MAX);
        assert(width + lsb <= MAX);

        if (value > 0) {
                return Bitpack_newu(word, width, lsb, value);
        } else {

                if (Bitpack_fitss(value, width) == false) {
                        RAISE(Bitpack_Overflow);
                } 
                uint64_t u_value = 0;
                u_value = value + (1 << width);
                return Bitpack_newu(word, width, lsb, u_value);
        }
}
