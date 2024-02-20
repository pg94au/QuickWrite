/*  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_error.c
 */

#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "include/qw_termio.h"

/* function prototypes */
void ER_put_error(int code);
/* */

extern WINDOW *statusbar_win; /* This is the pointer to the status bar, which is used to display error message */


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

/* This is the table which contains all the error messages */
char error_table[MAXERROR][80]={"Out of Memory.",
				"Power Failure.",
				"Error Reading file.",
				"Error Writing file.",
				"Error Closing file.",
				"Error in buffer.",
				"Error in command line arguements.",
				"Error in curses.",
				"Error in opening a window for about.",
				"Error in opening window.",
				"Unable to open buffer status window."};




/* ER_put_error() -  This will print out an appropriate error message corressponding on the error.
 *                   code that was sent to it.
 * Parameters:  Requires an integer which will be an index into a table of error messages.
 *
 * Returns:  None
 */
void ER_put_error(int code)
{
	int x; /* This is used as a general purpose variable. */
	size_t length; /* This is used to contain the length of the error message */

	/* Check to see if the window is useable */
	if (statusbar_win!=NULL)
	{
		/* Yes the window is useable, so the error message will be printed on the statusbar */
		beep();
		wmove(statusbar_win,0,0);
		wattrset(statusbar_win, A_BOLD | A_REVERSE);
		length=strlen(error_table[code]);
		waddstr(statusbar_win, "                                                                               ");
		wmove(statusbar_win, 0, 0);
		waddstr(statusbar_win, error_table[code]);
		wrefresh(statusbar_win);
		sleep(2); /* Pause for 2 seconds. */
		waddstr(statusbar_win," Please hit any key to continue.");
		for (x=length+32; x<=79; x++) /* 32 is the size of "Please hit any key to continue" */
			waddch(statusbar_win,' '); /* Fill the rest of the line with spaces. */
		wrefresh(statusbar_win);
		x=getch();
		/* */
	}
	else
	{
		/* No the window is not useable, so the error message will be printed to the screen
		   directly */
		printf("%s", error_table[code]);
		/* */
		
		return;
	}
}



