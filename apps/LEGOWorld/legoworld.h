#ifndef LEGO_WORLD_H
#define LEGO_WORLD_H

#include <opencv2/core.hpp>

/******************************************************************
 *
 * Piece Descriptors
 *
 *******************************************************************/

#define NUM_COLORS 5
#define RED_H_LO 0
#define RED_S_LO 138
#define RED_V_LO 0
#define RED_H_HI 16
#define RED_S_HI 255
#define RED_V_HI 255
#define BLUE_H_LO 66
#define BLUE_S_LO 0
#define BLUE_V_LO 115
#define BLUE_H_HI 179
#define BLUE_S_HI 255
#define BLUE_V_HI 255
#define GREEN_H_LO 31
#define GREEN_S_LO 95
#define GREEN_V_LO 95
#define GREEN_H_HI 66
#define GREEN_S_HI 255
#define GREEN_V_HI 255
#define WHITE_H_LO 0
#define WHITE_S_LO 0
#define WHITE_V_LO 255
#define WHITE_H_HI 179
#define WHITE_S_HI 112
#define WHITE_V_HI 255
#define YELLOW_H_LO 16
#define YELLOW_S_LO 150
#define YELLOW_V_LO 150
#define YELLOW_H_HI 28
#define YELLOW_S_HI 255
#define YELLOW_V_HI 255
#define SQ_SIDES_RATIO_UB 1.2
#define SQ_SIDES_RATIO_LB 0.8
#define LEGO_AREA_THRESHOLD 5000        // Inherent Scale Variance

    /******************************************************
     *
     * Type Declarations
     *
     ******************************************************/

// enum bool { false, true };
enum color_t { red, blue, green, yellow, white };

enum shape_t { square, rect };

enum connection_t { ss, rr, rs, sr };

enum project_t { stripedcube, staircase, tower, none };

namespace lw {

    typedef struct Colortab{
        color_t c;
        int sCount;
        int rCount;
    } Colortab;

    typedef struct Building{
        cv::Point loc;
    } Building;

    typedef struct Instruction{
        connection_t type;
    } Instruction;

    typedef struct Project{
        project_t name;
        Instruction instr[];
    } Project;

    /******************************************************
     *
     * Project Instructions
     *
     ******************************************************/

    // Striped Cube

    // Staircase

    // Tower

    /******************************************************
     *
     * Helper Functions
     *
     ******************************************************/

    void countPieces(cv::Mat frameInHSV, Colortab* tab);
    void countPieces(cv::Mat frameInHSV, Colortab* tabs, int tabsSize);
    void materialsReport(lw::Colortab * tabs, int tabsSize);
    bool projectPossible(project_t project, cv::Mat frame,
                        Colortab* tabs, int tabsSize);

}
#endif
