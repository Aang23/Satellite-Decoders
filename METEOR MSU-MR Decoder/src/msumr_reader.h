#pragma once

#include <cstdint>

#define cimg_use_png
#define cimg_display 0
#include "CImg.h"

class MSUMRReader
{
private:
    unsigned short *channels[6];
    uint16_t msumrBuffer[6][1572];

public:
    MSUMRReader();
    int lines;
    void work(uint8_t *buffer);
    cimg_library::CImg<unsigned short> getChannel(int channel);
};