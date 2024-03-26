#include "mean_absolute_error.h"


int main(int argc, char* argv[]) {
    // Accept arguments
    char* pgm_image_1_file_name = argv[1];
    char* pgm_image_2_file_name = argv[2];

    // Comparing images
    float mae = mean_absolute_error(pgm_image_1_file_name, pgm_image_2_file_name);

    if (mae < 0) {
        return -1;
    }

    printf("MAE = %.2f", mae);
    
    return 0;
}