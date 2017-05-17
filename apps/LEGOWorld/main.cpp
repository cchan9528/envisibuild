#include "legoworld.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>          // Media I/O
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
int main(int argc, char** argv)
{
    if(argc<2)
    {
        cout<<"\nUsage: ./LEGOWorld [filename]\n"<<endl;
        return -1;
    }
    else if(argc == 2)
    {
        project_t projectName = lw::strToProject(argv[1]);
        if(projectName==none)
        {
            lw::projectPossible(none, cv::Mat(), NULL, -1);
            return -1;
        }

        // Establish Frame Source
        cv::VideoCapture camera;
        camera.open(0);
        // for(int i = 0; !camera.isOpened(); i++ )
        // {
        //     if(i==1000){cout<<"\nFailed to connect camera!\n"<<endl; return 1;}
        //     camera.open(i);   // Auto-releases Before Opening Next
        // }

        // Process Frames
        while(1)
        {
            cv::Mat frame; camera >> frame;
            cv::imshow("Live Video", frame);
            if(cv::waitKey(30) >= 0)
                break;
        }
        return 0;
    }
    else if(argc == 3)
    {
        // Determine Desired Project
        project_t projectName = lw::strToProject(argv[2]);
        if(projectName==none)
        {
            lw::projectPossible(none, cv::Mat(), NULL, -1);
            return -1;
        }

        // Keep Tabs on Current Materials
        lw::Colortab colortabs[5] = {
            {.c = red,  .sCount = 0, .rCount = 0},
            {.c = yellow,   .sCount = 0, .rCount = 0},
            {.c = green,    .sCount = 0, .rCount = 0},
            {.c = blue, .sCount = 0, .rCount = 0},
            {.c = white,  .sCount = 0, .rCount = 0}
        };

        char * framesource = argv[1];

        if(fileType(framesource)==IMAGE)
        {
            // Load Frame
            cv::Mat frame = cv::imread(framesource, CV_LOAD_IMAGE_COLOR);
            if(!frame.data)
            {
                cout<<"\nRead Frame Error.\nLEGOFinder Terminated\n"<<endl;
                return -1;
            }
            //
            // // Process
            // lw::projectPossible(projectName,frame,colortabs,NUM_COLORS);
            // lw::materialsReport(colortabs, NUM_COLORS);

            lw::Workspace ws;
            lw::buildWorkspace(frame, &ws);
            lw::drawWorkspace(frame, &ws);

            int numsteps;
            if(projectName == stripedcube)
                numsteps = NUMSTEPS_STRIPEDCUBE;
            else if(projectName == staircase)
                numsteps = NUMSTEPS_STAIRCASE;
            else
                numsteps = NUMSTEPS_TOWER;
            for(int i=0; i < numsteps; i++)
            {
                cout<<"Step "<<i+1<<": "<<endl;
                lw::Instruction * nextInstr = lw::getInstrStep(projectName, i);
                lw::drawNextInstr(frame, &ws, nextInstr);
                cout<<endl<<endl<<endl;
            }
            // ~~~~~ DEBUG ~~~~~
            cv::Mat resized;
            cv::resize(frame, resized, cv::Size(), .5, .5);
            cv::imshow("original", resized);
            // ~~~ END DEBUG ~~~
            while(!lw::clearWorkspace(frame,&ws)){};

        }
        else
        {
            // Establish Frame Source
            cv::VideoCapture video(framesource);

            // Configure Workspace
            cv::Mat frame; video >> frame;
            lw::Workspace ws; lw::buildWorkspace(frame, &ws);

            // Begin Building
            while(video.isOpened())
            {
                video >> frame;
                // lw::projectPossible(projectName,frame,colortabs,NUM_COLORS);
                // lw::materialsReport(colortabs, NUM_COLORS);

                 // Need Clear Workspace
                if(!lw::clearWorkspace(frame,&ws))
                    continue;



                if(cv::waitKey(30) >= 0)
                    break;
            }
            return 0;
        }
    }
}
