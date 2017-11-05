#ifndef CURSES_WRAPPER_H
#define CURSES_WRAPPER_H

#define N_COLORS 8      // Curses defines eight colors from 0-7.
#define MONOCHROME 0    // Curses reserves 0 for monochrome text.

#include <stdio.h>
#include <stdlib.h>
#include "curses.h"
#include "error_handler.h"

// Global array used to reference console colors with PDCurses.
int col_map[N_COLORS][N_COLORS];

// -- Curses-specific initialization.

int curses_init_color_pairs(void);
int init_curses(void);

#endif // CURSES_WRAPPER_H
