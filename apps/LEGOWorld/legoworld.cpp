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

        cv::Mat mask;
        cv::Scalar lb, ub;
        string color;
        color_t c = (*tab).c;
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

        // Filter Noise
        cv::blur(mask, mask, cv::Size(5,5));
        cv::erode(mask, mask, getStructuringElement(cv::MORPH_RECT,
                                                    cv::Size(3,3)));

        // DEBUG
        cv::Mat maskInBGR;
        cv::cvtColor(mask, maskInBGR, CV_GRAY2BGR);
        // END DEBUG

        // Find LEGOs
        vector< vector<cv::Point> > contours;
        cv::findContours(mask.clone(), contours,
                        CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        for(int i = 0; i < contours.size(); i++)
        {
            // Draw Bounding Boxes Around LEGOs (Bounding Box with Minimum Area)
            cv::RotatedRect rr = cv::minAreaRect(contours[i]);

            // Extract ROI
            cv::Rect r = rr.boundingRect();
            r.x = r.x<0 ? 0 : r.x;
            r.y = r.y<0 ? 0 : r.y;
            r.width  = r.width+r.x>mask.cols  ? mask.cols-r.x:r.width;
            r.height = r.height+r.y>mask.rows ? mask.rows-r.y:r.height;
            cv::Mat roi; (mask)(r).copyTo(roi);

            // Calculate Area and (Average) Density of ROI
            double numWhites = (double) cv::countNonZero(roi);
            double area = contourArea(contours[i]);
            double density = numWhites/area;

            // Find Verticies of Polygonal Approximation of Contour
            vector<cv::Point> verts;
            double deviation = .02 * cv::arcLength(contours[i], true);
            cv::approxPolyDP(contours[i], verts, deviation, true);

            // Ignore Noise
            if( (!(LEGO_S_AREA_LB <= area && area <= LEGO_S_AREA_UB) &&
                !(LEGO_R_AREA_LB <= area && area <= LEGO_R_AREA_UB)) ||
                density < LEGO_DENSITY_THRESHOLD )
                continue;

            // Update Color Tab
            shape_t s = findShape(r);
            if(s == square)
                (*tab).sCount++;
            else
                (*tab).rCount++;

            // DEBUG
            cv::Scalar green(0,255,0);
            double radius = 25;
            int thickness = 10;
            cv::Point2f points[4]; rr.points( points );
            for( int k = 0; k < 4; k++ )
                line( maskInBGR, points[k], points[(k+1)%4], green, thickness);
            cout<<"Vertex Coordinates for Contour "<< i <<endl;
            for(int l = 0; l < verts.size(); l++)
            {
                cv::circle(maskInBGR, verts[l], radius, green, thickness);
                cout<<verts[l].x<<", "<<verts[l].y<<endl;
            }cout<<endl;
            // END DEBUG
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
