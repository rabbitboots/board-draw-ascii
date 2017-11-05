#include "curses_wrapper.h"

int curses_init_color_pairs() {
    // Curses treats colors as indexed BG + FG pairs.  To reference BG and
    // FG colors independently, we can populate a global array with all
    // color combos that can be referenced in a secondary function.

    int i, bg, fg;
    for(i = 0; i < N_COLORS*N_COLORS; i++) {
        // Note: color pair 0 is reserved by Curses for monochrome.
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

    // Start Curses
    if( initscr() == NULL ) {
        errLog( "init_curses(): initscr() failed." );
        return ERR;
    }

    // Enable colors, if supported
    if( !has_colors() ) {
        errLog( "init_curses(): Note: Curses is reporting that color is not supported by this terminal." );
    }
    else if( start_color() == ERR ) {
        errLog( "init_curses(): start_color() failed.");
        return ERR;
    }

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
                        // Options are: 0 = disable, 1 = visible, 2 = very visible.
						// Actual display varies by terminal / console.

    return 0;
}

