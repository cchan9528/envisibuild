#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "legohsvcolors.hpp"

#include AREA_THRESHOLD 5000        // Inherent Scale Variance

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
    cv::cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);

    // Define HSV LEGO Color Bounds
    cv::Scalar greenLB(GREEN_H_LO, GREEN_S_LO, GREEN_V_LO);
    cv::Scalar greenUB(GREEN_H_HI, GREEN_S_HI, GREEN_V_HI);
    cv::Scalar blueLB(BLUE_H_LO, BLUE_S_LO, BLUE_V_LO);
    cv::Scalar blueUB(BLUE_H_HI, BLUE_S_HI, BLUE_V_HI);
    cv::Scalar redLB(RED_H_LO, RED_S_LO, RED_V_LO);
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

    // Display the Masks
    string windowName[5] = {"Green Mask", "Blue Mask","Red Mask",
                            "Yellow Mask", "White Mask"};
    cv::Mat image[5] = {greenMask, blueMask, redMask, yellowMask, whiteMask};
    cv::Mat cumMask;
    for(int i = 0; i < 5; i++)
    {
        // LPF for Noise
        cv::blur(image[i], image[i], cv::Size(5,5));

        // Find Contours Around Blobs
        vector< vector<cv::Point> > contours;
        cv::findContours(image[i].clone(), contours, // hierarchy,
                        CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        // Draw Bounding Boxes Around LEGOs (Determined by Area)
        cv::Mat imageInBGR;
        cv::cvtColor(image[i], imageInBGR, CV_GRAY2BGR);
        for(int i = 0; i < contours.size(); i++)
        {
            if(cv::contourArea(contours[i]) < AREA_THRESHOLD)
                continue;
            cv::Rect r = cv::boundingRect(contours[i]);
            cv::rectangle(imageInBGR, r, cv::Scalar(0,255,0), 8);
        }

        // DEBUG
        cv::Mat resized;
        cv::resize(imageInBGR, resized, cv::Size(), .25, .25);
        cv::imshow(windowName[i] + "(blurred)", resized);

        // Add Found Pieces to Cumulative Mask
        cumMask = (i==0) ? imageInBGR : cumMask + imageInBGR;
    }

    // Show Cumulative Mask
    cv::Mat resized;
    cv::resize(cumMask, resized, cv::Size(), .25, .25);
    cv::imshow("Cumulative Mask", resized);

    // For Observation
    while(true)
        if(cv::waitKey(30) == 27)
            cout<<"\n\n\nUser held esc key to terminate program"<<endl; break;

    // Exit
    cout<< "\n\n\nHSVFinder Terminated on Success." << endl;
    return 0;
}
