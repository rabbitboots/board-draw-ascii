#include "curses_wrapper.h"

int curses_init_color_pairs() {
    // Curses treats colors as indexed BG + FG pairs.  To reference BG and
    // FG colors independently, we can populate a global array with all
    // color combos that can be referenced in a secondary function.

    int i, bg, fg;
    for(i = 0; i < N_COLORS*N_COLORS; i++) {
        // Note: 0 is reserved by Curses for monochrome.
        fg = i % N_COLORS;
        bg = i / N_COLORS;

        if( init_pair(i+1, fg, bg ) == ERR )  {
            errLog( "curses_init_color_pairs(): init_pair() failed." );
            return ERR;
        }
        if( has_colors() ) {
            col_map[fg][bg] = i+1;
        }
        else {
            // If terminal doesn't support color, map every color to the monochrome pair
            col_map[fg][bg] = 0;
        }
    }
    return 0;
}

// Curses startup wrapper.
int init_curses(void) {

    //#define CURSES_WIDTH 80
    //#define CURSES_HEIGHT 50
    //#define ENV_SZ 32
    //char widthEnv[ENV_SZ];
    //char heightEnv[ENV_SZ];

    //snprintf( "pdc_cols=%d", ENV_SZ, CURSES_WIDTH );
    //snprintf( "pdc_lines=%d", ENV_SZ, CURSES_HEIGHT );

    // Start Curses
    if( initscr() == NULL ) {
        errLog( "init_curses(): initscr() failed." );
        return ERR;
    }

    // Enable colors.
    if( !has_colors() ) {
        errLog( "init_curses(): Note: Curses is reporting that color is not supported by this terminal." );
    }
    else if( start_color() == ERR ) {
        errLog( "init_curses(): start_color() failed.");
        return ERR;
    }

    /*
    Curses specifies the following functions for altering colors,
    but I can't get them to work at the moment. I'll leave them
    alone for now.

    can_change_color() // This returns true in Windows -- maybe it just means the *user* can change colors?
    init_color(COLOR_GREEN, 1000,1000,1000);
    color_content(short, *short, *short, *short);
    */

    // Initialize color pairs
    if( curses_init_color_pairs() == ERR ) {
        errLog( "curses_init(): curses_init_color_pairs() reported failure." );
        return ERR;
    }

    raw();      // Disable line buffering and control characters (CTRL+C / CTRL+Z)
                // cbreak() can be used instead, if control characters are desired.
    noecho();   // Do not show characters typed by user into terminal
    keypad(stdscr, TRUE);   // enable input from function keys and arrow keys.

    curs_set(0);        // Turn off the blinking cursor for now.
                        // 0 = disable, 1 = blinking line, 2 = blinking square.

    return 0;
}


void colorTestPattern(void) {
    int x, y, f, g;
    for(x = 0; x < 8; x++ ) {
        for( y = 0; y < 8; y++ ) {
            for( f = 0; f < 2; f++ ) {
                for( g = 0; g < 2; g++ ) {
		    // TODO disabling this while trying to get the code to compile under linux gcc.
                    //colorSet( x, y, f, g );
                    //color_set( x, y, f, g );
                    mvaddch(y+(f*8),x+(g*8), '#');
                }
            }
        }
    }
}

void charTest(void) {
    int i;
    for (i=0; i<2000; i++) {
    mvaddch(i / 80,i % 80, i + 8000);
    }
}
