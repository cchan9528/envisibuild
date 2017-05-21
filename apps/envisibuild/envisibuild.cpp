#include "envisibuild.h"
#include "legoworld.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#define FILL_DIFF 500
#define WINDOW_SCALE .5

using namespace std;
using namespace lw;

/******************************************************
 *
 * Project Instructions
 *
 ******************************************************/

// Striped Cube
Instruction stripedcube_instr[6] = {
    { .op = PLC, .r1 = "BR", .r2 = XX, .r3 = XX},
    { .op = PLC, .r1 = "BR", .r2 = EE, .r3 = XX},
    { .op = STK, .r1 = "WR", .r2 = NE, .r3 = SE},
    { .op = PLC, .r1 = "WR", .r2 = WW, .r3 = XX},
    { .op = STK, .r1 = "RR", .r2 = NW, .r3 = SW},
    { .op = PLC, .r1 = "RR", .r2 = EE, .r3 = XX}
};
const Project stripedcube_ref = {
    .materials[red]   = {.c = red  , .sCount=0, .rCount=2},
    .materials[white] = {.c = white, .sCount=0, .rCount=2},
    .materials[blue]  = {.c = blue , .sCount=0, .rCount=2},
    .numInstr = 6,
    .instr = stripedcube_instr,
};

// Staircase
Instruction staircase_instr[4] = {
    { .op = PLC, .r1 = "RR", .r2 = XX, .r3 = XX},
    { .op = PLC, .r1 = "WS", .r2 = NN, .r3 = XX},
    { .op = STK, .r1 = "RR", .r2 = NW, .r3 = NE},
    { .op = STK, .r1 = "YS", .r2 = NW, .r3 = SW}
};
extern const Project staircase_ref = {
    .materials[red] = {.c = red, .sCount=2, .rCount=2},
    .numInstr = 4,
    .instr = staircase_instr,
};

// Tower
Instruction tower_instr[3] = {
    { .op = PLC, .r1 = "BS", .r2 = XX, .r3 = XX},
    { .op = STK, .r1 = "GS", .r2 = NW, .r3 = NW},
    { .op = STK, .r1 = "RS", .r2 = NW, .r3 = NW}
};
extern const Project tower_ref = {
    .materials[green] = {.c = green, .sCount=3, .rCount=0},
    .numInstr = 3,
    .instr = tower_instr,
};

/******************************************************
 *
 * Helper Functions
 *
 ******************************************************/

project_t strToProj(string s)
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
    ws->step = 0;
}

bool clearWorkspace(cv::Mat frame, Workspace * ws)
{
    return false;
}

void drawWorkspace(cv::Mat frame, Workspace * ws)
{
    cv::putText(frame, "workspace", ws->b_nw,
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(153,255,153), 2);
    cv::rectangle(frame, ws->b_nw, ws->b_se, cv::Scalar(153,255,153), 2);
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
    {   cout<<"\n>>> On Step "<<ws->step<<"..."<<endl;
        // Current Register Information for Instruction
        Piece p; getInstrMaterials(instr, &p);
        dir_t ra = instr->r2;
        dir_t rb = instr->r3;
        // findMaterials(frame, p);

        // Current Target and Piece Dimensions (and Orientation)
        int w = ws->se.x - ws->nw.x;
        int h = (ws->nw.y > ws->se.y) ?
            ws->nw.y-ws->se.y : ws->se.y-ws->nw.y ;
        int pw = SHORT, ph = (p.s==rect) ? LONG : SHORT;

        // Handle Instruction; Result is a Drawn Box to Direct User
        cv::Point p1, p2;
        if(instr->op==PLC)
        {
            if(ra==XX) // Place at Center
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
                if(ra==NN||ra==SS||ra==EE||ra==WW)
                {
                    if(pw!=w){int t=pw; pw=ph; ph=t;}
                    cv::Point tl = ws->nw, br = ws->se;
                    dir_t dir = ra;
                    if(h<w)
                    {
                        tl.y -= h; br.y += h;
                        switch(ra)
                        {
                            case NN: dir=WW; break;
                            case SS: dir=EE; break;
                            case EE: dir=NN; break;
                            case WW: dir=SS; break;
                        }
                    }
                    switch(dir)
                    {
                        case NN:
                            p1.x = tl.x; p1.y = tl.y-ph;
                            p2.x = tl.x + pw; p2.y = tl.y;
                            break;
                        case SS:
                            p1.x = br.x - pw; p1.y = br.y;
                            p2.x = br.x; p2.y = br.y + ph;
                            break;
                        case EE:
                            p1.x = tl.x+w; p1.y = tl.y;
                            p2.x = br.x+pw; p2.y = br.y;
                            break;
                        case WW:
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
            if(ph<pw){p1.y+=ph; p2.y-=ph;}
            ws->nw = p1; ws->se = p2;
        }
        else
        {
            if( ((ra==NE&&rb==SW) || (rb==NE&&ra==SW)) ||
                ((ra==NW&&rb==SE) || (rb==NW&&ra==SE)) ||
                (ra==rb))
            {   p1 = ws->nw; p2 = ws->se;}
            else
            {
                if(ra==NW||ra==NE)
                {
                    if(h>=w)
                    {
                        p2.y=ws->nw.y+pw;
                        if(ra==NW)
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
                        if(ra==NW)
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
                        if(ra==SE)
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
                        if(ra==SE)
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
        }
        cv::Mat roi = frame(cv::Rect(p1,p2));
        cv::Mat shade(roi.size(), CV_8UC3, drawColor(p.c));
        cv::addWeighted(shade, .3, roi, .7, 0.0, roi);
        cv::rectangle(frame, p1, p2, drawColor(p.c), 2);
        ws->bounds(cv::Rect(p1,p2)) = cv::Scalar(0);
        ws->missing = p.c;
    }
}

bool instrDone(cv::Mat frame, Workspace * ws, const Instruction * instr)
{
    cv::Mat frameInHSV;
    cv::cvtColor(frame, frameInHSV, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::Scalar lb, ub;
    color_t c = ws->missing;
    lw::getHSVRange(c, &lb, &ub);
    string color = colorToStr(c);
    cv::inRange(frameInHSV, lb, ub, mask);

    // Filter Noise
    cv::blur(mask, mask, cv::Size(5,5));
    cv::erode(mask, mask, getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(3,3)));

    cv::Mat res, wsMask(mask.rows, mask.cols, CV_8UC1, cv::Scalar(0));
    wsMask(cv::Rect(ws->b_nw, ws->b_se)) = cv::Scalar(255);
    cv::bitwise_and(wsMask, mask, mask);
    cv::bitwise_xor(mask, ws->bounds, res);
    // ~~~~~ DEBUG ~~~~~
    cv::Mat resized;
    cv::resize(res, resized, cv::Size(), WINDOW_SCALE, WINDOW_SCALE);
    cv::imshow("workspace mask", resized);

    // THIS IS [1] - TURN THIS ON TO UNDERSTAND MASKING
    // NOTE: ENVISIBUILD WILL ONLY WORK IF [1] XOR [2]!!!!!

    // IMAGE
    // while(true)
    // if(cv::waitKey(30) == 27)
    // {    cout<<"ESC pressed"<<endl<<endl; break;}

    // VIDEO
    // if(cv::waitKey(1) == 27)
    // {    cout<<"ESC pressed"<<endl<<endl;}

    // ~~~ END DEBUG ~~~

    if(cv::countNonZero(res) >= ws->area-FILL_DIFF)
    {
        ws->bounds(cv::Rect(ws->b_nw,ws->b_se)) = cv::Scalar(255);
        return true;
    }
    // ~~~~~ DEBUG ~~~~~
    // THIS IS [2] - TURN THIS ON FOR SEQUENCING OF STEPS (IMAGE ONLY)
    // NOTE: ENVISIBUILD WILL ONLY WORK IF [1] XOR [2]!!!!!
    ws->bounds(cv::Rect(ws->b_nw,ws->b_se)) = cv::Scalar(255);
    return true;
    // ~~~ END DEBUG ~~~
    return false;
}
bool projectComplete(Workspace * ws)
{
    int numProjSteps = 0;
    switch(ws->p)
    {
        case stripedcube:
            numProjSteps = stripedcube_ref.numInstr; break;
        case staircase:
            numProjSteps = staircase_ref.numInstr; break;
        case tower:
            numProjSteps = tower_ref.numInstr; break;
    }
    return ws->step == numProjSteps;
}
