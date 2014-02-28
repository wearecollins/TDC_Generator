//
//  TDCConstants.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/21/14.
//
//

#pragma once

// this is dumb
struct QuickVertex
{
    ofVec2f pos;
    int     index;
    bool    bInterior;
};

enum GridType {
    GRID_POINTS = 0,
    GRID_OUTLINE,
    GRID_NEGATIVE
};

enum DrawMode {
    DRAW_NULL   = -1,
    DRAW_POINTS = 0,
    DRAW_LINES,
    DRAW_LINES_RANDOMIZED,
    DRAW_LINES_ARBITARY,
    DRAW_SHAPES
};

enum MovementType {
    MOVE_NONE = 0,
    MOVE_NOISE,
    MOVE_WARP,
    MOVE_FLOCK,
    MOVE_BUMP,
    MOVE_PUSH,
    MOVE_GRAVITY
};

static float dimensionsX = 400;
static float dimensionsY = 600;