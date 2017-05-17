#include "legoworld.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
/******************************************************************
 *
 * Descriptors
 *
 *******************************************************************/
#define RED_H_LO 0
#define RED_S_LO 160
#define RED_V_LO 135
#define RED_H_HI 10
#define RED_S_HI 255
#define RED_V_HI 255
#define BLUE_H_LO 70
#define BLUE_S_LO 0
#define BLUE_V_LO 0
#define BLUE_H_HI 180
#define BLUE_S_HI 255
#define BLUE_V_HI 255
#define GREEN_H_LO 30
#define GREEN_S_LO 100
#define GREEN_V_LO 80
#define GREEN_H_HI 70
#define GREEN_S_HI 255
#define GREEN_V_HI 255
#define WHITE_H_LO 10
#define WHITE_S_LO 100
#define WHITE_V_LO 190
#define WHITE_H_HI 180
#define WHITE_S_HI 170
#define WHITE_V_HI 255
#define YELLOW_H_LO 15
#define YELLOW_S_LO 200
#define YELLOW_V_LO 180
#define YELLOW_H_HI 45
#define YELLOW_S_HI 255
#define YELLOW_V_HI 255
#define SQ_SIDES_RATIO_LB 0.85
#define SQ_SIDES_RATIO_UB 1.15
#define LEGO_S_AREA_LB 10000        // Inherent Scale Variance
#define LEGO_S_AREA_UB 40000
#define LEGO_R_AREA_LB 40001
#define LEGO_R_AREA_UB 100000
#define LEGO_DENSITY_THRESHOLD 0.45

#define WINDOW_SCALE .5
/******************************************************
 *
 * Project Instructions
 *
 ******************************************************/

// Striped Cube
lw::Instruction stripedcube_instr[5] = {
    { .op = PLC, .r1 = "BR", .r2 = "BR", .r3 = "LL"},
    { .op = STK, .r1 = "NW", .r2 = "WR", .r3 = "NN"},
    { .op = STK, .r1 = "SW", .r2 = "WR", .r3 = "NN"},
    { .op = STK, .r1 = "NE", .r2 = "RR", .r3 = "NN"},
    { .op = STK, .r1 = "SE", .r2 = "RR", .r3 = "NN"}
};
const lw::Project stripedcube_ref = {
    .materials[red]   = {.c = red  , .sCount=0, .rCount=2},
    .materials[white] = {.c = white, .sCount=0, .rCount=2},
    .materials[blue]  = {.c = blue , .sCount=0, .rCount=2},
    .numInstr = 5,
    .instr = stripedcube_instr,
};

// Staircase
lw::Instruction staircase_instr[3] = {
    { .op = PLC, .r1 = "RR", .r2 = "RS", .r3 = "SS"},
    { .op = STK, .r1 = "NN", .r2 = "RR", .r3 = "NN"},
    { .op = STK, .r1 = "NN", .r2 = "RS", .r3 = "CC"}
};
const lw::Project staircase_ref = {
    .materials[red] = {.c = red, .sCount=2, .rCount=2},
    .numInstr = 3,
    .instr = staircase_instr,
};

// Tower
lw::Instruction tower_instr[3] = {
    { .op = PLC, .r1 = "GS", .r2 = "", .r3 = ""},
    { .op = STK, .r1 = "CC", .r2 = "GS", .r3 = "CC"},
    { .op = STK, .r1 = "CC", .r2 = "GS", .r3 = "CC"}
};
const lw::Project tower_ref = {
    .materials[green] = {.c = green, .sCount=3, .rCount=0},
    .numInstr = 3,
    .instr = tower_instr,
};


/******************************************************
 *
 * HSV Color Bounds
 *
 ******************************************************/

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

cv::Scalar Green(0,255,0);

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

/******************************************************
 *
 * Helper Functions
 *
 ******************************************************/

namespace lw{

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
    project_t strToProject(string s)
    {
        if(s=="stripedcube" || s == "cube")
            return stripedcube;
        else if(s=="staircase" || s=="stairs")
            return staircase;
        else if(s=="tower")
            return tower;
        else
            return none;
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

           // ~~~~~ DEBUG ~~~~~
           cout<<"wToh: "<<wToh<<endl;
           cout<<"hToW: "<<hTow<<endl;
           cout<<"area: "<<area<<endl;
           // ~~~ END DEBUG ~~~

           if( (LEGO_S_AREA_LB <= area && area <= LEGO_S_AREA_UB) &&
               (SQ_SIDES_RATIO_LB <= wToh && wToh <= SQ_SIDES_RATIO_UB) &&
               (SQ_SIDES_RATIO_LB <= hTow && hTow <= SQ_SIDES_RATIO_UB))
               return square;
           else if( (LEGO_R_AREA_LB <= area && area <= LEGO_R_AREA_UB) )
               return rect;
       }

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

    bool enoughMaterials(Colortab * colortab, const Colortab * projBlocks)
    {
        // Cannot Obtain Accurate Count; Assume Possible
        if((*colortab).uCount != 0)
        {
            cout<<"********************************************* "\
                << "\n\n\tProject may be possible\n"\
                << "  Will evaluate again once pieces in frame"\
                << "\n\n       Materials Report UNFINISHED \n\n" \
                <<"********************************************* "\
                <<endl;
            return true;
        }

        // Check Enough Squares Present
        if((*colortab).sCount != (*projBlocks).sCount)
        {
            cout<<"***************************************\n"<<endl;
            cout<<"\tNeed more "<<colorToStr((*colortab).c)<<" squares"<< endl;
            cout<<"\n***************************************"<<endl;
            return false;
        }
        // Check Enough Rectangles Present
        else if((*colortab).rCount != (*projBlocks).rCount)
        {
            cout<<"***************************************\n"<<endl;
            cout<<"\tNeed more "<<colorToStr((*colortab).c)<<" rects"<< endl;
            cout<<"\n***************************************"<<endl;
            return false;
        }

        // ~~~~~ DEBUG ~~~~~
        return true;
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

        (*tab).sCount = (*tab).rCount = (*tab).uCount = 0;

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
                // cout<<"Square"<<endl;
                (*tab).sCount++;
            }
            else if(s == rect)
            {
                // cout<<"Rect"<<endl;
                (*tab).rCount++;
            }
            else
            {
                // cout<<"Unkwn"<<endl;
                (*tab).uCount++;
            }

            // DEBUG
            //cv::Scalar green(0,255,0);
            double radius = 25;
            int thickness = 10;
            cv::Point2f points[4]; rr.points( points );
            for( int k = 0; k < 4; k++ )
                line( maskInBGR, points[k], points[(k+1)%4], green, thickness);
            // cout<<"Vertex Coordinates for Contour "<< i <<endl;
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
            cv::resize(maskInBGR, resized,cv::Size(),WINDOW_SCALE,WINDOW_SCALE);
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
        cout<<"\n\tReport UNFINISHED if unknown > 0"<<endl;
        cout<<"\t\tWill Reevaluate"<<endl;
        cout<<"\n -1 denotes not needed for project (not counted)"<<endl;
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
                cout<<"\n\nProject not available."<<endl;
                cout<<"Please choose an available project:\n"\
                    <<"    . stripedcube\n"\
                    <<"    . staircase\n"\
                    <<"    . tower\n"<<endl<<endl;
                return false;
        }

        const Colortab * projBlocks = (*projectRef).materials;
        for(int color = red; color < numcolors; color++)
        {
            char numNeeded = projBlocks[color].sCount + projBlocks[color].rCount;

            // Nothing is Needed of This Color
            if( numNeeded == 0 )
            {
                // ~~~~~ DEBUG ~~~~~
                colortabs[color].sCount = -1;
                colortabs[color].rCount = -1;
                colortabs[color].uCount = -1;
                // ~~~ END DEBUG ~~~
                continue;
            }
            else
            {
                // Update Color Tabs
                countPieces(frame, (colortabs+color));
                if(enoughMaterials((colortabs+color), (projBlocks+color)))
                    continue;
                else
                    return false;
            }
        }
        return true;
    }

    void buildWorkspace(cv::Mat frame, Workspace * ws)
    {
        cv::Mat mask(frame.rows, frame.cols, CV_8UC1, cv::Scalar(0));
        mask(cv::Rect(mask.cols/4, mask.rows/4, mask.cols/2, mask.rows/2))=255;
        ws->bounds = mask;
        ws->cc = cv::Point2f(mask.cols/2, mask.rows/2);
        ws->nw = cv::Point2f(mask.cols/4, mask.rows/4);
        ws->se = cv::Point2f(3*(mask.cols/4), 3*(mask.rows/4));
        ws->area = cv::countNonZero(mask);
    }

    bool clearWorkspace(cv::Mat frame, Workspace * ws)
    {
        cv::Mat frameInHSV, mask, res;
        cv::cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);
        cv::Scalar lb, ub;

        for(int c = red; c < NUM_COLORS; c++)
        {
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
            }
            string color = colorToStr(c);
            cv::inRange(frameInHSV, lb, ub, mask);

            // Filter Noise
            cv::blur(mask, mask, cv::Size(5,5));
            cv::erode(mask, mask, getStructuringElement(cv::MORPH_RECT,
                                                        cv::Size(3,3)));

            cv::bitwise_and(mask, ws->bounds, res);
            cv::bitwise_xor(res, ws->bounds, res);
            cv::Mat resized;
            cv::resize(res, resized, cv::Size(), WINDOW_SCALE, WINDOW_SCALE);
            cv::imshow("XOR", resized);
            while(true)
                if(cv::waitKey(30) == 27)
                {    cout<<"ESC pressed"<<endl<<endl; break;}
            if(cv::countNonZero(res) != ws->area)
            {
                cout<<endl<<endl<<"On "<<colorToStr(c)<<endl;
                cout<<"Please clear workspace!"<<endl<<endl;
                return false;
            }
        }
        return true;
    }

    void getInstrMaterials(const Instruction * instr, Piece * p1, Piece * p2)
    {
        int numpieces;
        if(instr->op == PLC)
            numpieces = 2;
        else
            numpieces = 1;

        for(int i = 0; i < numpieces; i++)
        {
            const char * r = (numpieces==1 || i==1) ? instr->r2 : instr->r1;
            Piece * p = (i==1) ? p2 : p1;
            cout<<r<<endl;
            switch(r[0])
            {
                case 'R':
                    p->c = red; break;
                case 'Y':
                    p->c = yellow; break;
                case 'G':
                    p->c = green; break;
                case 'B':
                    p->c = blue; break;
                case 'W':
                    p->c = white; break;

            }
            switch(r[1])
            {
                case 'R':
                    p->s = rect; break;
                case 'S':
                    p->s = square; break;
            }
        }
    }

    void drawWorkspace(cv::Mat frame, Workspace * ws)
    {
        cv::rectangle(frame, ws->nw, ws->se, Green, 3);
    }

    Instruction * getInstrStep(project_t projectName, int step)
    {
        if(projectName == stripedcube)
            return &(stripedcube_ref.instr[step]);
        else if(projectName == staircase)
            return &(staircase_ref.instr[step]);
        else
            return &(tower_ref.instr[step]);
    }

    void drawNextInstr(cv::Mat frame, Workspace * ws, const Instruction * instr)
    {
        if(!instr)
            cout<<"No instruction given"<<endl;
        else
        {
            Piece p1, p2; getInstrMaterials(instr, &p1, &p2);
            // ~~~~~ DEBUG ~~~~~
            cout<<"P1: " << colorToStr(p1.c)<<" "<<shapeToStr(p1.s)<<endl;
            if(p2.c)
                cout<<"P2: " << colorToStr(p2.c)<<" "<<shapeToStr(p2.s)<<endl;
            // ~~~ END DEBUG ~~~
            //drawTarget()
        }
    }
}
