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

    cout << "Width: " << width << '\n';
    cout << "Height: " << height << '\n';
    cout << "Channels: " << channels << '\n';
    cout << "First pixel: "
     << (int)image[0] << ", "
     << (int)image[1] << ", "
     << (int)image[2] << '\n';
    stbi_image_free(image);
}