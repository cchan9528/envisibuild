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
#define S_AREA_LB 10000        // Inherent Scale Variance
#define S_AREA_UB 40000
#define R_AREA_LB 40001
#define R_AREA_UB 100000
#define SHORT 150
#define LONG 300
#define DENSITY_THRESHOLD 0.45

#define WINDOW_SCALE .5
/******************************************************
 *
 * Project Instructions
 *
 ******************************************************/

// Striped Cube
lw::Instruction stripedcube_instr[6] = {
    { .op = PLC, .r1 = "BR", .r2 = "", .r3 = ""},
    { .op = PLC, .r1 = "BR", .r2 = "EE", .r3 = ""},
    { .op = STK, .r1 = "WR", .r2 = "NE", .r3 = "SE"},
    { .op = PLC, .r1 = "WR", .r2 = "WW", .r3 = ""},
    { .op = STK, .r1 = "RR", .r2 = "NW", .r3 = "SW"},
    { .op = PLC, .r1 = "RR", .r2 = "EE", .r3 = ""}
};
const lw::Project stripedcube_ref = {
    .materials[red]   = {.c = red  , .sCount=0, .rCount=2},
    .materials[white] = {.c = white, .sCount=0, .rCount=2},
    .materials[blue]  = {.c = blue , .sCount=0, .rCount=2},
    .numInstr = 6,
    .instr = stripedcube_instr,
};

// Staircase
lw::Instruction staircase_instr[4] = {
    { .op = PLC, .r1 = "RR", .r2 = "", .r3 = ""},
    { .op = PLC, .r1 = "RS", .r2 = "NN", .r3 = ""},
    { .op = STK, .r1 = "RR", .r2 = "NW", .r3 = "NE"},
    { .op = STK, .r1 = "RS", .r2 = "NW", .r3 = "SW"}
};
const lw::Project staircase_ref = {
    .materials[red] = {.c = red, .sCount=2, .rCount=2},
    .numInstr = 4,
    .instr = staircase_instr,
};

// Tower
lw::Instruction tower_instr[3] = {
    { .op = PLC, .r1 = "GS", .r2 = "", .r3 = ""},
    { .op = STK, .r1 = "GS", .r2 = "NW", .r3 = "NW"},
    { .op = STK, .r1 = "GS", .r2 = "NW", .r3 = "NW"}
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

    cv::Scalar drawColor(color_t color)
    {
        switch(color)
        {
            case red: return cv::Scalar(0,0,255);
            case yellow: return cv::Scalar(255,255,0);
            case green: return cv::Scalar(0,255,0);
            case blue: return cv::Scalar(255,0,0);
            case white: return cv::Scalar(255,255,255);
        }
    }

    string colorToStr(int c)
    {
        switch(c)
        {
            case red: return "red";
            case yellow: return "yellow";
            case green: return "green";
            case blue: return "blue";
            case white: return "white";
            default: return "";
        }
    }
    string colorToStr(color_t c)
    {
        switch(c)
        {
            case red: return "red";
            case yellow: return "yellow";
            case green: return "green";
            case blue: return "blue";
            case white: return "white";
            default: return "";
        }
    }
    string shapeToStr(shape_t s)
    {
        switch(s)
        {
            case square: return "square";
            case rect: return "rectangle";
            default: return "";
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

           if( (S_AREA_LB <= area && area <= S_AREA_UB) &&
               (SQ_SIDES_RATIO_LB <= wToh && wToh <= SQ_SIDES_RATIO_UB) &&
               (SQ_SIDES_RATIO_LB <= hTow && hTow <= SQ_SIDES_RATIO_UB))
               return square;
           else if( (R_AREA_LB <= area && area <= R_AREA_UB) )
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
            case green: lb = greenLB; ub = greenUB; break;
            case blue: lb = blueLB; ub = blueUB; break;
            case red: lb = redLB; ub = redUB; break;
            case yellow: lb = yellowLB; ub = yellowUB; break;
            case white: lb = whiteLB; ub = whiteUB; break;
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
            if(!(S_AREA_LB <= area) || (density < DENSITY_THRESHOLD))
                continue;

            // Update Color Tab
            shape_t s = findShape(rr, numVerts, area, mask.cols, mask.rows);
            if(s == square)
                (*tab).sCount++;
            else if(s == rect)
                (*tab).rCount++;
            else
                (*tab).uCount++;

            // DEBUG
            //cv::Scalar green(0,255,0);
            double radius = 25;
            int thickness = 10;
            cv::Point2f points[4]; rr.points( points );
            for( int k = 0; k < 4; k++ )
                line( maskInBGR, points[k], points[(k+1)%4], green, thickness);
            // cout<<"Vertex Coordinates for Contour "<< i <<endl;
            // for(int l = 0; l < verts.size(); l++)
            // {
            //     cv::circle(maskInBGR, verts[l], radius, green, thickness);
            //     cout<<verts[l].x<<", "<<verts[l].y<<endl;
            // }cout<<endl;
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
            int numS = t.sCount, numR = t.rCount, numU = t.uCount;
            string s;
            color_t c  = t.c;
            switch(c)
            {
                case green: s = "Green "; break;
                case blue: s = "Blue "; break;
                case red: s = "Red "; break;
                case yellow: s = "Yellow "; break;
                case white: s = "White "; break;
            }
            cout<<s<<"Tab: "\
                << numS << " squares, "\
                << numR << " rectangles, "\
                << numU << " unknowns" << endl;
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
            case stripedcube: projectRef = &stripedcube_ref; break;
            case staircase: projectRef = &staircase_ref; break;
            case tower: projectRef = &tower_ref; break;
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
            char numNeeded = projBlocks[color].sCount+projBlocks[color].rCount;

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

    void buildWorkspace(cv::Mat frame, Workspace * ws, project_t p)
    {
        cv::Mat mask(frame.rows, frame.cols, CV_8UC1, cv::Scalar(0));
        mask(cv::Rect(mask.cols/4, mask.rows/4, mask.cols/2, mask.rows/2))=255;
        ws->bounds = mask;
        ws->b_cc = cv::Point2f(mask.cols/2, mask.rows/2);
        ws->b_nw = cv::Point2f(mask.cols/4, mask.rows/4);
        ws->b_se = cv::Point2f(3*(mask.cols/4), 3*(mask.rows/4));
        ws->area = cv::countNonZero(mask);
        ws->p = p;
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
                case green: lb = greenLB; ub = greenUB; break;
                case blue: lb = blueLB; ub = blueUB; break;
                case red: lb = redLB; ub = redUB; break;
                case yellow: lb = yellowLB; ub = yellowUB; break;
                case white: lb = whiteLB; ub = whiteUB; break;
            }
            string color = colorToStr(c);
            cv::inRange(frameInHSV, lb, ub, mask);

            // Filter Noise
            cv::blur(mask, mask, cv::Size(5,5));
            cv::erode(mask, mask, getStructuringElement(cv::MORPH_RECT,
                                                        cv::Size(3,3)));

            cv::bitwise_and(mask, ws->bounds, res);
            cv::bitwise_xor(res, ws->bounds, res);

            // ~~~~~ DEBUG ~~~~~~
            cv::Mat resized;
            cv::resize(res, resized, cv::Size(), WINDOW_SCALE, WINDOW_SCALE);
            cv::imshow("XOR", resized);
            while(true)
                if(cv::waitKey(30) == 27)
                {    cout<<"ESC pressed"<<endl<<endl; break;}
            // ~~~ END DEBUG ~~~~
            if(cv::countNonZero(res) != ws->area)
            {
                cout<<endl<<endl<<"On "<<colorToStr(c)<<endl;
                cout<<"Please clear workspace!"<<endl<<endl;
                return false;
            }
        }
        return true;
    }

    void drawWorkspace(cv::Mat frame, Workspace * ws)
    {
        cv::rectangle(frame, ws->b_nw, ws->b_se, drawColor(green), 3);
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

    void getInstrMaterials(const Instruction * instr, Piece * p)
    {
        switch(instr->r1[0])
        {
            case 'R': p->c = red; break;
            case 'Y': p->c = yellow; break;
            case 'G': p->c = green; break;
            case 'B': p->c = blue; break;
            case 'W': p->c = white; break;

        }
        switch(instr->r1[1])
        {
            case 'R': p->s = rect; break;
            case 'S': p->s = square; break;
        }
    }

    void drawInstr(cv::Mat frame, Workspace * ws, const Instruction * instr)
    {
        if(!instr)
            cout<<"No instruction given"<<endl;
        else
        {
            // Current Register Information for Instruction
            Piece p; getInstrMaterials(instr, &p);
            const char * ra = instr->r2;
            const char * rb = instr->r3;
            // findMaterials(frame, p);

            // Current Target and Piece Dimensions (and Orientation)
            int w = ws->se.x - ws->nw.x;
            int h = (ws->nw.y > ws->se.y) ?
                ws->nw.y-ws->se.y : ws->se.y-ws->nw.y ;
            int pw = SHORT, ph = (p.s==rect) ? LONG : SHORT;

            // ~~~~~ DEBUG ~~~~~
            cout<<h<<" h:w "<<w<<endl;
            cout<<ph<<" ph:pw "<<pw<<endl;
            cout<<"Color: "<<colorToStr(p.c)<<" Shape: "<<shapeToStr(p.s)<<endl;
            cout<<instr->r2<<endl<<instr->r3<<endl;
            // ~~~ END DEBUG ~~~

            cv::Point p1, p2;
            if(instr->op==PLC)
            {
                if(instr->r2[0]==0) // Place at Center
                {
                    int wOff = SHORT/2;
                    int hOff = (p.s==rect) ? LONG/2 : SHORT/2;
                    if(ws->p == staircase){int t = wOff; wOff = hOff; hOff = t;}
                    p1.x = ws->b_cc.x - wOff; p1.y = ws->b_cc.y - hOff;
                    p2.x = ws->b_cc.x + wOff; p2.y = ws->b_cc.y + hOff;
                    if(wOff*2==LONG){p1.y+=(hOff*2);p2.y-=(hOff*2);}
                }
                else
                {
                    if(ra[0]==ra[1])
                    {
                        if(pw!=w){int t=pw; pw=ph; ph=t;}
                        cv::Point tl = ws->nw, br = ws->se;
                        char dir = ra[0];
                        if(h<w)
                        {
                            tl.y -= h; br.y += h;
                            switch(ra[0])
                            {
                                case 'N': dir='W'; break;
                                case 'S': dir='E'; break;
                                case 'E': dir='N'; break;
                                case 'W': dir='S'; break;
                            }
                        }
                        switch(dir)
                        {
                            case 'N':
                                p1.x = tl.x; p1.y = tl.y-ph;
                                p2.x = tl.x + pw; p2.y = tl.y;
                                break;
                            case 'S':
                                p1.x = br.x - pw; p1.y = br.y;
                                p2.x = br.x; p2.y = br.y + ph;
                                break;
                            case 'E':
                                p1.x = tl.x+w; p1.y = tl.y;
                                p2.x = br.x+pw; p2.y = br.y;
                                break;
                            case 'W':
                                p1.x = tl.x-pw; p1.y = tl.y;
                                p2.x = br.x - w; p2.y = br.y;
                                break;
                        }
                    }
                    else
                    {
                        // Place piece next to it so it doens't fill up side
                        // outside of project scope
                    }
                }
                cv::rectangle(frame, p1, p2, drawColor(p.c), 3);
                if(ph<pw){p1.y+=ph; p2.y-=ph;}
                ws->nw = p1; ws->se = p2;
            }
            else
            {
                if( ((ra=="NE"&&rb=="SW") || (rb=="NE"&&ra=="SW")) ||
                    ((ra=="NW"&&rb=="SE") || (rb=="NW"&&ra=="SE")) ||
                    (ra[0] == rb[0] && ra[1] == rb[1]))
                {   p1 = ws->nw; p2 = ws->se;}
                else
                {
                    if(ra[0]=='N')
                    {
                        if(h>=w)
                        {
                            p2.y=ws->nw.y+pw;
                            if(ra[1]=='W')
                            {
                                p1=ws->nw; p2.x = ws->nw.x+ph;
                                ws->nw.y += pw;
                                ws->se.x = ws->nw.x+ph; ws->se.y = ws->nw.y+pw;
                            }
                            else
                            {
                                p1.x=ws->nw.x+w; p1.y=ws->nw.y; p2.x = p1.x-ph;
                                ws->nw = p2; ws->se = p1;
                            }
                        }
                        else
                        {
                            p2.x=ws->nw.x+pw;
                            if(ra[1]=='W')
                            {
                                p1=ws->nw; p2.y = ws->nw.y-ph;
                                ws->se.y = ws->nw.y;
                                ws->nw.y -= ph;
                                ws->se.x = ws->nw.x+pw;
                            }
                            else
                            {
                                p1.x=ws->nw.x; p1.y=ws->nw.y-h; p2.y = p1.y+ph;
                                ws->nw = p1; ws->se = p2;
                            }
                        }
                    }
                    else
                    {
                        if(h>=w)
                        {
                            p2.y=ws->nw.y-pw;
                            if(ra[1]=='E')
                            {
                                p1=ws->se; p2.x = ws->se.x-ph;
                                ws->se.y -= pw;
                                ws->nw.x = ws->se.x-ph; ws->nw.y = ws->se.y;
                            }
                            else
                            {
                                p1.x=ws->se.x-w; p1.y=ws->se.y; p2.x = p1.x+ph;
                                ws->nw=p1; ws->se=p2;
                            }
                        }
                        else
                        {
                            p2.x=ws->se.x-pw;
                            if(ra[1]=='E')
                            {
                                p1=ws->se; p2.y = ws->se.y+ph;
                                ws->nw.y = ws->se.y;
                                ws->se.y += ph;
                                ws->nw.x = ws->se.x-pw;
                            }
                            else
                            {
                                p1.x=ws->se.x; p1.y=ws->se.y+h; p2.y = p1.y-ph;
                                ws->nw=p2; ws->se=p1;
                            }
                        }
                    }
                }
                cv::rectangle(frame, p1, p2, drawColor(p.c),3);
            }
        }
    }

    bool instrDone(cv::Mat frame, Workspace * ws, const Instruction * instr)
    {
        return false;
    }
}
