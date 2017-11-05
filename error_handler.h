#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "curses.h"

// -- Variables

typedef struct error_handler_t {
    int initialized;
    int redirect_to_stderr;
    FILE *f_err_log;
} ErrorHandler;

ErrorHandler error_handler;


// -- Functions

// Startup / shutdown
// TODO include an option to *not* log debug messages at all.
void errorHandlerInit( ErrorHandler * e, int redirect_to_stderr );
void errorHandlerShutdown( ErrorHandler * e );

// Log an error message.
void errLog( char * formatted_string, ... );

// Log an error and crash the application.
void errQuit( char * formatted_string, ... );

// Variadic function wrapper for logging formatted messages.
// This should only be used by errLog() and errQuit().
void errLogVaList( char * formatted_string, va_list args );

#endif // ERROR_HANDLER_H
