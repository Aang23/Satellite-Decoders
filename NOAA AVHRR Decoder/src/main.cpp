#include <iostream>
#include <fstream>
#include <complex>
#include <vector>

#define cimg_use_png
#define cimg_display 0
#include "CImg.h"
// #include "tclap/CmdLine.h"
#include <tclap/CmdLine.h>

// Processing buffer size
#define BUFFER_SIZE (11090 * 2)

// Returns the asked bit!
template <typename T>
inline bool getBit(T &data, int &bit)
{
    return (data >> bit) & 1;
}

int main(int argc, char *argv[])
{
    TCLAP::CmdLine cmd("NOAA AVHRR decoder by Aang23", ' ', "1.1");

    // File arguments
    TCLAP::ValueArg<std::string> valueInput("i", "input", "AVHRR frames input", true, "", "avhrrframes.raw16");
    TCLAP::ValueArg<std::string> valueOutput("o", "output", "Output image", true, "", "image.png");

    // Register all of the above options
    cmd.add(valueOutput);
    cmd.add(valueInput);

    // Parse
    try
    {
        cmd.parse(argc, argv);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cout << e.error() << '\n';
        return 0;
    }

    // Output and Input file
    std::ifstream data_in(valueInput.getValue(), std::ios::binary);
    char outputImage[valueOutput.getValue().size() + 1];
    strcpy(outputImage, valueOutput.getValue().c_str());

    // Read buffer
    uint16_t buffer[BUFFER_SIZE / 2];

    // Frame counter
    int frame = 0;

    // This will need some fixes
    unsigned short *imageBufferR = new unsigned short[10000 * 2048];
    unsigned short *imageBufferG = new unsigned short[10000 * 2048];
    unsigned short *imageBufferB = new unsigned short[10000 * 2048];

    // Read until EOF
    while (!data_in.eof())
    {
        // Read buffer
        data_in.read((char *)buffer, BUFFER_SIZE);

        int pos = 750; // AVHRR Data, eg, User Data Field

        // Channel R
        for (int i = 0; i < 2048; i++)
        {
            uint16_t pixel = buffer[pos + i * 5 + 1];
            imageBufferR[frame * 2048 + i] = pixel * 60;
        }

        // Channel G
        for (int i = 0; i < 2048; i++)
        {
            uint16_t pixel = buffer[pos + i * 5 + 1];
            imageBufferG[frame * 2048 + i] = pixel * 60;
        }

        // Channel B
        for (int i = 0; i < 2048; i++)
        {
            uint16_t pixel = buffer[pos + i * 5 + 0];
            imageBufferB[frame * 2048 + i] = pixel * 60;
        }

        // Frame counter
        frame++;
    }

    // Print it all out into a .png
    cimg_library::CImg<unsigned short> finalImage(2048, frame, 1, 3);

    cimg_library::CImg<unsigned short> channelImageR(&imageBufferR[0], 2048, frame);
    cimg_library::CImg<unsigned short> channelImageG(&imageBufferG[0], 2048, frame);
    cimg_library::CImg<unsigned short> channelImageB(&imageBufferB[0], 2048, frame);

    finalImage.draw_image(0, 0, 0, 0, channelImageR);
    finalImage.draw_image(0, 0, 0, 1, channelImageG);
    finalImage.draw_image(0, 0, 0, 2, channelImageB);

    finalImage.save_png(outputImage);

    data_in.close();
}