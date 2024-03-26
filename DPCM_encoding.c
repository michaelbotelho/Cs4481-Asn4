#include "DPCM_encoding_function.h"


int main (int argc, char* argv[])
{
    float avg_abs_error_Ptr = 0;
    float std_abs_error_ptr = 0;


    Encode_Using_DPCM(argv[1], atoi(argv[2]), &avg_abs_error_Ptr, &std_abs_error_ptr);
    
}