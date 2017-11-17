#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

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

bool outOfBounds( int x, int y, int w, int h ) {
	return ( x < 0 || x > w - 1 || y < 0 || y > h - 1 );
}

#define CELL_OUT_OF_BOUNDS 0

Cell boardGetCell( Board * board, int x, int y ) {
	if( outOfBounds( x, y, board->w, board->h ) ) {
		Cell oob;
		oob.pattern = CELL_OUT_OF_BOUNDS;
		return oob;
	}
	else {
		return board->cells[ x*board->h + y ];
	}
}

void boardPutCell( Board * board, Cell new_cell, int x, int y ) {
	if( !outOfBounds( x, y, board->w, board->h ) ) {
		board->cells[ x*board->h + y ] = new_cell;
	}
}

void boardWipe( Board * board, int wipe_pattern, int fg, int bg, int bright, int blink ) {
	int x, y;
	for( x = 0; x < board->w; x++ ) {
		for( y = 0; y < board->h; y++ ) {
			Cell empty;
			empty.pattern = wipe_pattern;
			empty.fg = fg;
			empty.bg = bg;
			empty.bright = bright;
			empty.blink = blink;

			boardPutCell( board, empty, x, y );
		}
	}
}

#define TEST_FILE "test_file.sav"

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

	new_board->filename = malloc( sizeof(TEST_FILE) );
	if( !new_board->filename ) {
		errLog( "malloc() failed on new_board->filename" );
		return NULL;
	}
	strncpy( new_board->filename, TEST_FILE, sizeof( TEST_FILE ) );

	new_board->cells = malloc( (new_board->w * new_board->h) * sizeof(Cell) );
	if( !new_board->cells ) {
		// errLog malloc() failed on cells
		exit(1);
		return NULL;
	}

	
	boardWipe( new_board, ' ', COLOR_WHITE, COLOR_BLACK, 1, 0 );

	return new_board;
}

void boardFree( Board * board ) {
	if( board ) {
		free( board->cells );
		free( board->filename );
		free( board );
	}
}

void boardDraw( Board * board, Coord offset, bool draw_border ) {
	int x, y;
	Cell current;
	for( x = 0; x < board->w; x++ ) {
		for( y = 0; y < board->h; y++ ) {
			current = boardGetCell( board, x, y );
			colorSet( current.fg, current.bg, current.bright, current.blink );
			mvaddch( y + offset.y, x + offset.x, current.pattern );
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

bool sameCells( Cell a, Cell b ) {
	bool retval = false;

	if( a.pattern == b.pattern
		&& a.fg == b.fg
		&& a.bg == b.bg
		&& a.bright == b.bright
		&& a.blink == b.blink ) {
			retval = true;
	}

	return retval;
}

void floodFill( Board * board, Cell first, Cell second, int x, int y ) {
	if( !outOfBounds( x, y, board->w, board->h ) ) {
	
		Cell check = boardGetCell( board, x, y );

		if( sameCells( check, first ) && !sameCells( first, second ) ) {
			boardPutCell( board, second, x, y );

			floodFill( board, first, second, x - 1, y );
			floodFill( board, first, second, x + 1, y );
			floodFill( board, first, second, x, y - 1 );
			floodFill( board, first, second, x, y + 1 );
		}
	}
}

bool boardSaveToFile( Board * brd, char * filename ) {
	FILE * f = fopen ( filename, "w" );
	if( !f ) {
		errLog( "boardSaveToFile(): Could not open %s for writing", filename );
		return false;
	}
	fprintf( f, "%d\n%d\n%d\n", brd->w, brd->h, brd->color_enabled );

	int x, y;
	for( x = 0; x < brd->w; x++ ) {
		for( y = 0; y < brd->h; y++ ) {

			Cell work = boardGetCell( brd, x, y );

			fprintf( f, "%d\n", work.pattern );
			fprintf( f, "%d\n", work.fg );
			fprintf( f, "%d\n", work.bg );
			fprintf( f, "%d\n", work.bright );
			fprintf( f, "%d\n", work.blink );
		}
	}
	fclose( f );
	return true;
}

bool boardLoadFromFile( Board * brd, char * filename ) {
	// TODO board needs to be reallocated if the dimensions are not the same
	FILE * f = fopen( filename, "r" );
	if( !f ) {
		errLog( "boardLoadFromFile(): Could not load %s", filename );
		return false;
	}
	#define BUF_LEN 32
	int w = 0, h = 0, color_enabled = false;

	Cell work;
	work.pattern = ' ';
	work.fg = COLOR_WHITE;
	work.bg = COLOR_BLACK;
	work.bright = true;
	work.blink = false;

	char buf[BUF_LEN];
	if( fgets( buf, BUF_LEN, f ) != NULL ) {
		w = atoi( buf );
	}
	if( fgets( buf, BUF_LEN, f ) != NULL ) {
		h = atoi( buf );
	}
	if( fgets( buf, BUF_LEN, f ) != NULL ) {
		color_enabled = atoi( buf );
	}

	if( w < 1 || h < 1 ) {
		errLog( "boardLoadFromFile(): invalid dimensions w%d h%d", w, h ); 
		return false;
	}
	brd->w = w;
	brd->h = h;
	brd->color_enabled = color_enabled;

	int x, y;
	for( x = 0; x < w; x++ ) {
		for( y = 0; y < h; y++ ) {
			work.pattern = ' ';
			work.fg = COLOR_WHITE;
			work.bg = COLOR_BLACK;
			work.bright = true;
			work.blink = false;

			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.pattern = atoi( buf );
				 errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.fg = atoi( buf );
				 errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.bg = atoi( buf );
				 errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.bright = atoi( buf );
				 errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.blink = atoi( buf );
				 errLog( buf );
			}

			boardPutCell( brd, work, x, y );
		}
	}
	return true;
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
	Cell primary;
	primary.pattern = '#';
	primary.fg = COLOR_WHITE;
	primary.bg = COLOR_BLACK;
	primary.bright = true;
	primary.blink = false;

	Coord offset = {1, 1} ;

	bool enter_input = false;
	#define USER_INPUT_SZ 64
	char user_input[USER_INPUT_SZ] = {0};
	strcpy( user_input, "scratch.brd" );
	int user_input_spot = 0;
	bool skip_input_one_tick = false;

	while( keep_go ) {
		if( !first_tick && !skip_input_one_tick ) {
			input = getch();
		}
		if( skip_input_one_tick ) {
			input = ERR;
			skip_input_one_tick = false;
		}

		if( enter_input ) {
			if( isascii( input ) && input != '\n' && input != '\r' && input != '\t' ) {
				if( user_input_spot < USER_INPUT_SZ - 1) {
					user_input[user_input_spot] = input;
					user_input[user_input_spot + 1] = '\0';
					user_input_spot++;
				}
			}
			if( input == KEY_BACKSPACE ) {
				if( user_input_spot > 0 ) {
					user_input_spot--;
				}
				user_input[user_input_spot] = '\0';
			}
			if( input == '\n' ) {
				enter_input = false;
				skip_input_one_tick = true;
				continue;
			}
		}
		else {
			if( input == '@' ) {
				enter_input = true;
				skip_input_one_tick = true;
			}
			if( input == 'q') {	// quit
				keep_go = false;
			}
		
			if( input == 'r') { // reset board
				boardWipe( my_board, ' ', COLOR_WHITE, COLOR_BLACK, true, false );
			}
			
			if( input == '\t' ) {	// Toggle doodle mode
				doodle_mode = !doodle_mode;
			}
			
			if( input == 'f' ) {	// Floodfill
				Cell target = boardGetCell( my_board, cursor.x, cursor.y );
				floodFill( my_board, target, primary, cursor.x, cursor.y );
			}
			
			if( input == '\n' ) {
				primary = boardGetCell( my_board, cursor.x, cursor.y );
				mvprintw(24, 24, "Enter!");
			}
	
			if( input == 'c' ) {	// Cycle foreground color
				primary.fg++;
				if( primary.fg > N_COLORS - 1 ) {
					primary.fg = 0;
				}
			}
			if( input == 'v' ) {	// Cycle background color
				primary.bg++;
				if( primary.bg > N_COLORS - 1 ) {
					primary.bg = 0;
				}
			}
			if( input == 'D' ) {
				primary.bright = !primary.bright;
			}
			if( input == 'F' ) {
				primary.blink = !primary.blink;
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
				boardPutCell( my_board, primary, cursor.x, cursor.y );
				//boardPut( my_board, c_pattern, cursor.x, cursor.y );
			}
			if( input == KEY_DC ) {
				Cell empty;
				empty.pattern = ' ';
				empty.fg = COLOR_WHITE;
				empty.bg = COLOR_BLACK;
				empty.bright = true;
				empty.blink = false;
				//boardPut( my_board, ' ', cursor.x, cursor.y );
				boardPutCell( my_board, empty, cursor.x, cursor.y );
			}

			if( input == '[' ) {
				primary.pattern--;
				if( primary.pattern < 32 ) {
					primary.pattern = 126;
				}
			}
			if( input == ']' ) {
				primary.pattern++;
				if( primary.pattern > 126 ) {
					primary.pattern = 32;
				}
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
			if( input == 'S' ) {
				boardSaveToFile( my_board, user_input );
			}
			if( input == 'L' ) {
				boardLoadFromFile( my_board, user_input );
			}
			
			if( doodle_mode ) {
				boardPutCell( my_board, primary, cursor.x, cursor.y );
			}
		}
		clear();
		boardDraw( my_board, offset, true );
	
		if( !doodle_mode ) {
			curs_set( 1 ); // Hmm, this doesn't seem to show a different cursor under my current gnome-terminal.
			mvprintw( 22, 0, "                   " );
		}
		else {
			curs_set( 2 );
			mvprintw( 22, 0,"Doodle Mode Engaged", doodle_mode );
		}
		mvprintw( 23, 0, "X %d Y %d W %d H %d XStep %d YStep %d fg %d bg %d bright %d blink %d\npattern %d / %c", 
		cursor.x, cursor.y, my_board->w, my_board->h, cstep_x, cstep_y, primary.fg, primary.bg, primary.bright, primary.blink, primary.pattern, primary.pattern );

		if( enter_input ) {
			mvprintw( 25, 0, user_input );
		}
		
		curs_set( 1 );
		if( enter_input ) {
			move( 25, 0 + user_input_spot );
		}
		else {
			move( cursor.y + offset.y, cursor.x + offset.x );
		}
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
