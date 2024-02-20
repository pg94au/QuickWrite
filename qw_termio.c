/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_termio.c
 */





#include <stdio.h>
#include <assert.h>
#include <curses.h>
#include "include/qw_defs.h"
#include "include/qw_error.h"


/* pointers for the three major windows used in the program */
WINDOW *edit_win, *menubar_win, *statusbar_win;
/* */

/* function prototypes */

bool TM_initialize_display(void);
void TM_close_display(void);
void TM_redraw_screen(void);
void TM_write_statusbar(char *string);
void TM_write_menubar(char *string);
bool TM_read_string(int xpos, int maxlength, char *string);
bool TM_read_value(int xpos, int maxlength, int *value);
void TM_wait_bar(int xpos);

/* */





/*  TM_initialize_display()  -  Initialize the screen through curses
 *                              and open the three major windows
 *
 *  Parameters:  none
 *
 *  Returns:  TRUE if successful
 *            FALSE if unsuccessful (unknown terminal type or
 *                 out of memory)
 */
bool TM_initialize_display()
{
	/* initialize curses display */
	if (initscr() == NULL)
	{
		ER_put_error(CURSES_ERROR);
		return FALSE;
	}
	/* */

	/* set preferences for curses display */
	keypad(stdscr, TRUE);
	noecho();
	raw();
	refresh();
	/* */

	/* open the main windows */
	menubar_win = newwin(1,SCREEN_WIDTH,0,0);
	if (menubar_win == NULL)
	{
		endwin();
		ER_put_error(CURSES_ERROR);
		return FALSE;
	}
	keypad(menubar_win, TRUE);	/* enable keypad */
	noecho();
	raw();

	edit_win = newwin(EWH,SCREEN_WIDTH,1,0);
	if (edit_win == NULL)

	{
		delwin(menubar_win);
		endwin();
		ER_put_error(CURSES_ERROR);
		return FALSE;
	}
	idlok(edit_win, TRUE);		/* use H/W scrolling */
	scrollok(edit_win, TRUE);	/* this window scrolls */
	keypad(edit_win, TRUE);		/* enable keypad */
	noecho();
	raw();

	statusbar_win = newwin(1,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);
	if (statusbar_win == NULL)
	{
		delwin(menubar_win);
		delwin(edit_win);
		endwin();
		ER_put_error(CURSES_ERROR);
		return FALSE;
	}
	keypad(statusbar_win, TRUE);	/* enable keypad */
	noecho();
	raw();
	/* */

	refresh();

	return TRUE;
}





/*  TM_close_display()  -  Close the three major windows and cleanup
 *                         the screen & curses
 *
 *  Parameters:  none
 *
 *  Returns:  none
 */
void TM_close_display()
{
	/* close the three major windows */
	delwin(menubar_win);
	delwin(edit_win);
	delwin(statusbar_win);
	/* */

	/* cleanup curses display */
	endwin();
	/* */
}





/*  TM_redraw_screen()  -  Redraw the entire contents of the screen again.
 *
 *  Parameters:  none
 *
 *  Returns:  none
 *
 *  Notes:  This function is useful as it allows the program to redraw the
 *          entire screen if for example the user's display is garbled by
 *          line noise, etc.
 */
void TM_redraw_screen()
{
	/* make sure the screen gets completely redrawn on refresh */
	clearok(curscr, TRUE);
	wrefresh(curscr);
	/* */
}





/*  TM_write_statusbar()  -  write a string to the statusbar of the screen
 *
 *  Parameters:  A pointer to the string to be written to the status bar
 *
 *  Returns:  none
 *
 *  Notes:  Since this function writes to the last line on the terminal
 *          screen, it can only print out a maximum of 79 chars to the line.
 *          Attempting to print 80 chars will cause the display to scroll,
 *          which would not be a good thing.
 *
 *          Special characters and their interpreted meanings:
 *
 *               '\n' :  set draw mode to normal text
 *               '\r' :  set draw mode to reverse text
 *               '\b' :  set draw mode to bold text
 *               '\f' :  set draw mode to dim text
 *               '\t' :  set draw mode to standout text
 *
 *          The inclusion of a special character other than that for normal
 *          text will turn on that drawing feature.  Thus the inclusion of
 *          "...\r\b..." in the source string will print all following
 *          characters in bold and reverse.  In order to then switch to only
 *          bold text for example, the following sequence must be included
 *          in the string "\n\b".
 *
 *          The characters listed above were selected for convenience, as
 *          they would have no meaning otherwise and would not be allowed as
 *          legal characters to be printed to the status bar.
 */
void TM_write_statusbar(char *string)
{
	int pos = 0, chars = 0, x;


	/* Assertions */
	assert(string != NULL);
	/* */


	wmove(statusbar_win, 0,0);	/* rewrite previous line */
	wattrset(statusbar_win, A_NORMAL);

	/* print the given string */
	while ((chars < SCREEN_WIDTH-1) && (string[pos] != NULL))
	{
		switch (string[pos])
		{
			case '\n':
				wattrset(statusbar_win, A_NORMAL);
				break;
			case '\r':
				wattron(statusbar_win, A_REVERSE);
				break;
			case '\b':
				wattron(statusbar_win, A_BOLD);
				break;
			case '\f':
				wattron(statusbar_win, A_DIM);
				break;
			case '\t':
				wattron(statusbar_win, A_STANDOUT);
				break;
			default:
				waddch(statusbar_win, string[pos]);
				chars++;
				break;
		}
		pos++;
	}
	/* */

	/* fill in the rest of the line with reversed spaces */
	wattrset(statusbar_win, A_REVERSE);

	for (x=chars; x < SCREEN_WIDTH-1; x++)
	{
		waddch(statusbar_win, ' ');
	}
	/* */

	/* refresh the status bar on screen */
	wrefresh(statusbar_win);
	/* */
}





/*  TM_write_menubar()  -  write a string to the menu bar of the screen
 *                         This function will print up to 80 chars to the
 *                         menubar.
 *
 *  Parameters:  A pointer to the string to be copied to the menu bar.
 *
 *  Returns:  none
 *
 *  Notes:  See TM_write_statusbar() for details on the format of strings
 *          passed.
 */
void TM_write_menubar(char *string)
{
	int pos = 0, chars = 0, x;


	/* Assertions */
	assert(string != NULL);
	/* */


	wmove(menubar_win, 0,0);	/* start of line */
	wattrset(menubar_win, A_NORMAL);

	/* print the given string */
	while ((chars < SCREEN_WIDTH) && (string[pos] != NULL))
	{
		switch (string[pos])
		{
			case '\n':
				wattrset(menubar_win, A_NORMAL);
				break;
			case '\b':
				wattron(menubar_win, A_REVERSE);
				break;
			case '\r':
				wattron(menubar_win, A_BOLD);
				break;
			case '\f':
				wattron(menubar_win, A_DIM);
				break;
			case '\t':
				wattron(menubar_win, A_STANDOUT);
				break;
			default:
				waddch(menubar_win, string[pos]);
				chars++;
				break;
		}
		pos++;
	}
	/* */

	/* fill in the rest of the line with reversed spaces */
	wattrset(menubar_win, A_REVERSE);

	for (x=chars; x < SCREEN_WIDTH; x++)
	{
		waddch(menubar_win, ' ');
	}
	/* */

	/* refresh the menu bar on the screen */
	wrefresh(menubar_win);
	/* */
}





/*  TM_read_string()  -  Read in a string from the status bar on the bottom
 *                       of the screen.
 *
 *  Parameters:  xpos - position which to begin string prompt on bar
 *               maxlength - maximum chars which can be read into string
 *               string - pointer to string into which chars will be copied
 *                        The string will be null-terminated.
 *
 *  Returns:  TRUE if a string was successfully entered by the user
 *            FALSE if the user aborted the prompt before hitting enter
 */
bool TM_read_string(int xpos, int maxlength, char *string)
{
	int x, chars, pos, c;
	char tmpstring[80];


	/* Assertions */
	assert(string != NULL);
	assert(xpos >= 0);
	assert((xpos+maxlength) < SCREEN_WIDTH);
	/* */


	/* move to proper spot and clear space on the line */
	wmove(statusbar_win, 0, xpos);
	wrefresh(statusbar_win);
	/* */

	/* set initial position */
	chars = 0;
	pos = 0;
	tmpstring[0] = NULL;
	/* */

	/* read in the string from the user */
	for (;;)
	{
		c = wgetch(statusbar_win);

		switch (c)
		{
			case KEY_REDRAW:
				/* redraw the screen for the user */
				TM_redraw_screen();
				/* */
				break;
			case KEY_ENTER:
			case KEY_RETURN:
				if (strlen(tmpstring) > 0)	/* did the user actually type anything? */
				{
					strcpy(string, tmpstring);

					return TRUE;
				}
				else
				{
					return FALSE;	/* user just enter on empty line */
				}
				break;
			case KEY_SWITCH:
				return FALSE;
				break;
			case KEY_LEFT:
			case KEY_LEFT2:
				/* handle left arrow key */
				if (pos > 0)
				{
					wmove(statusbar_win, 0, (--pos + xpos));
				}
				else
				{
					beep();
				}
				/* */
				break;
			case KEY_RIGHT:
			case KEY_RIGHT2:
				/* handle right arrow key */
				if (pos < chars)
				{
					wmove(statusbar_win, 0, (++pos + xpos));
				}
				else
				{
					beep();
				}
				/* */
				break;
			case KEY_BACKSPACE:
			case KEY_BACKSPACE2:
				/* basically move left and delete */
				if (pos > 0)	/* setup for delete */
				{
					pos--;
					wmove(statusbar_win, 0, xpos + pos);
				}
				else	/* no can do */
				{
					beep();
					break;
				}
				/* */
			case KEY_DELETE:
			case KEY_DELETE2:
				/* delete current character */
				if (chars > pos)
				{
					wdelch(statusbar_win);
					for (x=pos; x < chars; x++)
					{
						tmpstring[x] = tmpstring[x+1];
					}
					chars--;

					mvwaddch(statusbar_win, 0, SCREEN_WIDTH-2, ' ');	/* put back the last space */
					wmove(statusbar_win, 0, xpos + pos);
				}
				else	/* no can do */
				{
					beep();
				}
				/* */
				break;
			default:
				/* check if this char is printable */
				if ((c >= 32) && (c <= 126))
				{
					if (chars < maxlength)
					{
						winsch(statusbar_win, c);
						for (x=chars; x >= pos; x--)
						{
							tmpstring[x+1] = tmpstring[x];
						}
						tmpstring[pos] = c;
						wmove(statusbar_win, 0, (++pos + xpos));
						chars++;
					}
					else
					{
						beep();
					}
				}
				else
				{
					beep();
				}
				/* */
				break;
		}

		wrefresh(statusbar_win);
	}
	/* */
}





/*  TM_read_value()  -  Read in a numerical value from the status bar on
 *                      the bottom of the screen.
 *
 *  Parameters:  xpos - position which to begin string prompt on bar
 *               maxlength - maximum number of digits to be accepted
 *               value - pointer to integer into which the value  will be
 *                       stored
 *
 *  Returns:  none
 *
 *  Notes:  The maximum length possible of an integer to be read in is 9.
 *          This is the limit of digits for which we can be sure a number
 *          can be stored in a 32 bit integer without overflowing.  Nine
 *          digits will still provide a much larger range of values than
 *          will ever be necessary for this program.
 */
bool TM_read_value(int xpos, int maxlength, int *value)
{
	int x, chars, pos, c;
	char string[10];


	/* Assertions */
	assert(string != NULL);
	assert(xpos >= 0);
	assert((xpos+maxlength) < SCREEN_WIDTH);
	/* */


	/* move to proper spot and clear space on the line */
	wmove(statusbar_win, 0, xpos);
	wrefresh(statusbar_win);
	/* */

	/* set initial position */
	chars = 0;
	pos = 0;
	string[0] = NULL;
	/* */

	/* read in the string from the user */
	for (;;)
	{
		c = wgetch(statusbar_win);

		switch (c)
		{
			case KEY_REDRAW:
				/* redraw the screen for the user */
				TM_redraw_screen();
				/* */
				break;
			case KEY_ENTER:
			case KEY_RETURN:
				if (strlen(string) > 0)	/* did the user actually type anything? */
				{
					*value = atoi(string);  /* convert string to integer */
					return TRUE;
				}
				else
				{
					return FALSE;	/* hit enter on empty line */
				}
				break;
			case KEY_SWITCH:
				return FALSE;
				break;
			case KEY_LEFT:
			case KEY_LEFT2:
				/* handle left arrow key */
				if (pos > 0)
				{
					wmove(statusbar_win, 0, (--pos + xpos));
				}
				/* */
				break;
			case KEY_RIGHT:
			case KEY_RIGHT2:
				/* handle right arrow key */
				if (pos < chars)
				{
					wmove(statusbar_win, 0, (++pos + xpos));
				}
				/* */
				break;
			case KEY_BACKSPACE:
			case KEY_BACKSPACE2:
				/* basically move left and delete */
				if (pos > 0)	/* setup for delete */
				{
					pos--;
					wmove(statusbar_win, 0, xpos + pos);
				}
				else	/* no can do */
				{
					beep();
					break;
				}
				/* */
			case KEY_DELETE:
				/* delete current character */
				if (chars > pos)
				{
					wdelch(statusbar_win);
					for (x=pos; x < chars; x++)
					{
						string[x] = string[x+1];
					}
					chars--;

					mvwaddch(statusbar_win, 0, SCREEN_WIDTH-2, ' ');	/* add the last space in */
					wmove(statusbar_win, 0, xpos + pos);
				}
				else	/* no can do */
				{
					beep();
				}
				/* */
				break;
			default:
				/* check if this char is printable */
				if ((c >= '0') && (c <= '9'))
				{
					if (chars < maxlength)
					{
						winsch(statusbar_win, c);
						for (x=chars; x >= pos; x--)
						{
							string[x+1] = string[x];
						}
						string[pos] = c;
						wmove(statusbar_win, 0, (++pos + xpos));
						chars++;
					}
					else
					{
						beep();
					}
				}
				else
				{
					beep();
				}
				/* */
				break;
		}

		wrefresh(statusbar_win);
	}
	/* */
}




/*  TM_wait_bar()  -  Used to draw the circling bar that lets user know that
 *                    something is being done during lengthy operations.
 *
 *  Parameters:  the xpos on the statusbar to draw the bar
 *
 *  Returns:  none
 */
void TM_wait_bar(int xpos)
{
	static int count = 0;
	char *lines = "|/-\\";

	/* Assertions */
	assert(xpos >= 0);
	assert(xpos < 80);
	/* */

	/* draw the character on the statusbar at xpos */
	wmove(statusbar_win, 0, xpos);
	waddch(statusbar_win, lines[count]);
	wmove(statusbar_win, 0, 78);
	wrefresh(statusbar_win);
	/* */

	/* increment count */
	if (++count > 3)
	{
		count = 0;
	}
	/* */
}

