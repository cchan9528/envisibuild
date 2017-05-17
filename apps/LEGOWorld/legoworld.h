#ifndef LEGO_WORLD_H
#define LEGO_WORLD_H

#include <opencv2/core.hpp>

/******************************************************************
 *
 * Descriptors
 *
 *******************************************************************/

#define IMAGE 1
#define VIDEO 2
#define NUM_COLORS 5

#define NUMSTEPS_STRIPEDCUBE 5
#define NUMSTEPS_STAIRCASE 3
#define NUMSTEPS_TOWER 3

/******************************************************
 *
 * Type Declarations
 *
 ******************************************************/

// enum bool { false, true };
enum color_t { red=0, blue=3, green=2, yellow=1, white=4 };

enum shape_t { square, rect, unkwn };

enum op_t { PLC, STK };

enum dir_t{ NN, NE, EE, SE, SS, SW, WW, NW };

enum project_t { stripedcube, staircase, tower, none };

int fileType(char * s);

namespace lw {

    typedef struct Colortab{
        color_t c;
        char sCount;
        char rCount;
        char uCount;
    } Colortab;

    typedef struct Piece{
        color_t c;
        shape_t s;
        cv::Point center, north;
    } Piece;

    typedef struct Building{
        cv::Point center, north;
        double area;
    } Building;

    typedef struct Instruction{
        op_t op;
        char r1[3];
        char r2[3];
        char r3[3];
    } Instruction;

    typedef struct Project{
        Colortab materials[5];
        int numInstr;
        Instruction * instr;
    } Project;

    typedef struct Workspace{
        cv::Point cc, nw, se;
        cv::Mat bounds;
        int area;
    } Workspace;

    /******************************************************
     *
     * Helper Functions
     *
     ******************************************************/

    /* Construction */
    project_t strToProject(std::string s);
    Instruction * getInstrStep(project_t projectName, int step);
    void countPieces(cv::Mat frameInHSV, Colortab* tab);
    void countPieces(cv::Mat frameInHSV, Colortab* tabs, int tabsSize);
    void materialsReport(lw::Colortab * tabs, int tabsSize);
    bool projectPossible(project_t project, cv::Mat frame,
                        Colortab* tabs, int tabsSize);

    /* Workspace */
    void buildWorkspace(cv::Mat frame, Workspace * ws);
    void drawWorkspace(cv::Mat frame, Workspace * ws);
    void drawNextInstr(cv::Mat frame, Workspace * ws,const Instruction * instr);
    bool clearWorkspace(cv::Mat frame, Workspace * ws);
}
#endif
