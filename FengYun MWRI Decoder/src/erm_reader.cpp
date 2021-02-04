#include "erm_reader.h"

#include <iostream>

ERMReader::ERMReader()
{
    staticChannel = new unsigned short[10000 * 151];
    imageVector.push_back(ERMImage());
}

void ERMReader::work(libccsds::CCSDSPacket &packet)
{
    if (packet.payload.size() < 1018)
        return;

    int counter = packet.payload[36] & 0b00011111;
    int mk = (packet.payload[36] >> 5) & 1;

    std::cout << "CNT" << counter << " MK " << mk << std::endl;

    /*
    int pos = 38 + 32 * 2;

    for (int i = 0; i < 151; i++)
    {
        int line_number = (lines * 16) + counter * 4 + (counter % 4);
        channels[0][line_number * 151 + i] = packet.payload[pos + 0] << 8 | packet.payload[pos + 1];
        pos += 2;
    }

    pos = 38 + 233 * 2;

    for (int i = 0; i < 151; i++)
    {
        channels[1][lines * 151 + i] = packet.payload[pos + 0] << 8 | packet.payload[pos + 1];
        pos += 2;
    }

    // Frame counter
    if (counter == 31)
        lines++;
    */

    if (imageVector[imageVector.size() - 1].mk == -1)
        imageVector[imageVector.size() - 1].mk = mk;

    if (mk == imageVector[imageVector.size() - 1].mk)
    {

        imageVector[imageVector.size() - 1].lastMkMatch = counter;

        int pos = 38 + 32 * 2;

        for (int i = 0; i < 151; i++)
        {
            int line_number = (lines * 16) + counter * 4 + (counter % 4);
            // channels[0][line_number * 151 + i] = packet.payload[pos + 0] << 8 | packet.payload[pos + 1];
            imageVector[imageVector.size() - 1].imageData[counter * 151 + i] = packet.payload[pos + 0] << 8 | packet.payload[pos + 1];
            pos += 2;
        }

        if (mk == 1)
            imageVector[imageVector.size() - 1].imageData[counter * 151 + 0] = 6000;
        else
            imageVector[imageVector.size() - 1].imageData[counter * 151 + 0] = 4000;
    }

    if (counter == 31)
    {
        imageVector.push_back(ERMImage());
    }
}

cimg_library::CImg<unsigned short> ERMReader::getChannel(int channel)
{
    if (channel == 0)
    {
        cimg_library::CImg<unsigned short> img(151, imageVector.size() * 4, 1, 1);

        for (int i = 0; i < imageVector.size(); i++)
        {
            std::cout << "Channel " << (i + 1) << std::endl;
            imageVector[i].getImage().save_png(std::string("ERM_CH-" + std::to_string(i + 1) + ".png").c_str());
        }

        int line = 0;

        for (int cnt = 0; cnt < imageVector.size(); cnt++)
        {
            for (int i = 0; i < 4; i++)
            {
                std::memcpy(&img.data()[line * 151], &imageVector[imageVector.size() - cnt].getImage().data()[(imageVector[imageVector.size() - cnt].lastMkMatch - i) * 151], 2 * 151);
                line++;
            }
        }

        img.equalize(1000);
        img.normalize(0, 65535);
        img.mirror('x');

        return img;
    }
    else
    {
        cimg_library::CImg<unsigned short> img = cimg_library::CImg<unsigned short>(staticChannel, 151, lines * 32);
        img.normalize(0, 65535);
        //img.equalize(1000);
        return img;
    }
}

cimg_library::CImg<unsigned short> ERMImage::getImage()
{
    cimg_library::CImg<unsigned short> img(imageData, 151, 32);
    //img.equalize(1000);
    //img.normalize(0, 65535);
    return img;
}