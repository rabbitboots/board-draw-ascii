#include "board.h"

bool outOfBounds( int x, int y, int w, int h ) {
	return ( x < 0 || x > w - 1 || y < 0 || y > h - 1 );
}


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
		errLog( "boardInit(): invalid dimensions." );
		return NULL;
		exit(1);
	}

	Board * new_board = malloc( sizeof(Board) );
	if( !new_board ) {
		errLog( "boardInit(): malloc() failed on new_board" );
		return NULL;
	}

	new_board->w = w;
	new_board->h = h;
	new_board->color_enabled = color;

	new_board->filename = malloc( sizeof(TEST_FILE) );
	if( !new_board->filename ) {
		errLog( "boardInit(): malloc() failed on new_board->filename" );
		return NULL;
	}
	strncpy( new_board->filename, TEST_FILE, sizeof( TEST_FILE ) );

	new_board->cells = malloc( (new_board->w * new_board->h) * sizeof(Cell) );
	if( !new_board->cells ) {
		errLog( "boardInit(): malloc() failed on new_board->cells" );
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

Board * boardLoadFromFile( char * filename ) {
//bool boardLoadFromFile( Board * brd, char * filename ) {
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
		errLog( "boardLoadFromFile(): invalid dimensions (w%d h%d) on %s", w, h, filename ); 
		return NULL;
	}

	Board * brd = boardInit( w, h, color_enabled );
	if( !brd ) {
		errLog( "boardLoadFromFile(): malloc failed on brd" );
		goto cleanup;
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
				 //errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.fg = atoi( buf );
				 //errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.bg = atoi( buf );
				 //errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.bright = atoi( buf );
				 //errLog( buf );
			}
			if( fgets( buf, BUF_LEN, f ) != NULL ) {
				 work.blink = atoi( buf );
				 //errLog( buf );
			}

			boardPutCell( brd, work, x, y );
		}
	}
	cleanup:
	fclose( f );
	return brd;
}

bool boardCopySection( Board * target, Board * dest, int tx, int ty, int tw, int th, int dx, int dy ) {
	if( !target || !dest ) {
		errLog( "boardCopySection(): Supplied NULL pointer(s).");
		return false;
	}
	if( tw < 1 || th < 1 ) {
		errLog( "boardCopySection(): Supplied 0 or negative number for target w/h.");
		return false;
	}

	int x, y;

	for( x = tx; x < tx + tw; x++ ) {
		for( y = ty; y < ty + th; y++ ) {
			boardPutCell( dest, boardGetCell( target, x, y ), dx + (x-tx), dy + (y-ty) );
		}
	}
}

Board * boardMakeFromSelection( Board * target, int tx, int ty, int tw, int th ) {
	Board * dest = boardInit( tw, th, target->color_enabled );
	if( !dest ) {
		errLog( "boardMakeFromSelection(): boardInit() failed. Return NULL." );
		goto cleanup;
	}

	boardCopySection( target, dest, tx, ty, tw, th, 0, 0 );

	cleanup:
	return dest;
}

