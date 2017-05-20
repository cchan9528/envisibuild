#ifndef LEGO_WORLD_H
#define LEGO_WORLD_H

#include <opencv2/core.hpp>

/******************************************************************
 *
 * Descriptors
 *
 *******************************************************************/
#define NUM_COLORS 5
#define SHORT 150
#define LONG 300

/******************************************************
 *
 * Type Declarations
 *
 ******************************************************/
enum color_t { red=0, blue=3, green=2, yellow=1, white=4, clear };
enum shape_t { square, rect, unkwn };
enum op_t { PLC, STK };
enum dir_t{ NN, NE, EE, SE, SS, SW, WW, NW, XX };

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
        cv::Point cc;
    } Piece;

    typedef struct Building{
        cv::Point center, north;
        double area;
    } Building;

    typedef struct Instruction{
        op_t op;
        char r1[3];
        dir_t r2;
        dir_t r3;
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

    /* Piece Details */
    cv::Scalar drawColor(color_t color);
    std::string colorToStr(int c);
    std::string shapeToStr(shape_t s);

    /* Construction */
    void getHSVRange(color_t c, cv::Scalar * lb, cv::Scalar * ub);
    // void countPieces(cv::Mat frameInHSV, Colortab* tab);
    // void countPieces(cv::Mat frameInHSV, Colortab* tabs, int tabsSize);
    // void materialsReport(lw::Colortab * tabs, int tabsSize);
    // bool projectPossible(project_t project, cv::Mat frame,
    //                     Colortab* tabs, int tabsSize);


}
#endif
