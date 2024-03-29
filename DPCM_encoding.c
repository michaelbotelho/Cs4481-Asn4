#include "DPCM_encoding_function.h"


int main (int argc, char* argv[])
{
    float avg_abs_error_Ptr = 0;
    float std_abs_error_Ptr = 0;

	uintmax_t start;	/* starting tick value */
	uintmax_t end;		/* ending tick value */


    // Call DPCM encoding function
	start = clock(); 	/* mark the start time */
    Encode_Using_DPCM(argv[1], atoi(argv[2]), &avg_abs_error_Ptr, &std_abs_error_Ptr);
	end = clock(); 		/* mark the end time */


    // Report statistics
    printf("Average of Absolute Prediction Errors: %f\n", avg_abs_error_Ptr);
    printf("Standard Deviation of Absolute Prediction Errors: %f\n", std_abs_error_Ptr);
	printf("Compression time: %8.6f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}