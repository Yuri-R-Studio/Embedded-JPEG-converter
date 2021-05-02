#include <iostream>
#include "frame_buffer_example.h"
#include "custom_types.h"
#include "img_converters.h"

using namespace std;

int main()
{
    cout<<"Hello World";
    camera_fb_t imageTest;
    imageTest.format = PIXFORMAT_RGB888;
    imageTest.height = 480;
    imageTest.width = 640;
    imageTest.len = sizeof(rgb_image_bin);
    imageTest.buf = (uint8_t *)rgb_image_bin;

    remove ("output.jpeg");

    if (file == nullptr)
        if ((file = fopen("output.jpeg","wb")) == NULL)
        {
           printf("Error! opening file");
           return 0;
       }

    frame2jpg_cb(&imageTest, 100, jpg_encode_file, nullptr);
    fclose(file);
    return 0;
}
