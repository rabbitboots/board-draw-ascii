#include "draw.h"

// Wrap curses draw-character function to include attributes.
void drawGlyph( int glyph, int x, int y, int fg, int bg, int bright_fg, int bright_bg ) {

    colorSet( fg, bg, bright_fg, bright_bg );

    mvaddch( y, x, glyph);

    return;
}

int colPair(int fg, int bg) {
    // if out of bounds, report as an error and use the monochrome pair (0).
    if(fg < 0 || fg > N_COLORS || bg < 0 || bg > N_COLORS ) {
        errLog("colPair(): Was asked to return a pair that is out of bounds. Requested: fg %d, bg %d. Returning monochrome (pair 0) instead.", fg, bg );
        return 0;
    }
    return col_map[fg][bg];
}

void colorSet( int fg, int bg, int fg_intensity, int bg_blink ) {
    if( fg_intensity ) {
        attron( A_BOLD );
    }
    else {
        attroff( A_BOLD );
    }

    if( bg_blink ) {
        attron( A_BLINK );
    }
    else {
        attroff( A_BLINK );
    }
    attron( COLOR_PAIR( colPair( fg, bg ) ) );
}
