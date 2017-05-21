#include "envisibuild.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>          // Media I/O
#include <opencv2/imgproc.hpp>
#include <iostream>

#define SUCCESS 0
#define FAILURE 1

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
        cout<<"\nUsage: ./envisibuild [filename] project_name\n"<<endl;
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
    cout << "\nBuilding " << ((argc==2) ? argv[1] : argv[2]) << "..."<<endl;

    char * framesource = argv[1];
    Workspace ws;
    if(argc == 2 || (argc == 3 && fileType(framesource)==VIDEO))
    {
        // Establish Frame Source
        cv::VideoCapture video;
        (argc==3) ? video.open(framesource) : video.open(0);
        // for(int i = 0; !camera.isOpened(); i++ )
        // {
        //     if(i==1000){cout<<"\nFailed to connect camera!\n"<<endl; return 1;}
        //     camera.open(i);   // Auto-releases Before Opening Next
        // }


        // Configure Workspace
        cv::Mat frame; video >> frame;
        buildWorkspace(frame, &ws, p);

        // Build
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
            cv::resize(clone, resized, cv::Size(), .5, .5);
            cv::imshow("original", resized);
            // while(true)
            // ~~~ END DEBUG ~~~
            if(cv::waitKey(1) == 27)
                break;

            if(instrDone(frame, &ws, curInstr))
                ws.step++;
        }
    }
    else
    {
        // Load Frame
        cv::Mat frame = cv::imread(framesource, CV_LOAD_IMAGE_COLOR);
        if(!frame.data)
        {
            cout<<"\nRead Frame Error.\nenvisibuild Terminated\n"<<endl;
            return -1;
        }

        // Build Workspace
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
        }
    }
    if(projectComplete(&ws))
    {
        cout << endl << endl;
        cout << "*******************************"<<endl;
        cout << "       Congratulations!\n     Project is Complete!"<<endl;
        cout << "*******************************"<<endl;
        cout << "\n\n Thanks for Using Envisibuild!\n\t     :)\n\n"<<endl;
        return SUCCESS;
    }
    return FAILURE;
}
