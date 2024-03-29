#include "DPCM_decoding_function.h"


void Decode_Using_DPCM (char* in_filename_Ptr)
{
    // Open given file and Parse headers
    char buffer[256];
    FILE *compressed_fptr = fopen(in_filename_Ptr, "r");
    if(compressed_fptr == NULL) exit(0);
    int prediction_rule, width, height, max_gray_value; 
    fscanf(compressed_fptr, "%d", &prediction_rule); // Read prediction rule
    fscanf(compressed_fptr, "%d %d", &width, &height); // Read width and height
    fscanf(compressed_fptr, "%d", &max_gray_value); // Read max gray value
    

    // Read Prediction Errors
    int* errors = calloc(width * height, sizeof(int));
    int error, pixels = 0;
    while (fscanf(compressed_fptr, "%d", &error) == 1)
    {
        errors[pixels] = error;
        pixels++;
    }

    // Read T_values if CALIC rule passed
    int* T_values = calloc(width * height, sizeof(int));
    if (prediction_rule == 4) 
    {
        char c;
        fscanf(compressed_fptr, "%c", &c); // Skip comma separating prediction errors and T-values
        int t_val;
        pixels = 0;
        while (fscanf(compressed_fptr, "%d", &t_val) == 1)
        {
            T_values[pixels] = t_val;
            pixels++;
        }
    }


    // Setup empty PGM image file
    struct PGM_Image img;
    create_PGM_Image(&img, width, height, max_gray_value);

    // Declare variables for decoding algorithm
    int prediction;
    int* neighbors = calloc(7,  sizeof(int)); // Indices are WW, W, NW, N, NE, NN, NNE respectively
    int* visited = calloc(7,  sizeof(int));
    int count_distinct = 0;
    int pixel = 0;
    int s1;
    int dh, dv;

    // Decode pixels in Raster scan fashion
    for (int h = 0; h < img.height; h++) 
    {
        for (int w = 0; w < img.width; w++) 
        {
            // Handle pixels in first row
            if (h == 0) 
            {
                // Handle the first pixel (top left corner)
                if (w == 0) 
                {
                    prediction = 128;
                }
                // Handle remaining pixels in the first row
                else 
                {
                    prediction = img.image[h][w - 1]; // West pixel
                }
                img.image[h][w] = errors[pixel] + prediction;
            }
            // Handle pixels in second row, remaining pixels in the first two columns, and the remaining pixels in the last column
            else if (h == 1 || w < 2 || w == img.width - 1)
            {
                prediction = img.image[h - 1][w]; // North pixel
                img.image[h][w] = errors[pixel] + prediction;
            }
            // Handle all other pixels in the image
            else 
            {  
                // Use West pixel for prediction
                if (prediction_rule == 1) 
                {
                    prediction = img.image[h][w - 1]; // West pixel
                    img.image[h][w] = errors[pixel] + prediction;
                }
                // Use North pixel for prediction
                else if (prediction_rule == 2)
                {
                    prediction = img.image[h - 1][w]; // North pixel
                    img.image[h][w] = errors[pixel] + prediction;
                }
                // Use W/2 + N/2 for prediction
                else if (prediction_rule == 3)
                {   
                    prediction = (img.image[h][w - 1] / 2) + (img.image[h - 1][w] / 2); // W/2 + H/2
                    img.image[h][w] = errors[pixel] + prediction;
                }
                // Use CALIC for prediction
                else if (prediction_rule == 4)
                {
                    // Clearing visited array and counter for finding unique values
                    for (int i = 0; i < 7; i++) 
                    {
                        visited[i] = 0;
                    }
                    count_distinct = 0;
                    
                    // Getting 7 neighboring pixels
                    neighbors[0] = img.image[h][w - 1];     // W
                    neighbors[1] = img.image[h][w - 2];     // WW
                    neighbors[2] = img.image[h - 1][w - 1]; // NW
                    neighbors[3] = img.image[h - 1][w];     // N
                    neighbors[4] = img.image[h - 1][w + 1]; // NE
                    neighbors[5] = img.image[h - 2][w];     // NN
                    neighbors[6] = img.image[h - 2][w + 1]; // NNE

                    // Check distinct values in neighboring pixels
                    for(int i = 0; i < 7; i++)
                    {
                        // Only if unvisited
                        if(visited[i] == 0)
                        { 
                            // Getting vale other than W for binary mode (will be ignored if > 2 unique values are found)
                            s1 = neighbors[i];
                            for(int j = i + 1; j < 7; j++){
                                // If item appears again in the array
                                if(neighbors[i] == neighbors[j]){
                                    // Mark visited
                                    visited[j] = 1;
                                }
                            }
                            // Increase count of unique values as current array item counted
                            count_distinct++;
                        }
                    }


                    // Binary mode
                    if (count_distinct < 3) 
                    {
                        // T = 0 if current pixel = W
                        if (T_values[pixel] == 0) img.image[h][w] = img.image[h][w - 1];
                        // T = 1 if current pixel = s1 (other value)
                        else if (T_values[pixel] == 1) img.image[h][w] = s1;
                        // T = 2 otherwise (escape signal to continuous-tone mode)
                        else 
                        {
                            dh = abs(neighbors[0] - neighbors[1]) + abs(neighbors[3] - neighbors[2]) + abs(neighbors[4] - neighbors[3]); // |W–WW| + |N–NW| + |NE–N| 
                            dv = abs(neighbors[0] - neighbors[2]) + abs(neighbors[3] - neighbors[5]) + abs(neighbors[4] - neighbors[6]); // |W–NW| + |N–NN| + |NE–NNE|

                            if (dv - dh > 80) // Sharp horizontal edge
                            {
                                prediction = neighbors[0]; // W
                            }
                            else 
                            {
                                if (dh - dv > 80) // Sharp vertical edge
                                {
                                    prediction = neighbors[3]; // N
                                }
                                else 
                                {   
                                    prediction = ((neighbors[0] + neighbors[3]) / 2) + ((neighbors[4] - neighbors[2]) / 4); // (W + N)/2 + (NE – NW)/4
                                    if (dv - dh > 32) // Horizontal edge
                                    {   
                                        prediction = (prediction / 2) + (neighbors[0] / 2); // (1/2 × Prediction + 1/2 × W)
                                    }
                                    else if (dh - dv > 32) // Vertical edge
                                    {
                                        prediction = (prediction / 2) + (neighbors[3] / 2); // (1/2 × Prediction + 1/2 × N)
                                    }
                                    else if (dv - dh > 8) // Weak horizontal edge
                                    {
                                        prediction = (3 * prediction / 4) + (neighbors[0] / 4); // (3/4 × Prediction + 1/4 × W)
                                    }
                                    else if (dh - dv > 8) // Weak vertical edge
                                    {
                                        prediction = (3 * prediction / 4) + (neighbors[3] / 4); // (3/4 × Prediction + 1/4 × N)
                                    }
                                }
                            }
                            img.image[h][w] = errors[pixel] + prediction;
                        }
                    }

                    // Continuous-tone mode
                    else 
                    {
                        dh = abs(neighbors[0] - neighbors[1]) + abs(neighbors[3] - neighbors[2]) + abs(neighbors[4] - neighbors[3]); // |W–WW| + |N–NW| + |NE–N| 
                        dv = abs(neighbors[0] - neighbors[2]) + abs(neighbors[3] - neighbors[5]) + abs(neighbors[4] - neighbors[6]); // |W–NW| + |N–NN| + |NE–NNE|

                        if (dv - dh > 80) // Sharp horizontal edge
                        {
                            prediction = neighbors[0]; // W
                        }
                        else 
                        {
                            if (dh - dv > 80) // Sharp vertical edge
                            {
                                prediction = neighbors[3]; // N
                            }
                            else 
                            {   
                                prediction = ((neighbors[0] + neighbors[3]) / 2) + ((neighbors[4] - neighbors[2]) / 4); // (W + N)/2 + (NE – NW)/4
                                if (dv - dh > 32) // Horizontal edge
                                {   
                                    prediction = (prediction / 2) + (neighbors[0] / 2); // (1/2 × Prediction + 1/2 × W)
                                }
                                else if (dh - dv > 32) // Vertical edge
                                {
                                    prediction = (prediction / 2) + (neighbors[3] / 2); // (1/2 × Prediction + 1/2 × N)
                                }
                                else if (dv - dh > 8) // Weak horizontal edge
                                {
                                    prediction = (3 * prediction / 4) + (neighbors[0] / 4); // (3/4 × Prediction + 1/4 × W)
                                }
                                else if (dh - dv > 8) // Weak vertical edge
                                {
                                    prediction = (3 * prediction / 4) + (neighbors[3] / 4); // (3/4 × Prediction + 1/4 × N)
                                }
                            }
                        }
                        img.image[h][w] = errors[pixel] + prediction;
                    }
                }
            }
            pixel++;
        } // next column (w)
    } // next row (h)


    // Create file name and save PGM image
    char image_file_name[256];
    snprintf(image_file_name, 256, "%s.pgm", in_filename_Ptr);
    save_PGM_Image(&img, image_file_name, 1);


    // Free memory
    fclose(compressed_fptr);

    free(errors);
    free(T_values);
    free(neighbors);
    free(visited);
    free_PGM_Image(&img);
}
