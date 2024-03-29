#include "DPCM_encoding_function.h"


void Encode_Using_DPCM (char* in_PGM_filename_Ptr, int prediction_rule, float* avg_abs_error_Ptr, float* std_abs_error_ptr) 
{
    // Read file to pgm image and halt program if an error occurs
    struct PGM_Image img;
    if (load_PGM_Image(&img, in_PGM_filename_Ptr) == -1) 
    {
        printf("ERROR: cannot open the given file '%s'.\n", in_PGM_filename_Ptr);
        exit(0);
    }


    // Declare variables for encoding algorithm
    int prediction;
    int* abs_errors = calloc(img.maxGrayValue, sizeof(int));
    int* errors = calloc(img.width * img.height, sizeof(int));
    int* T_values = calloc(img.width * img.height, sizeof(int));
    int* neighbors = calloc(8,  sizeof(int)); // Indices are WW, W, NW, N, NE, NN, NNE, Current respectively
    int* visited = calloc(8,  sizeof(int));
    int count_distinct = 0;
    int pixel = 0;
    int s1;
    int dh, dv;


    // Iterate through pixels in Raster scan fashion 
    for (int h = 0; h < img.height; h++) 
    {
        for (int w = 0; w < img.width; w++) 
        {
            T_values[pixel] = 2; // Write default T = 2 to T_values array

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
                errors[pixel] = img.image[h][w] - prediction;
            }
            // Handle pixels in second row, remaining pixels in the first two columns, and the remaining pixels in the last column
            else if (h == 1 || w < 2 || w == img.width - 1)
            {
                prediction = img.image[h - 1][w]; // North pixel
                errors[pixel] = img.image[h][w] - prediction;
            }
            // Handle all other pixels in the image
            else 
            {  
                // Use West pixel for prediction
                if (prediction_rule == 1) 
                {
                    prediction = img.image[h][w - 1]; // West pixel
                    errors[pixel] = img.image[h][w] - prediction;
                }
                // Use North pixel for prediction
                else if (prediction_rule == 2)
                {
                    prediction = img.image[h - 1][w]; // North pixel
                    errors[pixel] = img.image[h][w] - prediction;
                }
                // Use W/2 + N/2 for prediction
                else if (prediction_rule == 3)
                {   
                    prediction = (img.image[h][w - 1] / 2) + (img.image[h - 1][w] / 2); // W/2 + H/2
                    errors[pixel] = img.image[h][w] - prediction;
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
                    neighbors[7] = img.image[h][w]; // Current Pixel


                    // Check distinct values in neighboring pixels
                    for(int i = 0; i < 8; i++)
                    {
                        // Getting value other than W for binary mode (will be ignored if > 2 unique values are found)
                        if (neighbors[i] != neighbors[0]) s1 = neighbors[i];

                        // Only if unvisited
                        if(visited[i] == 0)
                        { 
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
                        // Write T = 0 if current pixel = W
                        if (img.image[h][w] == neighbors[0]) T_values[pixel] = 0;
                        // Write T = 1 if current pixel = s1 (other value)
                        else if (img.image[h][w] == s1) T_values[pixel] = 1;
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
                            errors[pixel] = img.image[h][w] - prediction;
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
                        errors[pixel] = img.image[h][w] - prediction;
                    }
                }
            }
            pixel++;
        } // next column (w)
    } // next row (h)


    // Creating file names and writing file headers
    char compressed_file_name[256];
    char errors_file_name[256];
    snprintf(compressed_file_name, 256, "%s.%d.DPCM", in_PGM_filename_Ptr, prediction_rule);
    snprintf(errors_file_name, 256, "%s.%d.errors.csv", in_PGM_filename_Ptr, prediction_rule);

    FILE *compressed_fptr = fopen(compressed_file_name, "w");
    FILE *errors_fptr = fopen(errors_file_name, "w");
    fprintf(compressed_fptr, "%d\n", prediction_rule);
    fprintf(compressed_fptr, "%d %d\n", img.width, img.height);
    fprintf(compressed_fptr, "%d\n", img.maxGrayValue);
    fprintf(errors_fptr, "ABSOLUTE prediction error value,frequency\n"); // Prediction error histogram data  


    // Write prediction error array (and T values array if CALIC prediction rule applied) to compressed file
    // Simultaneously count prediction error frequencies and accumulate average
    int count = 0;
    for (int i = 0; i < img.width * img.height; i++)
    {
        fprintf(compressed_fptr, "%d ", errors[i]);
        if (T_values[i] >= 2) // Make sure value at errors[i] is relevant (no prediction error is sent whe T = 0 or T = 1)
        {
            abs_errors[abs(errors[i])]++;
            *avg_abs_error_Ptr += abs(errors[i]);
            count++;
        }
    }
    *avg_abs_error_Ptr /= count;
    fprintf(compressed_fptr, ",");
    // Write T_values array to compressed file if CALIC rule applied
    if (prediction_rule == 4)
    {
        for (int i = 0; i < img.width * img.height; i++)
        {
            fprintf(compressed_fptr, "%d ", T_values[i]);
        }
    }
    // Write Absolute errors to file and calculate avg_abs_errors
    for (int i = 0; i < img.maxGrayValue; i++)
    {
        if (abs_errors[i] > 0)
        {   
            *std_abs_error_ptr += abs(i - *avg_abs_error_Ptr) * abs(i - *avg_abs_error_Ptr);
            fprintf(errors_fptr, "%d,%d\n", i, abs_errors[i]);
        }
    }
    // Finish calculating standard deviation
    *std_abs_error_ptr = sqrt(*std_abs_error_ptr / count);


    // Free memory
    fclose(compressed_fptr);
    fclose(errors_fptr);

    free(abs_errors);
    free(errors);
    free(T_values);
    free(neighbors);
    free(visited);
    free_PGM_Image(&img);
}
