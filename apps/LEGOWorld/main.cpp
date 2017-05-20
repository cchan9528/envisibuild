#include "envisibuild.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>          // Media I/O
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;

int fileType(char * s)
{
    int i = 0;
    while(1) {i++; if(s[i]==0){break;}}
    if(i<5)
    {
        return -1;
    }
    else
    {
        switch(s[i-3])
        {
            case 'j':
            case 'J':
            case 'P':
            case 'p':
                return IMAGE;
            case 'm':
            case 'M':
                return VIDEO;
            default:
                cout<<"\n\nCouldn't Resolve FileType "<<s[i-4]<<"\n\n"<<endl;
                return -1;
        }
    }
}

int main(int argc, char** argv)
{
    if(argc<2)
    {
        cout<<"\nUsage: ./LEGOWorld [filename]\n"<<endl;
        return -1;
    }
    cout << endl << endl;
    cout << "*******************************"<<endl;
    cout << "\n\n   Welcome to Envisibuild!\n\n"<<endl;
    cout << "*******************************"<<endl;
    project_t p = strToProj(((argc==2) ? argv[1] : argv[2]));
    if(p==none)
    {
        cout<<endl<<"\'"<<((argc==2) ? argv[1] : argv[2])\
            <<"\' not yet available."<<endl\
            <<"Please choose an available project:\n"\
            <<"    . stripedcube\n"\
            <<"    . staircase\n"\
            <<"    . tower\n"<<endl<<endl;
        return -1;
    }
    cout << "\nBuilding " << argv[2] << "..."<<endl;

    if(argc == 2)
    {
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
        // Keep Tabs on Current Materials
        // lw::Colortab colortabs[5] = {
        //     {.c = red,  .sCount = 0, .rCount = 0},
        //     {.c = yellow,   .sCount = 0, .rCount = 0},
        //     {.c = green,    .sCount = 0, .rCount = 0},
        //     {.c = blue, .sCount = 0, .rCount = 0},
        //     {.c = white,  .sCount = 0, .rCount = 0}
        // };

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

            // Build Workspace
            Workspace ws;
            buildWorkspace(frame, &ws, p);
            drawWorkspace(frame, &ws);

            lw::Instruction * curInstr;
            while(!projectComplete(&ws))
            {
                cv::Mat clone = frame.clone();
                drawWorkspace(clone, &ws);

                curInstr = getInstrStep(p, ws.step);
                drawInstr(clone, &ws, curInstr);

                cv::Mat resized;
                cv::resize(clone, resized, cv::Size(), .5, .5);
                cv::imshow("original", resized);
                cv::resize(ws.bounds, resized, cv::Size(), .5, .5);
                cv::imshow("mask", resized);
                while(true)
                    if(cv::waitKey(30) == 27)
                        break;

                if(instrDone(frame, &ws, curInstr))
                    ws.step++;

                // ~~~~~ DEBUG ~~~~~

                // NOTE: WILL WORK IF:
                // - ONLY [1] IS ON (MASKING)
                //          AND ONLY IMAGE SECTION in [1] IS ON
                // - ONLY [2] IS ON (SEQUENCING)
                //          AND ALL OF [1] IS OFF

                // ~~~ END DEBUG ~~~
            }
        }
        else
        {
            // Establish Frame Source
            cv::VideoCapture video(framesource);

            // Configure Workspace
            cv::Mat frame; video >> frame;
            Workspace ws;
            buildWorkspace(frame, &ws, p);

            lw::Instruction * curInstr;
            while(!projectComplete(&ws) && video.isOpened())
            {
                video >> frame;
                cv::Mat clone = frame.clone();
                drawWorkspace(clone, &ws);

                curInstr = getInstrStep(p, ws.step);
                drawInstr(clone, &ws, curInstr);

                // ~~~~~ DEBUG ~~~~~
                cv::Mat resized;
                cv::resize(frame, resized, cv::Size(), .5, .5);
                cv::imshow("original", resized);
                // while(true)
                // ~~~ END DEBUG ~~~
                if(cv::waitKey(1) == 27)
                    break;

                if(instrDone(frame, &ws, curInstr))
                    ws.step++;

                // ~~~~~ DEBUG ~~~~~

                // NOTE: WILL ONLY WORK IF [2] IS OFF
                //          AND ONLY VIDEO SECTION in [1] IS ON
                // MASKING (BUT SLOWER FRAME RATE)

                // ~~~ END DEBUG ~~~
            }
        }
        cout << endl << endl;
        cout << "*******************************"<<endl;
        cout << "       Congratulations!\n     Project is Complete!"<<endl;
        cout << "*******************************"<<endl;
        cout << "\n\n Thanks for Using Envisibuild!\n\t     :)\n\n"<<endl;
        return 0;
    }
}
