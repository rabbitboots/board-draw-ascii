#ifndef DRAW_H
#define DRAW_H

#include "curses.h"
#include "curses_wrapper.h"

#include "error_handler.h"

#define RANGE_TYPE_SQUARE 1
#define RANGE_TYPE_CIRCLE 2

// The viewport is the window into the gameworld / map.
// If the map is larger than the viewport, scroll to follow the player or another point of interest.
// Viewport does not include the status bar or message log.
int VIEWPORT_W;
int VIEWPORT_H;

// Drawing offsets from 0,0 in the top-left corner.
int VIEWPORT_X;
int VIEWPORT_Y;

// Size of the console window. Traditionally this is typically 80x24 or 80x25 (MS-DOS).
// This could be made variable.
#define SCREEN_W 80
#define SCREEN_H 25

void drawGlyph( int glyph, int x, int y, int fg, int bg, int bright_fg, int bright_bg );
int colPair( int fg, int bg );
void colorSet( int fg, int bg, int fg_intensity, int bg_blink );

#endif // DRAW_H

