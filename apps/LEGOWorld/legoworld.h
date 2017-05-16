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
#define RED_H_HI 12
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
#define GREEN_V_LO 70
#define GREEN_H_HI 70
#define GREEN_S_HI 255
#define GREEN_V_HI 255
#define WHITE_H_LO 0
#define WHITE_S_LO 0
#define WHITE_V_LO 210
#define WHITE_H_HI 180
#define WHITE_S_HI 120
#define WHITE_V_HI 255
#define YELLOW_H_LO 15
#define YELLOW_S_LO 200
#define YELLOW_V_LO 180
#define YELLOW_H_HI 45
#define YELLOW_S_HI 255
#define YELLOW_V_HI 255
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

enum connection_t { ss, rr, rs, sr };

enum project_t { stripedcube, staircase, tower, none };

namespace lw {

    typedef struct Colortab{
        color_t c;
        int sCount;
        int rCount;
        int uCount;
    } Colortab;

    typedef struct Building{
        cv::Point loc;
    } Building;

    typedef struct Instruction{
        connection_t type;
    } Instruction;

    typedef struct Project{
        const Colortab materials[5];
        int numInstr;
        Instruction instr[];
    } Project;

    /******************************************************
     *
     * Project Instructions
     *
     ******************************************************/

    // Striped Cube
    const Project stripedcube_ref = {
        .materials[red]   = {.c = red  , .sCount=0, .rCount=2,},
        .materials[white] = {.c = white, .sCount=0, .rCount=2,},
        .materials[blue]  = {.c = blue , .sCount=0, .rCount=2,},
        // .numInstr = ;
        // .instr[0] = {
        //
        // }
    };

    // Staircase
    const Project staircase_ref = {

    };

    // Tower
    const Project tower_ref = {

    };

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
