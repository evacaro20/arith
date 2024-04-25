# arith
PPM Image file lossy compressor/decompressor. Results in minimal (&lt;2%) loss.

//////////////////////////////////////////////////////////////////////////////
                                ACKNOWLEDGMENTS
//////////////////////////////////////////////////////////////////////////////
 TAs at Office Hours: Dan, Naomi, Rohun, and others!


//////////////////////////////////////////////////////////////////////////////
                                ARCHITECTURE
//////////////////////////////////////////////////////////////////////////////
 40image.c, which was provided to me, contains the main executable and handles
 command-line argument processing. It then calls the functions in compress40, 
 which either compress or decompress a file based on the user's input. 

COMPRESSION:
 If the user wants to use compression, compress40.c it will call on the 
 compress40 function. This function calls on 3 separate files (int.c, float.c,
 and codewords.c respectively). In int.c, the program first trims the given
 image if the width and/or height are odd. If either is odd, it will map over
 the original array and put the values into a new array that is of the trimmed 
 size. It then deals will converting the Pnm_rgb values into their component 
 video values by doing the arithmetic and placing the comp video values (Y, pB,
 and pR) in a struct and placing those structs into a new array. This becomes 
 the new "pixels" for the Pnm_ppm and is passed back into compress40.c. 
 Then compress40 calls on the second file: float.c. In this file, the program 
 converts from component video format (Y, pB, and pR) to their scaled DCT 
 values (a, b, c, d, avg pB, and avg pR). There is significant loss here 
 because the program is shrinking the image. The average of 4 pixels is placed
 into 1 pixel, pB and pR are quantized, and a, b, c, and d values are scaled 
 from floats to unsigned and signed integer values (a goes from [0, 1] to 
 [0, 511] and b, c, and d go from [-0.5, 0.5] to [-15, 15]). This new shrunken 
 Pnm_ppm is passed back to compress40.c. Finally, compress40 calls on the third 
 file called codewords.c. This file utilizes Bitpack to pack the scaled DCT 
 values into 32-bit codewords. After packing the codewords it will print them 
 out to stdout. compress40.c then frees the Pnm_ppm. 

DECOMPRESSION:
 If the user wants to use decompression, compress40.c will call on the 
 decompress40 function. This function calls on 3 separate files (codewords.c,
 float.c, and int.c respectively). First, the header is read in by decompress40
 and a new ppm is created with an empty pixels array (height and width reflect
 the final array's height and width). Then, the codewords are read in byte by 
 byte from the input by codewords.c. Bitpack is used to turn the bytes into 
 codewords, and the codewords are stored in the pixels array. Then, the 
 codewords are unpacked from 32-bit sequences to a 9-bit unsigned a, 5-bit 
 signed b, c, and d, and 4-bit unsigned pB and pR values. Decompress40 then 
 feeds the ppm to float.h, which turns the scaled DCT values into comp video 
 values by unscaling a, b, c, and d back to their original ranges, unquantizing
 pB and pR, and applying the inverse DCT equations to the a, b, c, and d values
 to calculate  the Y for four pixels. In this step, 1 pixel gets mapped to a 
 2-by-2 block of pixels, where all four resulting pixels will have the same pB
 and pR (as they were averaged in compression) and their own Y values, so the 
 resulting pixels array is 4 times the size of the one it wasd provided. Next, 
 decompress40 feeds the ppm to int.h, which converts the pixels from comp video
 float form to red, green, and blue unsigned int values through a series of 
 conversion formulas. Finally, decompress40 prints the resulting, decompressed
 ppm to standard output. 


