#include "mean_absolute_error.h"


float mean_absolute_error(char* file_name_1_ptr, char* file_name_2_ptr) {
    // Loading images from given file names, halting program if either file is not found
    struct PGM_Image img1, img2;
    if (load_PGM_Image(&img1, file_name_1_ptr) == -1) {
        printf("ERROR: cannot open the given file '%s'.\n", file_name_1_ptr);
        return -1;
    }
    if (load_PGM_Image(&img2, file_name_2_ptr) == -1) {
        printf("ERROR: cannot open the given file '%s'.\n", file_name_2_ptr);
        return -1;
    }


    // Halting program if dimensions are not the same
    if (img1.width != img2.width || img1.height != img2.height) {
        printf("ERROR: cannot calculate mean absolute error on images of different dimensions.\n");
        return -1;
    } 


    // Scale up the image with smaller max gray value
    if (img1.maxGrayValue < img2.maxGrayValue) {
        float scale_factor = img2.maxGrayValue / img1.maxGrayValue;

        img1.maxGrayValue = img2.maxGrayValue;
        for (int h = 0; h < img1.height; h++) {
            for (int w = 0; w < img1.width; w++) {
                img1.image[h][w] = (int) round(img1.image[h][w] * scale_factor);
            }
        }
    }
    else if (img1.maxGrayValue > img2.maxGrayValue) {
        float scale_factor = img1.maxGrayValue / img2.maxGrayValue;

        img2.maxGrayValue = img1.maxGrayValue;
        for (int h = 0; h < img2.height; h++) {
            for (int w = 0; w < img2.width; w++) {
                img2.image[h][w] = (int) round(img2.image[h][w] * scale_factor);
            }
        }
    }


    // Calculate mean absolute error
    float mae = (float) (1.0f/(img1.width * img1.height));
    float sum = 0;
    for (int x = 0; x < img1.width; x++) {
        for (int y = 0; y < img1.height; y++) {
            sum += abs(img1.image[y][x] - img2.image[y][x]);
        }
    }

    mae *= sum;

    return mae;
}