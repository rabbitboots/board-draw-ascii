#ifndef BOARD_H
#define BOARD_H
	
#include "curses.h"

#include "error_handler.h"
#include "curses_wrapper.h"
#include "draw.h"


/*typedef struct Stringl_t {
	char * s;
	size_t len;
} Stringl;*/

typedef struct Coord_t {
	int x;
	int y;
} Coord;

typedef struct Cell_t {
	int pattern;
	int fg;
	int bg;
	int bright;
	int blink;
} Cell;

typedef struct Board_t {
	int w;
	int h;
	Cell * cells;
	bool color_enabled;

	char * filename;
} Board;

#define CELL_OUT_OF_BOUNDS 0
#define TEST_FILE "test_file.sav"

bool outOfBounds( int x, int y, int w, int h );
Cell boardGetCell( Board * board, int x, int y );
void boardPutCell( Board * board, Cell new_cell, int x, int y );
void boardWipe( Board * board, int wipe_pattern, int fg, int bg, int bright, int blink );
Board * boardInit( int w, int h, bool color );
void boardFree( Board * board );
void boardDraw( Board * board, Coord offset, bool draw_border );
bool sameCells( Cell a, Cell b );
void floodFill( Board * board, Cell first, Cell second, int x, int y );
bool boardSaveToFile( Board * brd, char * filename );
Board * boardLoadFromFile( char * filename );

Board * boardMakeFromSelection( Board * target, int tx, int ty, int tw, int th );
bool boardCopySection( Board * target, Board * dest, int tx, int ty, int tw, int th, int dx, int dy );

#endif
