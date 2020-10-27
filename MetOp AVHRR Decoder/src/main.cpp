#include <iostream>
#include <fstream>
#include <complex>
#include <vector>
#include <ccsds/demuxer.h>
#include <ccsds/vcdu.h>
#include "avhrr_reader.h"

// #include "tclap/CmdLine.h"
#include <tclap/CmdLine.h>

// Processing buffer size
#define BUFFER_SIZE (12966)

// Return filesize
size_t getFilesize(std::string filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::size_t fileSize = file.tellg();
    file.close();
    return fileSize;
}

// IO files
std::ifstream data_in;

int main(int argc, char *argv[])
{
    TCLAP::CmdLine cmd("MetOp AVHRR decoder by Aang23", ' ', "1.1");

    // File arguments
    TCLAP::ValueArg<std::string> valueInput("i", "input", "Demuxed frames input", true, "", "demuxedframes.bin");
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

    // Complete filesize
    size_t filesize = getFilesize(valueInput.getValue());

    // Read buffer
    libccsds::CADU cadu;

    // Counters
    uint64_t avhrr_cadu = 0, ccsds = 0, avhrr_ccsds = 0;

    // Graphics
    std::cout << "---------------------------" << std::endl;
    std::cout << "    MetOp AVHRR Decoder" << std::endl;
    std::cout << "         by Aang23" << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;

    std::cout << "Demultiplexing and deframing..." << std::endl;

    libccsds::Demuxer ccsdsDemuxer(882, true);

    AVHRRReader reader;

    // Read until EOF
    while (!data_in.eof())
    {
        // Read buffer
        data_in.read((char *)&cadu, 1024);

        // Parse this transport frame
        libccsds::VCDU vcdu = libccsds::parseVCDU(cadu);

        // Right channel? (VCID 30/42 is MODIS)
        if (vcdu.vcid == 9)
        {
            avhrr_cadu++;

            // Demux
            std::vector<libccsds::CCSDSPacket> ccsdsFrames = ccsdsDemuxer.work(cadu);

            // Count frames
            ccsds += ccsdsFrames.size();

            // Push into processor (filtering APID 64)
            for (libccsds::CCSDSPacket &pkt : ccsdsFrames)
            {
                if (pkt.header.apid == 103 || pkt.header.apid == 104)
                {
                    avhrr_ccsds++;
                    reader.work(pkt);
                }
            }
        }

        // Show our progress
        std::cout << "\rProgress : " << round(((float)data_in.tellg() / (float)filesize) * 1000.0f) / 10.0f << "%     " << std::flush;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "VCID 9 (AVHRR) Frames  : " << avhrr_cadu << std::endl;
    std::cout << "CCSDS Frames           : " << ccsds << std::endl;
    std::cout << "AVHRR CCSDS Frames     : " << avhrr_ccsds << std::endl;
    std::cout << "AVHRR Lines            : " << reader.lines << std::endl;

    std::cout << std::endl;

    // Write images out
    std::cout << "Writing images... (Can take a while)" << std::endl;

    cimg_library::CImg<unsigned short> image1 = reader.getChannel(0);
    cimg_library::CImg<unsigned short> image2 = reader.getChannel(1);
    cimg_library::CImg<unsigned short> image3 = reader.getChannel(2);
    cimg_library::CImg<unsigned short> image4 = reader.getChannel(3);
    cimg_library::CImg<unsigned short> image5 = reader.getChannel(4);

    std::cout << "Channel 1..." << std::endl;
    image1.save_png("AVHRR-1.png");

    std::cout << "Channel 2..." << std::endl;
    image2.save_png("AVHRR-2.png");

    std::cout << "Channel 3..." << std::endl;
    image3.save_png("AVHRR-3.png");

    std::cout << "Channel 4..." << std::endl;
    image4.save_png("AVHRR-4.png");

    std::cout << "Channel 5..." << std::endl;
    image5.save_png("AVHRR-5.png");

    std::cout << "221 Composite..." << std::endl;
    cimg_library::CImg<unsigned short> image221(2048, reader.lines, 1, 3);
    {
        image221.draw_image(0, 0, 0, 0, image2);
        image221.draw_image(0, 0, 0, 1, image2);
        image221.draw_image(0, 0, 0, 2, image1);
        image221.equalize(1000);
        image221.normalize(0, std::numeric_limits<unsigned char>::max());
    }
    image221.save_png("AVHRR-RGB-221.png");

    std::cout << "321 Composite..." << std::endl;
    cimg_library::CImg<unsigned short> image321(2048, reader.lines, 1, 3);
    {
        image321.draw_image(0, 0, 0, 0, image3);
        image321.draw_image(0, 0, 0, 1, image2);
        image321.draw_image(0, 0, 0, 2, image1);
        image321.equalize(1000);
        image321.normalize(0, std::numeric_limits<unsigned char>::max());
    }
    image321.save_png("AVHRR-RGB-321.png");
    // finalImage.save_png(outputImage);

    data_in.close();
}