#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include "curses.h"

#include "error_handler.h"
#include "curses_wrapper.h"
#include "draw.h"
#include "board.h"

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

	bool typewriter_mode = false;
	bool enter_input = false;
	#define USER_INPUT_SZ 64
	char user_input[USER_INPUT_SZ] = {0};
	strcpy( user_input, "scratch.brd" );
	int user_input_spot = 0;
	bool skip_input_one_tick = false;
	Coord clip_z = {0};
	Coord clip_x = {0};
	Board * clipboard = NULL;

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
		else if( typewriter_mode ) {
			if( isascii( input ) && input != '\t' && input != KEY_DC && input != '\n' ) {
				primary.pattern = input;
				boardPutCell( my_board, primary, cursor.x, cursor.y );
				if( cursor.x < my_board->w - 1 ) {
					cursor.x++;
				}
			}
			if( input == KEY_BACKSPACE ) {
				primary.pattern = ' ';
				boardPutCell( my_board, primary, cursor.x - 1, cursor.y );
				if( cursor.x > 0 ) {
					cursor.x--;
				}
			}
			if( input == '\n' ) {
				typewriter_mode = false;
				skip_input_one_tick = true;
				continue;
			}
		}
		else {

			if( input == 'z' ) {
				clip_z.x = cursor.x;
				clip_z.y = cursor.y;
			}
			if( input == 'x' ) {
				clip_x.x = cursor.x;
				clip_x.y = cursor.y;
			}
			if( input == 'Z' ) {
				// Copy to clipboard buffer
				if( clipboard ) {
					boardFree( clipboard );
					clipboard = NULL;
				}
				clipboard = boardMakeFromSelection( my_board, clip_z.x, clip_z.y, clip_x.x - (clip_z.x - 1), clip_x.y - (clip_z.y - 1) );
			}
			if( input == 'X' ) {
				// Paste from clipboard buffer
				if( clipboard ) {
					boardCopySection( clipboard, my_board, 0, 0, clipboard->w, clipboard->h, cursor.x, cursor.y );
				}
			}
			if( input == 't' ) {
				typewriter_mode = true;
			}

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
				Board * try_load = boardLoadFromFile( user_input );
				if( try_load ) {
					boardFree( my_board );
					my_board = NULL;
					my_board = try_load;
				}
				else {
					errLog( "Couldn't load %s into a Board structure.", user_input );
				}
			}
			
			if( doodle_mode ) {
				boardPutCell( my_board, primary, cursor.x, cursor.y );
			}
		}
		clear();
		boardDraw( my_board, offset, true );
	
		if( !doodle_mode ) {
			curs_set( 1 ); // Hmm, this doesn't seem to show a different cursor under my current gnome-terminal.
			mvprintw( 22, 0, "                                          " );
		}
		else {
			curs_set( 2 );
			mvprintw( 22, 0,"Doodle Mode Engaged - TAB to stop" );
		}
		if( typewriter_mode ) {
			mvprintw( 22, 0, "Typewriter Mode Engaged - ENTER to stop" );
		}

		mvprintw( 23, 0, "X %d Y %d W %d H %d XStep %d YStep %d fg %d bg %d bright %d blink %d\npattern %d / %c clip_z %d %d clip_x %d %d", 
		cursor.x, cursor.y, my_board->w, my_board->h, cstep_x, cstep_y, primary.fg, primary.bg, primary.bright, primary.blink, primary.pattern, primary.pattern, clip_z.x, clip_z.y, clip_x.x, clip_x.y );

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

	/* Shutdown */
	if( my_board ) {
		boardFree( my_board );
		my_board = NULL;
	}

	/* Close error handler */
    errLog("    **  Shutting down.  **\n");
    errorHandlerShutdown( &error_handler );
	
	endwin();
	return 0;
}
