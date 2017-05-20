#ifndef ENVISIBUILD_H
#define ENVISIBUILD_H

#include "legoworld.h"
#include <opencv2/core.hpp>

/******************************************************************
 *
 * Descriptors
 *
 *******************************************************************/
#define IMAGE 1
#define VIDEO 2

/******************************************************
 *
 * Type Declarations
 *
 ******************************************************/

enum project_t { stripedcube, staircase, tower, none };

typedef struct Workspace{
    int step;
    project_t p;
    color_t missing;
    cv::Point b_cc, b_nw, b_se;
    cv::Point nw, se;
    cv::Mat bounds;
    int area;
} Workspace;

/******************************************************
 *
 * Helper Functions
 *
 ******************************************************/

project_t strToProj(std::string s);

// Workspace
void buildWorkspace(cv::Mat frame, Workspace * ws, project_t p);
void drawWorkspace(cv::Mat frame, Workspace * ws);
bool clearWorkspace(cv::Mat frame, Workspace * ws);
void drawInstr(cv::Mat frame, Workspace * ws,const lw::Instruction * instr);

// Project
bool projectComplete(Workspace * ws);
bool instrDone(cv::Mat frame, Workspace * ws,const lw::Instruction * instr);
lw::Instruction * getInstrStep(project_t projectName, int step);


#endif
