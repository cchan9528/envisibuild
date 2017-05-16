#include "legoworld.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>          // Media I/O
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
int main(int argc, char** argv)
{
    // Verify the Frame Source
    if(argc!=3)
    {
        cout<<"\nUsage: ./LEGOWorld [framesource] [filename]\n"<<endl;
        return -1;
    }
    char * framesource = argv[1];
    project_t projectName = lw::strToProject(argv[2]);

    // Keep Tabs on Current Materials
    lw::Colortab colortabs[5] = {
        {.c = red,  .sCount = 0, .rCount = 0},
        {.c = yellow,   .sCount = 0, .rCount = 0},
        {.c = green,    .sCount = 0, .rCount = 0},
        {.c = blue, .sCount = 0, .rCount = 0},
        {.c = white,  .sCount = 0, .rCount = 0}
    };

    // Load Frame from Source
    cv::Mat frame = cv::imread(framesource, CV_LOAD_IMAGE_COLOR);
    if(!frame.data)
    {
        cout<<"\nRead Frame Error.\nLEGOFinder Terminated\n"<<endl;
        return -1;
    }

    if(lw::projectPossible(projectName,frame,colortabs,NUM_COLORS))
        lw::materialsReport(colortabs, NUM_COLORS);

    // DEBUG
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(), .15, .15);
    cv::imshow("original", resized);
    // END DEBUG
}
