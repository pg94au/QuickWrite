/*
 *  QuickWrite
 *  (c)1993,1994 Insync Development
 *
 *  qw_error.h
 */


#ifndef QW_ERROR_H	/* prevent multiple includes */
#define QW_ERROR_H


/* function prototypes */

void ER_put_error(int code);

/* */


/* This will contain the maximum number of error codes */
#define MAXERROR 11
/* */

/* This is the list of error codes */
#define OUT_OF_MEM 0
#define NOPOW 1
#define ERREAD 2
#define ERWRITE 3
#define ERCLOSE 4
#define ERROR_BUFFER 5
#define ERROR_COMMAND_LINE 6 
#define CURSES_ERROR 7
#define ERROR_ABOUT 8
#define WINDOW_ERROR 9
#define ERROR_BUFFSTAT_WIN 10
/* */


#endif	/* QW_ERROR_H */

