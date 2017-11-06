#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "curses.h"

#include "error_handler.h"
#include "curses_wrapper.h"
#include "draw.h"

typedef struct Board_t {
	int w;
	int h;
	int * data;
	bool color_enabled;
} Board;

typedef struct Coord_t {
	int x;
	int y;
} Coord;

bool outOfBounds( int x, int y, int w, int h ) {
	if( x < 0 || x > w - 1 || y < 0 || y > h - 1 ) {
		return true;
	}
	return false;
}

int boardGet( Board * board, int x, int y ) {
	if( outOfBounds( x, y, board->w, board->h ) ) {
		return 0;
	}
	else {
		return board->data[ (x * board->h) + y ];
	}
}

void boardPut( Board * board, int new_cell, int x, int y ) {
	if( !outOfBounds( x, y, board->w, board->h ) ) {
		board->data[ (x * board->h) + y ] = new_cell;
	}
}

void boardWipe( Board * board, int wipe_pattern, int fg, int bg, int bright, int blink ) {
	int x, y;
	for( x = 0; x < board->w; x++ ) {
		for( y = 0; y < board->h; y++ ) {
			boardPut( board, wipe_pattern, x, y );
		}
	}
}

Board * boardInit( int w, int h, bool color ) {
	if( w < 1 || h < 1 ) {
		// errLog boardInit(): invalid dimensions.
		return NULL;
		exit(1);
	}

	Board * new_board = malloc( sizeof(Board) );
	if( !new_board ) {
		// errLog malloc() failed on new_board
		return NULL;
	}

	new_board->w = w;
	new_board->h = h;
	new_board->color_enabled = color;

	new_board->data = malloc( (new_board->w * new_board->h) * sizeof(int) );
	if( !new_board->data ) {
		// errLog malloc() failed on data
		exit(1);
		return NULL;
	}
	
	boardWipe( new_board, ' ', COLOR_WHITE, COLOR_BLACK, 1, 0 );

	return new_board;
}

void boardFree( Board * board ) {
	if( board ) {
		free( board->data );
		free( board );
	}
}

void boardDraw( Board * board, Coord offset, bool draw_border ) {
	int x, y;
	int packed_cell = 'X';
	for( x = 0; x < board->w; x++ ) {
		for( y = 0; y < board->h; y++ ) {
			colorSet( COLOR_WHITE, COLOR_BLACK, 1, 0 );
			mvaddch( y + offset.y, x + offset.x, boardGet( board, x, y ) );
		}
	}
	if( draw_border ) {
		colorSet( COLOR_BLACK, COLOR_BLACK, 1, 0 );
		for( x = 0; x < board->w; x++ ) {
			//top
			mvaddch( offset.y - 1,        x + offset.x, '-' );
			//bottom
			mvaddch( offset.y + board->h, x + offset.x, '-' );
		}
		for( y = 0; y < board->h; y++ ) {
			//left
			mvaddch( y + offset.y, offset.x - 1,           '|' );
			//right
			mvaddch( y + offset.y, offset.x + board->w,    '|' );
		}
		//corners
		mvaddch( offset.y - 1,        offset.x - 1, '+' );
		mvaddch( offset.y + board->h, offset.x - 1, '+' );
		mvaddch( offset.y + board->h, offset.x + board->w, '+' );
		mvaddch( offset.y - 1,        offset.x + board->w, '+' );
	}
}


int main( int argc, char * argv[] ) {

	// Seed randomizer.
	srand(time(NULL));

	Coord cursor;
	cursor.x = 5;
	cursor.y = 5;
	curs_set(2);

	   // Debug logging
    errorHandlerInit( &error_handler, 0 );
    errLog( "    ** Logging new session **");


	if( init_curses() != 0 ) {
		// errLog init_curses() failed.
		return 1;
	}

	Board * my_board = boardInit( 74, 20, true );
	if( !my_board ) {
		exit(1);
	}

	int input = 0;
	bool doodle_mode = false;
	bool first_tick = true;
	bool keep_go = true;
	int cstep_x = 1;
	int cstep_y = 1;
	int cstep_count = 0;
	int c_fg = COLOR_WHITE;
	int c_bg = 0;
	int c_bright = 1;
	int c_blink = 0;
	Coord offset = {1, 1} ;

	while( keep_go ) {
		if( !first_tick ) {
			input = getch();
		}
		if( input == 'q') {
			keep_go = false;
		}
		if( input == 'r') {
			boardPut( my_board, 32 + (rand() % 128), rand() % my_board->w, rand() % my_board->h );
		}
		if( input == '\t' ) {
			doodle_mode = !doodle_mode;
		}
		if( input == 'c' ) {
			c_fg++;
			if( c_fg > N_COLORS - 1 ) {
				c_fg = 0;
			}
		}
		if( input == 'v' ) {
			c_bg++;
			if( c_bg > N_COLORS - 1 ) {
				c_bg = 0;
			}
		}
		if( input == 'd' ) {
			c_bright = !c_bright;
		}
		if( input == 'f' ) {
			c_blink = !c_blink;
		}

		if( input == '-' ) {
			if( cstep_x > 1 ) {
				cstep_x--;
			}
		}
		if( input == '=' ) {
			if( cstep_x < my_board->w - 1 ) {
				cstep_x++;
			}
		}

		if( input == '_' ) {
			if( cstep_y > 1 ) {
				cstep_y--;
			}
		}
		if( input == '+' ) {
			if( cstep_y < my_board->h - 1 ) {
				cstep_y++;
			}
		}

		if( input == ' ' ) {
			boardPut( my_board, '#', cursor.x, cursor.y );
		}
		if( input == KEY_DC ) {
			boardPut( my_board, ' ', cursor.x, cursor.y );
		}

		if( input == KEY_LEFT ) {
			for( cstep_count = 0; cstep_count < cstep_x; cstep_count++ ) {
				if( cursor.x > 0 )  {
					cursor.x--;
				}
			}
		}
		if( input == KEY_RIGHT ) {
			for( cstep_count = 0; cstep_count < cstep_x; cstep_count++ ) {
				if( cursor.x < my_board->w - 1 ) {
					cursor.x++;
				}
			}
		}
		if( input == KEY_UP ) {
			for( cstep_count = 0; cstep_count < cstep_y; cstep_count++ ) {
				if( cursor.y > 0 )  {
					cursor.y--;
				}
			}
		}
		if( input == KEY_DOWN ) {
			for( cstep_count = 0; cstep_count < cstep_y; cstep_count++ ) {
				if( cursor.y < my_board->h - 1) {
					cursor.y++;
				}
			}
		}
		
		if( doodle_mode ) {
			boardPut( my_board, '#', cursor.x, cursor.y );
		}

		clear();
		boardDraw( my_board, offset, true );

		//mvaddch( cursor.y, cursor.x, 'X' );
		if( !doodle_mode ) {
			curs_set( 1 ); // Hmm, this doesn't seem to show a different cursor under my current gnome-terminal.
			mvprintw( 22, 0, "                   " );
		}
		else {
			curs_set( 2 );
			mvprintw( 22, 0,"Doodle Mode Engaged", doodle_mode );
		}
		mvprintw( 23, 0, "X %d Y %d W %d H %d XStep %d YStep %d c_fg %d c_bg %d c_bright %d c_blink %d", 
		cursor.x, cursor.y, my_board->w, my_board->h, cstep_x, cstep_y, c_fg, c_bg, c_bright, c_blink );

		//testing color configuration vars
		//colorSet( COLOR_WHITE, COLOR_BLACK, 1, 0 );
		curs_set( 0 );
		colorSet( c_fg, c_bg, c_bright, c_blink );
		mvaddch( cursor.y + offset.y, cursor.x + offset.x, '+' );
		move( cursor.y + offset.y, cursor.x + offset.x );
		refresh();
		first_tick = false;
	}

	if( my_board ) {
		boardFree( my_board );
	}

/* Close error handler */
    errLog("    **  Shutting down.  **\n");
    errorHandlerShutdown( &error_handler );
	
	endwin();
	return 0;
}
