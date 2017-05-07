//
//      Note: We want to control an acceptance interval, not a single value
//              Thus, we need two (value) trackbars for each category [H,S,V]
//
//      Note: OpenCV has HSV ranges of
//              H=[0,1,...,179]
//              S=[0,1,...,255]
//              V=[0,1,...,255]

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

/* #define RED_H_LO 0
#define RED_S_LO 76
#define RED_V_LO 129
#define RED_H_HI 6
#define RED_S_HI 255
#define RED_V_HI 255
#define BLUE_H_LO 73
#define BLUE_S_LO 42
#define BLUE_V_LO 122
#define BLUE_H_HI 126
#define BLUE_S_HI 168
#define BLUE_V_HI 214
#define GREEN_H_LO 54
#define GREEN_S_LO 25
#define GREEN_V_LO 0
#define GREEN_H_HI 80
#define GREEN_S_HI 211
#define GREEN_V_HI 197
#define WHITE_H_LO 11
#define WHITE_S_LO 0
#define WHITE_V_LO 221
#define WHITE_H_HI 23
#define WHITE_S_HI 33
#define WHITE_V_HI 255
#define YELLOW_H_LO 20
#define YELLOW_S_LO 115
#define YELLOW_V_LO 204
#define YELLOW_H_HI 28
#define YELLOW_S_HI 255
#define YELLOW_V_HI 255 */

using namespace std;

int main(int argc, char** argv)
{
    // Configure the Frame Source
    if(argc!=2)
    {
        cout << "\nUsage: HSVFinder filename" << endl;
        return -1;
    }

    // Set up Slider GUI
    int lowH = 0, highH = 0;
    int lowS = 0, highS = 0;
    int lowV = 0, highV = 0;
    cv::namedWindow("HSV Control", CV_WINDOW_NORMAL);
    cv::createTrackbar("[H]ue Lo", "HSV Control", &lowH, 179);
    cv::createTrackbar("[H]ue Hi", "HSV Control", &highH, 179);
    cv::createTrackbar("[S]at Lo", "HSV Control", &lowS, 255);
    cv::createTrackbar("[S]at Hi", "HSV Control", &highS, 255);
    cv::createTrackbar("[V]al Lo", "HSV Control", &lowV, 255);
    cv::createTrackbar("[V]al Hi", "HSV Control", &highV, 255);

    // Capture Frame from Source
    cv::Mat frame = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Note: BGR on Load
    if(!frame.data)
    {
        cout << "\n\n\nCould not read frame from source!!!" << endl;
        cout << "HSVFinder Terminated on Read Image Error." << endl;
        return -1;
    }
    cv::imshow("Original Image", frame);

    // HSV Finder
    cout<<"Processing...\n\n"<<endl;
    while(true)
    {
        // Convert Frame to HSV
        cv::Mat frameInHSV;
        cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);
        cout<<"Done converting to HSV."<<endl;

        // Threshold Frame According to What User Adjusts HSV Intervals to
        cout<<"low HSV: " << lowH << ", " << lowS <<", "<< lowV <<endl;
        cout<<"high HSV: " << highH << ", " << highS <<", "<< highV <<endl;
        cv::Mat mask;
        cv::Scalar lowerBound(lowH, lowS, lowV), upperBound(highH, highS, highV);
        cv::inRange(frameInHSV, lowerBound, upperBound, mask);
        cout<<"Done Thresholding."<<endl;

        // Display the (Threshold) Mask
        cv::imshow("Thresholded Mask", mask);
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
