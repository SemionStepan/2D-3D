
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

#include <iostream>
#include <algorithm>

using namespace std;

unsigned char toByte(double value)
{
    return static_cast<unsigned char>(clamp(value, 0.0, 255.0));
}

void applyGrayscale(unsigned char* image, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 3;

            unsigned char gray = toByte(
                0.299 * image[index] +
                0.587 * image[index + 1] +
                0.114 * image[index + 2]
            );

            image[index] = gray;
            image[index + 1] = gray;
            image[index + 2] = gray;
        }
    }
}

void applySepia(unsigned char* image, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 3;

            double r = image[index];
            double g = image[index + 1];
            double b = image[index + 2];

            image[index] = toByte(0.393 * r + 0.769 * g + 0.189 * b);
            image[index + 1] = toByte(0.349 * r + 0.686 * g + 0.168 * b);
            image[index + 2] = toByte(0.272 * r + 0.534 * g + 0.131 * b);
        }
    }
}

void applyNegative(unsigned char* image, int width, int height)
{
    int totalPixels = width * height * 3;

    for (int i = 0; i < totalPixels; i++)
    {
        image[i] = 255 - image[i];
    }
}

int main()
{
    int width, height;

    unsigned char* image = stbi_load(
        "assets/input.jpg",
        &width,
        &height,
        nullptr,
        3
    );

    if (image == nullptr)
    {
        cout << "Failed to load image" << endl;
        return 1;
    }

    cout << "Image loaded: " << width << "x" << height << endl;

    cout << "\nChoose filter:" << endl;
    cout << "1. Grayscale" << endl;
    cout << "2. Sepia" << endl;
    cout << "3. Negative" << endl;

    int choice;
    cout << "Your choice: ";
    cin >> choice;

    switch (choice)
    {
        case 1:
            applyGrayscale(image, width, height);
            break;

        case 2:
            applySepia(image, width, height);
            break;

        case 3:
            applyNegative(image, width, height);
            break;

        default:
            cout << "Unknown filter" << endl;
            stbi_image_free(image);
            return 1;
    }

    if (stbi_write_jpg("assets/output.jpg", width, height, 3, image, 95) == 0)
    {
        cout << "Failed to save image" << endl;
        stbi_image_free(image);
        return 1;
    }

    cout << "Image saved: assets/output.jpg" << endl;

    stbi_image_free(image);

    return 0;
}