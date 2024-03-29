#include "libpnm.h"
#include "math.h"
#include "stdint.h"
#include "time.h"


#ifndef _DPCM_ENCODING_FUNCTION_
#define _DPCM_ENCODING_FUNCTION_

void Encode_Using_DPCM (char* in_PGM_filename_Ptr, int prediction_rule, float* avg_abs_error_Ptr, float* std_abs_error_ptr);

#endif /* _DPCM_ENCODING_FUNCTION_ */