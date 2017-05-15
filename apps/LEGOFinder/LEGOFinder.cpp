#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "legohsvcolors.hpp"

#define LEGO_AREA_THRESHOLD 20000        // Inherent Scale Variance
#define LEGO_DENSITY_THRESHOLD 0.40

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

    // Define HSV LEGO Color Bounds and Bounding Box Color
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
    cv::Scalar bbcolor(0,255,0);

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
        cv::medianBlur(image[i], image[i], 3);

        // Find Contours Around Blobs
        vector< vector<cv::Point> > contours;
        cv::findContours(image[i].clone(), contours, // hierarchy,
                        CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        cv::Mat imageInBGR;
        cv::cvtColor(image[i], imageInBGR, CV_GRAY2BGR);
        for(int j = 0; j < contours.size(); j++)
        {
            // Draw Bounding Boxes Around LEGOs (Bounding Box with Minimum Area)
            cv::RotatedRect rr = cv::minAreaRect(contours[j]);

            // Adjust Parameters of Bounding Box
            cv::Rect r = rr.boundingRect();
            r.x = r.x<0 ? 0 : r.x;
            r.y = r.y<0 ? 0 : r.y;
            r.width  = r.width+r.x>image[i].cols  ? image[i].cols-r.x:r.width;
            r.height = r.height+r.y>image[i].rows ? image[i].rows-r.y:r.height;

            // Calculate Area and Density of ROI for LEGO Decision
            cv::Mat roi; (image[i])(r).copyTo(roi);
            float numWhites = (float) cv::countNonZero(roi);
            float area = rr.size.width * rr.size.height;
            float density = numWhites/area;
            if(cv::contourArea(contours[j]) < LEGO_AREA_THRESHOLD ||
                density < LEGO_DENSITY_THRESHOLD)
                continue;

            // ~~~~~ DEBUG ~~~~~
            cout<<r.x<<" "<<r.y<<" "<<density<<endl;
            // ~~~ END DEBUG ~~~

            // Draw Rotated Rect
            cv::Point2f points[4]; rr.points( points );
            for( int k = 0; k < 4; k++ )
            {
                line( imageInBGR, points[k], points[(k+1)%4], bbcolor, 10);
                cout<<points[k].x<<", "<<points[k].y<<endl;
            } cout<<endl;

            // Draw Centers of Bounding Boxes Them
            // double radius = 10;
            // cv::circle(imageInBGR, rr.center, radius, bbcolor, 10);
        }

        // ~~~~~ DEBUG ~~~~~
        cv::Mat resized;
        cv::resize(imageInBGR, resized, cv::Size(), .15, .15);
        cv::imshow(windowName[i] + "(blurred)", resized);
        // ~~~ END DEBUG ~~~

        // Add Found Pieces to Cumulative Mask
        cumMask = (i==0) ? imageInBGR : cumMask + imageInBGR;
    }

    // Show Cumulative Mask
    cv::Mat resized;
    cv::resize(cumMask, resized, cv::Size(), .15, .15);
    cv::imshow("Cumulative Mask", resized);

    // For Observation
    while(true)
        if(cv::waitKey(30) == 27)
        {    cout<<"\n\n\nUser held esc key to terminate program"<<endl; break;}

    // Exit
    cout<< "\n\n\nLEGOFinder Terminated on Success." << endl;
    return 0;
}
