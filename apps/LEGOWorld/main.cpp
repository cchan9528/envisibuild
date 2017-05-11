#include "legoworld.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>          // Media I/O
#include <iostream>

using namespace std;
int main(int argc, char** argv)
{
    // Verify the Frame Source
    if(argc!=2)
    {
        cout<<"\nUsage: ./LEGOFinder [filename]\n"<<endl;
        return -1;
    }

    // Load Frame from Source
    cv::Mat frame = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);
    if(!frame.data)
    {
        cout<<"\nRead Frame Error.\nLEGOFinder Terminated\n"<<endl;
        return -1;
    }

    // Determine Current Materials
    lw::Colortab tabs[5] = {
        {.c = green,  .sCount = 0, .rCount = 0},
        {.c = blue,   .sCount = 0, .rCount = 0},
        {.c = red,    .sCount = 0, .rCount = 0},
        {.c = yellow, .sCount = 0, .rCount = 0},
        {.c = white,  .sCount = 0, .rCount = 0}
    };
    lw::countPieces(frame, tabs, 5);
    lw::materialsReport(tabs, 5);
}
