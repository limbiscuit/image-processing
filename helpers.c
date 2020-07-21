#include "helpers.h"
#include "stdbool.h"
#include "stddef.h"
#include "math.h"
#include "stdio.h"


// Function prototype
int make_window(int x, int y, int height, int width, RGBTRIPLE image[width][height], RGBTRIPLE window[3][3]);

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    /* Iterate over all pixels in image and take the average of the RGB values for
    the equivalent gray value */
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            RGBTRIPLE *pixel = &image[i][j];
            uint8_t gray = round((pixel->rgbtBlue + pixel->rgbtGreen + pixel->rgbtRed) / 3.00);
            pixel->rgbtBlue = gray;
            pixel->rgbtGreen = gray;
            pixel->rgbtRed = gray;
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    int adj_width = width / 2;

    // Iterate over left half of image to reflect to right side
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < adj_width; j++)
        {
            int pixel_column = width - 1 - j;

            // Swap pixels
            RGBTRIPLE temp = image[i][j];
            image[i][j] = image[i][pixel_column];
            image[i][pixel_column] = temp;
        }
    }
    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    // Declare temp RGBTRIPLE arrays for make_window and to copy new blur pixels
    RGBTRIPLE window[3][3];
    RGBTRIPLE temp[height][width];

    // Call make_window on all pixels in image, then apply blurring formula
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int blue = 0;
            int green = 0;
            int red = 0;

            int valid_count = make_window(i, j, height, width, image, window);

            // Take averages for each valid pixel's colors in the window
            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    blue += window[y][x].rgbtBlue;
                    green += window[y][x].rgbtGreen;
                    red += window[y][x].rgbtRed;
                }
            }
            temp[i][j].rgbtBlue = round((float) blue / valid_count);
            temp[i][j].rgbtGreen = round((float) green / valid_count);
            temp[i][j].rgbtRed = round((float) red / valid_count);
        }
    }

    // Copy new edge values from temp image to actual image
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = temp[i][j];
        }
    }

    return;
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    // Use Sobel operator and compute G_x & G_y
    int G_y[3][3] =
    {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    int G_x[3][3] =
    {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    // Declare temp RGBTRIPLE arrays for make_window and to copy new edge pixels
    RGBTRIPLE window[3][3];
    RGBTRIPLE temp[height][width];

    // Call make_window on all pixels and apply Sobel operator
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            make_window(i, j, height, width, image, window);

            int G_x_red = 0;
            int G_x_green = 0;
            int G_x_blue = 0;

            int G_y_red = 0;
            int G_y_green = 0;
            int G_y_blue = 0;

            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    G_x_red += G_x[y][x] * window[y][x].rgbtRed;
                    G_x_green += G_x[y][x] * window[y][x].rgbtGreen;
                    G_x_blue += G_x[y][x] * window[y][x].rgbtBlue;

                    G_y_red += G_y[y][x] * window[y][x].rgbtRed;
                    G_y_green += G_y[y][x] * window[y][x].rgbtGreen;
                    G_y_blue += G_y[y][x] * window[y][x].rgbtBlue;
                }
            }

            // Compute final Sobel value
            int sum_red = round(sqrt(pow(G_x_red, 2) + pow(G_y_red, 2)));
            int sum_green = round(sqrt(pow(G_x_green, 2) + pow(G_y_green, 2)));
            int sum_blue = round(sqrt(pow(G_x_blue, 2) + pow(G_y_blue, 2)));

            // Check for all pixels being within 0-255 range
            sum_red = (sum_red < 256) ? sum_red : 255;
            sum_green = (sum_green < 256) ? sum_green : 255;
            sum_blue = (sum_blue < 256) ? sum_blue : 255;

            temp[i][j].rgbtRed = sum_red;
            temp[i][j].rgbtGreen = sum_green;
            temp[i][j].rgbtBlue = sum_blue;
        }
    }

    // Copy new edge values from temp image to actual image
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = temp[i][j];
        }
    }
    return;
}

// Identify 3x3 box around selected pixel
int make_window(int y, int x, int height, int width, RGBTRIPLE image[height][width], RGBTRIPLE window[3][3])
{

    int valid_count = 0;

    // Initialize window array to include all black pixels
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            window[i][j].rgbtBlue = 0;
            window[i][j].rgbtGreen = 0;
            window[i][j].rgbtRed = 0;
        }
    }

    // Check conditions for if neighboring pixels are valid and within image boundaries
    bool top = (y > 0);
    bool left = (x > 0);
    bool right = (x < width - 1);
    bool bottom = (y < height - 1);


    // Copy selected pixel
    window[1][1] = image[y][x];
    valid_count++;

    // Top left corner pixel
    if (top && left)
    {
        window[0][0] = image[y - 1][x - 1];
        valid_count++;
    }

    // Top pixel
    if (top)
    {
        window[0][1] = image[y - 1][x];
        valid_count++;
    }

    // Top right corner pixel
    if (top && right)
    {
        window[0][2] = image[y - 1][x + 1];
        valid_count++;
    }

    // Left pixel
    if (left)
    {
        window[1][0] = image[y][x - 1];
        valid_count++;
    }

    // Right pixel
    if (right)
    {
        window[1][2] = image[y][x + 1];
        valid_count++;
    }

    // Bottom left pixel
    if (bottom && left)
    {
        window[2][0] = image[y + 1][x - 1];
        valid_count++;
    }

    // Bottom pixel
    if (bottom)
    {
        window[2][1] = image[y + 1][x];
        valid_count++;
    }

    // Bottom right pixel
    if (bottom && right)
    {
        window[2][2] = image[y + 1][x + 1];
        valid_count++;
    }

    return valid_count;
}

