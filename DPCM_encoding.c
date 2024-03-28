#include "DPCM_encoding_function.h"


int main (int argc, char* argv[])
{
    float avg_abs_error_Ptr = 0;
    float std_abs_error_Ptr = 0;

    // Call DPCM encoding function
    Encode_Using_DPCM(argv[1], atoi(argv[2]), &avg_abs_error_Ptr, &std_abs_error_Ptr);

    // Report statistics
    printf("Average of Absolute Prediction Errors: %f\n", avg_abs_error_Ptr);
    printf("Standard Deviation of Absolute Prediction Errors: %f\n", std_abs_error_Ptr);
    printf("Compression time here ...\n");
    
}