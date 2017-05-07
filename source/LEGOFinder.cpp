#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "legohsvcolors.hpp"

using namespace std;

int main(int argc, char** argv)
{
    // Verify the Frame Source
    if(argc!=2)
    {
        cout << "\nUsage: HSVFinder filename" << endl;
        return -1;
    }

    // Load Frame from Source and Convert it to HSV
    cv::Mat frame = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Note: BGR on Load
    if(!frame.data)
    {
        cout << "\n\n\nCould not read frame from source!!!" << endl;
        cout << "HSVFinder Terminated on Read Image Error." << endl;
        return -1;
    }
    cv::Mat frameInHSV;
    cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);
    cout<<"Done converting to HSV."<<endl;

    // Define HSV LEGO Color Bounds
    cv::Scalar greenLB(GREEN_H_LO, GREEN_S_LO, GREEN_V_LO);
    cv::Scalar greenUB(GREEN_H_HI, GREEN_S_HI, GREEN_V_HI);
    cv::Scalar blueLB(BLUE_H_LO, BLUE_S_LO, BLUE_V_LO);
    cv::Scalar blueUB(BLUE_H_HI, BLUE_S_HI, BLUE_V_HI);
    cv::Scalar redLB(GREEN_H_LO, RED_S_LO, RED_V_LO);
    cv::Scalar redUB(RED_H_HI, RED_S_HI, RED_V_HI);
    cv::Scalar yellowLB(YELLOW_H_LO, YELLOW_S_LO, YELLOW_V_LO);
    cv::Scalar yellowUB(YELLOW_H_HI, YELLOW_S_HI, YELLOW_V_HI);
    cv::Scalar whiteLB(WHITE_H_LO, WHITE_S_LO, WHITE_V_LO);
    cv::Scalar whiteUB(WHITE_H_HI, WHITE_S_HI, WHITE_V_HI);

    // Create Threshold Masks for LEGO Colors and One Cumulative Mask
    cv::Mat greenMask;
    cv::inRange(frameInHSV, greenLB, greenUB, greenMask);
    cv::Mat blueMask;
    cv::inRange(frameInHSV, blueLB, blueUB, blueMask);
    cv::Mat redMask;
    cv::inRange(frameInHSV, redLB, redUB, redMask);
    cv::Mat yellowMask;
    cv::inRange(frameInHSV, yellowLB, yellowUB, yellowMask);
    cv::Mat whiteMask;
    cv::inRange(frameInHSV, whiteLB, whiteUB, whiteMask);
    cv::Mat cumMask = greenMask;
    cv::Mat masks[] = {greenMask, blueMask, redMask, yellowMask, whiteMask};
    for(int i = 0 ; i < 5; i++)
        cv::bitwise_or(cumMask, masks[i], cumMask);

    // Display the Masks
    string windowName[7] = {"Original", "Green Mask", "Blue Mask","Red Mask",
                            "Yellow Mask", "White Mask", "Cumulative Mask"};
    cv::Mat image[7] = {frame, greenMask, blueMask, redMask,
                        yellowMask, whiteMask, cumMask};
    for(int i = 0; i < 7; i++)
    {
        cv::Mat resized;
        cv::resize(image[i], resized, cv::Size(), .25, .25);
        cv::imshow(windowName[i], resized);
    }

    // For Observations
    while(true)
    {
        if(cv::waitKey(30) == 27)
        {
            cout<<"\n\n\nUser held esc key to terminate program"<<endl;
            break;
        }
    }

    // Exit
    cout<< "\n\n\nHSVFinder Terminated on Success." << endl;
    return 0;
}
