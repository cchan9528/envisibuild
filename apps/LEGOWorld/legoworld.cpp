#include "legoworld.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

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

namespace lw{
    using namespace std;

    shape_t findShape(cv::Rect boundingBox)
    {
        float wToh = ((float)boundingBox.width)/((float)boundingBox.height);
        float hTow = 1/wToh;
        if((SQ_SIDES_RATIO_LB <= wToh && wToh <= SQ_SIDES_RATIO_UB)
            && (SQ_SIDES_RATIO_LB <= hTow && hTow <= SQ_SIDES_RATIO_UB))
            return square;
        else
            return rect;
    }

    void countPieces(cv::Mat frame, lw::Colortab* tabs, int tabsSize)
    {
        projectPossible(none, frame, tabs, tabsSize);
    }

    void countPieces(cv::Mat frame, Colortab * tab)
    {
        // CANNY
        cv::Mat frameInGray,t;
        cv::cvtColor(frame, frameInGray, CV_BGR2GRAY);

        cv::equalizeHist(frameInGray,frameInGray);
        // cv::normalize(frameInGray, frameInGray);
        cv::resize(frameInGray, t, cv::Size(), .15, .15);
        cv::imshow("equalized", t);

        cv::blur(frameInGray, frameInGray, cv::Size(5,5));
        // cv::Mat cannyEdges;
        // double hysLo = 15, hysHi = hysLo * 3;
        // cv::Canny( frameInGray, cannyEdges, hysLo, hysHi );
        // cv::Canny( frame, cannyEdges, hysLo, hysHi );
        // int dxOrder = 1, dyOrder = 1;
        // cv::Sobel(frameInGray, frameInGray, -1, dxOrder, dyOrder);
        cv::Mat r1, r2, edgeMask=frameInGray.clone(), tmp;
        // cv::Sobel(frameInGray, r1, -1, 3, 0, 15, 10);
        // cv::Sobel(frameInGray, r2, -1, 0, 3, 15, 10);
        // edgeMask = r1 + r2;

        // cv::GaussianBlur(edgeMask, edgeMask, cv::Size(11,11), 200, 200);
        // cv::blur(edgeMask, edgeMask, cv::Size(201,201));
        // cv::erode(r,r, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3)));
        // cv::dilate(edgeMask,tmp,cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
        // cv::medianBlur(edgeMask, edgeMask, 31);
        // cv::blur(edgeMask, edgeMask, cv::Size(201,201));
        // edgeMask = tmp - edgeMask;
        // cv::Laplacian(frameInGray, t, CV_8U , 5);
        // edgeMask = edgeMask - t;

        cv::Mat dst = frame.clone();
        dst = cv::Scalar::all(0);
        frame.copyTo(dst,edgeMask);
        cv::Mat w;
        cv::resize(edgeMask, w, cv::Size(), .15, .15);
        // cv::resize(dst, w, cv::Size(), .15, .15);
        cv::imshow("Canny Edges", w);

        cv::Mat frameInHSV;
        cv::cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);
        // END CANNY

        color_t c = (*tab).c;

        // DEBUG
        string color;
        cv::Mat maskInBGR;
        // END DEBUG

        cv::Mat mask;
        cv::Scalar lb, ub;
        switch(c)
        {
            case green:
                lb = greenLB; ub = greenUB; color="Green"; break;
            case blue:
                lb = blueLB; ub = blueUB; color="Blue"; break;
            case red:
                lb = redLB; ub = redUB; color="Red"; break;
            case yellow:
                lb = yellowLB; ub = yellowUB; color="Yellow"; break;
            case white:
                lb = whiteLB; ub = whiteUB; color="White"; break;
            default:
                return;
        }
        cv::inRange(frameInHSV, lb, ub, mask);

        // Find LEGOs
        vector< vector<cv::Point> > contours;
        cv::findContours(mask.clone(), contours,
                        CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        //DEBUG
        if(contours.size()!=0)
            cv::cvtColor(mask, maskInBGR, CV_GRAY2BGR);
        // END DEBUG

        for(int i = 0; i < contours.size(); i++)
        {
            // Calculate (Avg) Density of White Pixels
            cv::Rect r = cv::boundingRect(contours[i]);
            cv::Mat patch; mask(r).copyTo(patch);
            float numWhites = (float) cv::countNonZero(patch);
            float area = r.width * r.height;
            float density = numWhites/area;

            // Ignore Noise
            if(cv::contourArea(contours[i]) < LEGO_AREA_THRESHOLD ||
                density < LEGO_DENSITY_THRESHOLD)
                continue;

            // Update Color Tab
            shape_t s = findShape(r);
            if(s == square)
                (*tab).sCount++;
            else
                (*tab).rCount++;

            //DEBUG
            cv::Scalar green(0,255,0);
            cv::rectangle(maskInBGR, r, green, 8);
        }
        // DEBUG
        if(contours.size()!=0)
        {
            cv::Mat resized;
            cv::resize(maskInBGR, resized, cv::Size(), .15, .15);
            cv::imshow(color + "(blurred)", resized);
            while(true)
                if(cv::waitKey(30) == 27)
                {    cout<<"\n\n\nUser held esc key to terminate program"<<endl; break;}
        }
        // END DEBUG
    }

    void materialsReport(Colortab * tabs, int tabsSize)
    {
        cout<<"\n///////////////  Materials Report  ///////////////\n"<<endl;
        for(int i = 0 ; i < tabsSize; i++)
        {
            Colortab t = *(tabs+i);
            int numS = t.sCount;
            int numR = t.rCount;
            color_t c  = t.c;
            switch(c)
            {
                case green:
                    cout<<"Green Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles" << endl;
                    break;
                case blue:
                    cout<<"Blue Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles" << endl;
                    break;
                case red:
                    cout<<"Red Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles" << endl;
                    break;
                case yellow:
                    cout<<"Yellow Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles" << endl;
                    break;
                case white:
                    cout<<"White Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles" << endl;
                    break;
                default:
                    return;
            }
        }
        cout<<"\n//////////////////  End Report  //////////////////\n"<<endl;
    }

    bool projectPossible(project_t project, cv::Mat frame,
                        Colortab * tabs, int tabsSize)
    {
        // Count the number of pieces
        for(int i = 0; i < tabsSize; i++)
            countPieces(frame, (tabs+i));

        // Compare with Needed Project Materials
        return false;
    }
}
