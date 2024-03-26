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


    // Declare variables for prediction, error, neighboring pixels, visited pixels, unique values counter, other value (binary mode), and gradients (continuous-tone mode)
    unsigned char prediction, error;
    unsigned char* neighbors = calloc(7,  sizeof(unsigned char)); // Indices are WW, W, NW, N, NE, NN, NNE respectively
    unsigned char* visited = calloc(7,  sizeof(unsigned char));
    int count_distinct = 0;
    unsigned char s1;
    unsigned char dh, dv;

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
                    error = img.image[h][w] - prediction;
                    // Write to encoded file
                }
                // Handle remaining pixels in the first row
                else 
                {
                    prediction = img.image[h][w - 1]; // West pixel
                    error = img.image[h][w] - prediction;
                    // Write to encoded file
// DEBUG printf("%u\n", prediction);
                }
            }
            // Handle pixels in second row, remaining pixels in the first two columns, and the remaining pixels in the last column
            else if (h == 1 || w < 2 || w == img.width - 1)
            {
                prediction = img.image[h - 1][w]; // North pixel
                error = img.image[h][w] - prediction;
                // Write to encoded file
            }
            // Handle all other pixels in the image
            else 
            {  
                // Use West pixel for prediction
                if (prediction_rule == 1) 
                {
                    prediction = img.image[h][w - 1]; // West pixel
                    error = img.image[h][w] - prediction;
                    // Write to encoded file
                }
                // Use North pixel for prediction
                else if (prediction_rule == 2)
                {
                    prediction = img.image[h - 1][w]; // North pixel
                    error = img.image[h][w] - prediction;
                    // Write to encoded file
                }
                // Use W/2 + N/2 for prediction
                else if (prediction_rule == 3)
                {   
                    prediction = (img.image[h][w - 1] / 2) + (img.image[h - 1][w] / 2); // W/2 + H/2
                    error = img.image[h][w] - prediction;
                    // Write to encoded file
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
                        // Encoded file:
                        // T0 T1 T2 error error
                        // Where if T2 is read by decoder it automatically knows the next value is the error 
                        // Do other prediction rules (1, 2, and 3) also mean to write prediction error or just CALIC ??
                        
                        // T = 0 if current pixel = W
                        if (img.image[h][w] == neighbors[1]) 
                        {
                            // Write T0 to encoded file
                        }
                        // T = 1 if current pixel = s1 (other value)
                        else if (img.image[h][w] == s1)
                        {
                            // Write T1 to encoded file
                        }
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
                        }

                        error = img.image[h][w] - prediction;
                        // Write to encoded file (along with T2 indicator)
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

                        error = img.image[h][w] - prediction;
                        // Write to encoded file
                    }
                }
            }
        } // next column (w)
    } // next row (h)


    free(neighbors);
    free(visited);
    free_PGM_Image(&img);
}
