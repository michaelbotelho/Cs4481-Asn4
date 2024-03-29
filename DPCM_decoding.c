#include "DPCM_decoding_function.h"


int main (int argc, char* argv[])
{
	uintmax_t start;	/* starting tick value */
	uintmax_t end;		/* ending tick value */


    // Call DPCM decoding function
    start = clock(); 	/* mark the start time */
    Decode_Using_DPCM(argv[1]);
	end = clock(); 		/* mark the end time */


    // Report decompression time    
	printf("Decompression time: %8.6f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}