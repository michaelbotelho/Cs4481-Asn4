#include "libpnm.h"


int main(int argc, char* argv[]) {
    char* img_file_name = argv[1];
    struct PGM_Image img;
    if (load_PGM_Image(&img, img_file_name) == -1) {
        printf("Error opening the given file.\n");
        return -1;
    }

    struct PGM_Image img2;
    copy_PGM(&img, &img2);
   
    
    for (int h = 0; h < img.height; h++) {
        for (int w = 0; w < img.width; w++) {
            if (img2.image[h][w] >= 127) {
                img2.image[h][w] = 255;
            }
            else {
                img2.image[h][w] = 0;
            }
        }
    }
    
    // Reference this file and the save_PGM_Image() function when working with raw file formats and encoding
    save_PGM_Image(&img2, "copy.pgm", 0);

    free_PGM_Image(&img2);
    free_PGM_Image(&img);
}