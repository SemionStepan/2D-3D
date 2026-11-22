#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"
#include <iostream>


using namespace std;

int main()
{
    int width, height, channels;

    unsigned char* image = stbi_load(
        "assets/input.jpg",
        &width,
        &height,
        &channels,
        0
    );

    if (image == nullptr) 
    {
        cout << "Failed to load image\n";
        return 1;
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * channels;

            unsigned char gray = static_cast<unsigned char>(
                0.299 * image[index] +
                0.587 * image[index + 1] +
                0.114 * image[index + 2]
            );

            image[index] = gray;
            image[index + 1] = gray;
            image[index + 2] = gray;
        }
    }

    if(stbi_write_jpg("assets/grayscale.jpg", width, height, channels, image, 100) == 0)
    {
        cout << "Error: image aint got saved" << endl;
    }
    stbi_image_free(image);
}