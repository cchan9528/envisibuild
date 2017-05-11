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
        cv::Mat frameInHSV;
        cv::cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);

        color_t c = (*tab).c;

        cv::Mat mask;
        cv::Scalar lb, ub;
        switch(c)
        {
            case green:
                lb = greenLB; ub = greenUB; break;
            case blue:
                lb = blueLB; ub = blueUB; break;
            case red:
                lb = redLB; ub = redUB; break;
            case yellow:
                lb = yellowLB; ub = yellowUB; break;
            case white:
                lb = whiteLB; ub = whiteUB; break;
            default:
                return;
        }
        cv::inRange(frameInHSV, lb, ub, mask);

        // Find LEGOs
        vector< vector<cv::Point> > contours;
        cv::findContours(mask.clone(), contours,
                        CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        for(int i = 0; i < contours.size(); i++)
        {
            if(cv::contourArea(contours[i]) < LEGO_AREA_THRESHOLD)
                continue;
            cv::Rect r = cv::boundingRect(contours[i]);
            shape_t s = findShape(r);
            if(s == square)
                (*tab).sCount++;
            else
                (*tab).rCount++;
        }

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
