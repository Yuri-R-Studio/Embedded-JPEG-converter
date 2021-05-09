#include <iostream>
#include "frame_buffer_example.h"
#include "custom_types.h"
#include "img_converters.h"

using namespace std;


long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


int main()
{
    // Converting file RGB 888 to JPEG ///////////////////

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

    ///////////////// Convertion Done ///////////////////

    // Converting from JPEG to RGM 888///////////////////

    uint32_t fileSize = GetFileSize("output.jpeg");
    uint8_t* jpegFile = (uint8_t*)malloc(fileSize);
    memset(jpegFile, 0, fileSize);

    if ((file = fopen("output.jpeg","rb")) == NULL)
    {
        printf("Error! opening file");
        return 0;
    }

    size_t result = fread(jpegFile, 1, fileSize, file);

    static uint8_t result888[640*480*3];

    fmt2rgb888(jpegFile, fileSize, pixformat_t::PIXFORMAT_JPEG, result888);
    
    ///////////////// Convertion Done ///////////////////

    return 0;
}
