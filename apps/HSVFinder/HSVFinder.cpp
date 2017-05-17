//
//      Note: We want to control an acceptance interval, not a single value
//              Thus, we need two (value) trackbars for each category [H,S,V]
//
//      Note: OpenCV has HSV ranges of
//              H=[0,1,...,179]
//              S=[0,1,...,255]
//              V=[0,1,...,255]
//
//      Note: Color Inherently Depends on Exposure
//            Thus need to determine exposure for picture
//              or at least equalize histogram
//


#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

int lowH = 0, highH = 0;
int lowS = 0, highS = 0;
int lowV = 0, highV = 0;

void onChange(int, void*)
{
    cout<<"low HSV: " << lowH << ", " << lowS <<", "<< lowV <<endl;
    cout<<"high HSV: " << highH << ", " << highS <<", "<< highV <<endl;
}

int main(int argc, char** argv)
{
    // Configure the Frame Source
    if(argc!=2)
    {
        cout << "\nUsage: HSVFinder filename" << endl;
        return -1;
    }

    // Set up Slider GUI
    cv::namedWindow("HSV Control", CV_WINDOW_NORMAL);
    cv::createTrackbar("[H]ue Lo", "HSV Control", &lowH, 179, onChange);
    cv::createTrackbar("[H]ue Hi", "HSV Control", &highH, 179, onChange);
    cv::createTrackbar("[S]at Lo", "HSV Control", &lowS, 255, onChange);
    cv::createTrackbar("[S]at Hi", "HSV Control", &highS, 255, onChange);
    cv::createTrackbar("[V]al Lo", "HSV Control", &lowV, 255, onChange);
    cv::createTrackbar("[V]al Hi", "HSV Control", &highV, 255, onChange);

    // Capture Frame from Source
    cv::Mat frame = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Note: BGR on Load
    if(!frame.data)
    {
        cout << "\n\n\nCould not read frame from source!!!" << endl;
        cout << "HSVFinder Terminated on Read Image Error." << endl;
        return -1;
    }
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(), .5, .5);
    cv::imshow("Original Image", resized);

    // Convert Frame to HSV
    cv::Mat frameInHSV;
    cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);

    // HSV Finder
    cout<<"Processing...\n\n"<<endl;
    while(true)
    {
        // Threshold Frame According to What User Adjusts HSV Intervals to
        cv::Mat mask;
        cv::Scalar lowerBound(lowH, lowS, lowV), upperBound(highH, highS, highV);
        cv::inRange(frameInHSV, lowerBound, upperBound, mask);

        // Display the (Threshold) Mask
        cv::resize(mask, resized, cv::Size(), .5, .5);
        cv::imshow("Mask", resized);

        // Equalize Histogram
        cv::Mat eqMask;
        equalizeHist( mask, eqMask );
        cv::resize(eqMask, resized, cv::Size(), .5, .5);
        cv::imshow("Equalized Mask", resized);

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
