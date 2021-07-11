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

    cout<<"Starting Jpeg Test\n";
    camera_fb_t imageTest;
    imageTest.format = PIXFORMAT_RGB888;
    imageTest.height = 480;
    imageTest.width = 640;
    imageTest.len = sizeof(rgb_image_bin);
    imageTest.buf = (uint8_t *)rgb_image_bin;

    cout<<"Deleting output.jpeg\n";
    remove ("output.jpeg");

    if (file == nullptr)
        if ((file = fopen("output.jpeg","wb")) == NULL)
        {
           printf("Error! opening file");
           return 0;
       }

    cout<<"Converting RGB 888 to JPEG\n";

    frame2jpg_cb(&imageTest, 100, jpg_encode_file, nullptr, nullptr);
    fclose(file);

    ///////////////// Convertion Done ///////////////////

    //////// Converting from JPEG to RGB 888 ////////////
    cout<<"Converting JPEG to RGB 888 \n";
    uint32_t fileTagSize = GetFileSize("tag.jpeg");
    uint8_t* jpegTagFile = (uint8_t*)malloc(fileTagSize);
    memset(jpegTagFile, 0, fileTagSize);

    if ((file = fopen("tag.jpeg","rb")) == NULL)
    {
        printf("Error! opening file");
        return 0;
    }

    size_t result = fread(jpegTagFile, 1, fileTagSize, file);

    // Resolution 640x480 * RGB = 3 bytes
    static uint8_t tag888[324*36*3];
    memset(tag888, 0, 324*36*3);

    if (fmt2rgb888(jpegTagFile, fileTagSize, pixformat_t::PIXFORMAT_JPEG, tag888) == false)
    {
        printf("Error! converting jpg file to frame 888");
        return 0;
    }
    fclose(file);

    JDEC jdec;
    if (get_jpeg_decoder(jpegTagFile, fileTagSize, JPG_SCALE_NONE, &jdec) == false)
    {
        printf("Error!!!!!\n");
        return false;
    }

    ///////////////// Convertion Done ///////////////////
    return 0;
    uint32_t fileSize = GetFileSize("output.jpeg");
    uint8_t* jpegFile = (uint8_t*)malloc(fileSize);
    // Converting from JPEG to RGM 888///////////////////

    if ((file = fopen("output.jpeg","rb")) == NULL)
    {
        printf("Error! opening file");
        return 0;
    }

    result = fread(jpegFile, 1, fileSize, file);

    // Resolution 640x480 * RGB = 3 bytes
    static uint8_t result888[640*480*3];
    memset(result888, 0, 640*480*3);

    if (fmt2rgb888(jpegFile, fileSize, pixformat_t::PIXFORMAT_JPEG, result888) == false)
    {
        printf("Error! converting jpg file to frame 888");
        return 0;
    }
    
    ///////////////// Convertion Done ///////////////////
     fclose(file);



    return 0;
}
