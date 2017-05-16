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

    string colorToStr(int c)
    {
        switch(c)
        {
            case red:
                return "red";
            case yellow:
                return "yellow";
            case green:
                return "green";
            case blue:
                return "blue";
            case white:
                return "white";
            default:
                return "";
        }
    }
    string colorToStr(color_t c)
    {
        switch(c)
        {
            case red:
                return "red";
            case yellow:
                return "yellow";
            case green:
                return "green";
            case blue:
                return "blue";
            case white:
                return "white";
            default:
                return "";
        }
    }
    string shapeToStr(shape_t s)
    {
        switch(s)
        {
            case square:
                return "square";
            case rect:
                return "rectangle";
            default:
                return "";
        }
    }

    shape_t findShape(cv::RotatedRect rr, int numVerts, double area,
                        int frW, int frH)
    {
        // Corners of the Rotated Rectangle
        cv::Point tl(rr.center.x-(rr.size.width/2),
                    rr.center.y-(rr.size.height/2));
        cv::Point tr(rr.center.x+(rr.size.width/2),
                    rr.center.y-(rr.size.height/2));
        cv::Point bl(rr.center.x-(rr.size.width/2),
                    rr.center.y+(rr.size.height/2));
        cv::Point br(rr.center.x+(rr.size.width/2),
                    rr.center.y+(rr.size.height/2));

        // Validate Location and Shape of Bounding Rectangle
        if(numVerts == 4 &&
           ((0<=tl.x && tl.x<=frW) && (0<=tl.y && tl.y<=frH)) &&
           ((0<=bl.x && tl.x<=frW) && (0<=bl.y && bl.y<=frH)) &&
           ((0<=tr.x && tr.x<=frW) && (0<=tr.y && tr.y<=frH)) &&
           ((0<=br.x && tr.x<=frW) && (0<=br.y && br.y<=frH)))
       {
           // Decide Shape
           float wToh = ((float)rr.size.width)/((float)rr.size.height);
           float hTow = 1/wToh;
           if( (LEGO_S_AREA_LB <= area && area <= LEGO_S_AREA_UB) &&
               (SQ_SIDES_RATIO_LB <= wToh && wToh <= SQ_SIDES_RATIO_UB) &&
               (SQ_SIDES_RATIO_LB <= hTow && hTow <= SQ_SIDES_RATIO_UB))
               return square;
           else if( (LEGO_R_AREA_LB <= area && area <= LEGO_R_AREA_UB) )
               return rect;
       }
       else
           return unkwn;

    // ~~~~~ DEBUG ~~~~~~
    // cout<<"Finding Shape"<<endl;
    // cout<<frW << " " << frH<<endl;
    // cout<< "tl: ("<< tl.x << ", " << tl.y << ")" << endl;
    // cout<< "tr: ("<< tr.x << ", " << tr.y << ")" << endl;
    // cout<< "bl: ("<< bl.x << ", " << bl.y << ")" << endl;
    // cout<< "br: ("<< br.x << ", " << br.y << ")" << endl;
    //
    // cout<<"wToh: "<<wToh<<endl;
    // cout<<"hToW: "<<hTow<<endl;
    // ~~~ END DEBUG ~~~
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
        color_t c = (*tab).c;
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
        string color = colorToStr(c);
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
            int numVerts = verts.size();

            // Ignore Noise
            if(!(LEGO_S_AREA_LB <= area) || (density < LEGO_DENSITY_THRESHOLD))
                continue;

            // Update Color Tab
            shape_t s = findShape(rr, numVerts, area, mask.cols, mask.rows);
            if(s == square)
            {
                cout<<"Square"<<endl;
                (*tab).sCount++;
            }
            else if(s == rect)
            {
                cout<<"Rect"<<endl;
                (*tab).rCount++;
            }
            else
            {
                cout<<"Unkwn"<<endl;
                (*tab).uCount++;
            }

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
                {    cout<<"ESC pressed"<<endl<<endl; break;}
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
            int numU = t.uCount;
            color_t c  = t.c;
            switch(c)
            {
                case green:
                    cout<<"Green Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles, "\
                        << numU << " unknowns" << endl;
                    break;
                case blue:
                    cout<<"Blue Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles, "\
                        << numU << " unknowns" << endl;
                    break;
                case red:
                    cout<<"Red Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles, "\
                        << numU << " unknowns" << endl;
                    break;
                case yellow:
                    cout<<"Yellow Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles, "\
                        << numU << " unknowns" << endl;
                    break;
                case white:
                    cout<<"White Tab: "\
                        << numS << " squares, "\
                        << numR << " rectangles, "\
                        << numU << " unknowns" << endl;
                    break;
                default:
                    return;
            }
        }
        cout<<"\n//////////////////  End Report  //////////////////\n"<<endl;
    }

    bool projectPossible(project_t project, cv::Mat frame,
                        Colortab * colortabs, int numcolors)
    {
        // Establish Project Reference
        const Project * projectRef;
        switch(project)
        {
            case stripedcube:
                projectRef = &stripedcube_ref; break;
            case staircase:
                projectRef = &staircase_ref; break;
            case tower:
                projectRef = &tower_ref; break;
            default:
                return false;
        }

        const Colortab * projBlocks = (*projectRef).materials;
        for(int color = red; color < numcolors; color++)
        {
            int numNeeded = projBlocks[color].sCount + projBlocks[color].rCount;

            // Nothing is Needed of This Color
            if( numNeeded == 0 )
                continue;
            else
            {
                // Update Color Tabs
                countPieces(frame, (colortabs+color));

                // Cannot Obtain Accurate Count; Assume Possible
                if(colortabs[color].uCount != 0)
                {
                    cout<<"***** Project may be possible; "\
                        << "please spread the pieces out for better evaluation"\
                        << "\n\n\nMaterials Report UNFINISHED \n" \
                        <<endl;
                    return true;
                }

                // Check Enough Squares Present
                if(colortabs[color].sCount != projBlocks[color].sCount)
                {
                    cout<<"Need more "<<colorToStr(color)<<"  squares"<< endl;
                    return false;
                }
                // Check Enough Rectangles Present
                else if(colortabs[color].rCount != projBlocks[color].rCount)
                {
                    cout<<"Need more "<<colorToStr(color)<<"  rects"<< endl;
                    return false;
                }
            }
        }
        cout<<"Project Possible"<<endl;
        return true;
    }
}
