#ifndef LEGO_WORLD_H
#define LEGO_WORLD_H

#include <opencv2/core.hpp>

/******************************************************************
 *
 * Piece Descriptors
 *
 *******************************************************************/

#define NUM_COLORS 5
#define SQ_SIDES_RATIO_LB 0.85
#define SQ_SIDES_RATIO_UB 1.15
#define LEGO_S_AREA_LB 20000        // Inherent Scale Variance
#define LEGO_S_AREA_UB 500000
#define LEGO_R_AREA_LB 500001
#define LEGO_R_AREA_UB 1000000
#define LEGO_DENSITY_THRESHOLD 0.40

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

int findFiletype(char * s);

namespace lw {

    typedef struct Colortab{
        color_t c;
        int sCount;
        int rCount;
        int uCount;
    } Colortab;

    typedef struct Piece{
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

    /******************************************************
     *
     * Helper Functions
     *
     ******************************************************/
    project_t strToProject(std::string s);
    void countPieces(cv::Mat frameInHSV, Colortab* tab);
    void countPieces(cv::Mat frameInHSV, Colortab* tabs, int tabsSize);
    void materialsReport(lw::Colortab * tabs, int tabsSize);
    bool projectPossible(project_t project, cv::Mat frame,
                        Colortab* tabs, int tabsSize);

}
#endif
