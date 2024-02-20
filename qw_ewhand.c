/*  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_ewhand.c
 */


#include <curses.h>
#include <assert.h>
#include "include/qw_defs.h"
#include "include/qw_buffio.h"


extern WINDOW *edit_win;	/* pointer to the edit window */

int cursor_x;	/* current position of the cursor in the edit window from the left edge */
int cursor_y;	/* current position of the cursor in the edit window from the top edge */
int desired_x;	/* x positoin of the cursor stored from the last left/right movement */
		/* useful for cursoring up/down */

int ew_topline;		/* number of line (in buffer) currently displayed on first line of edit window */
int ew_currline;	/* number of line (in buffer) that the cursor is currently at */


/* function prototypes */

void EW_initial_draw(BUFFER *buffptr);
void EW_refresh(void);
void EW_touch(void);
void EW_update_position(BUFFER *buffptr);
void EW_get_cursor_pos(int *cur_x, int *des_x, int *cur_y);
void EW_set_cursor_pos(int new_cursor_x, int new_desired_x);
void EW_insert_char(int c);
void EW_delete_char(void);
void EW_redraw_char(BUFFER *buffptr, int mode);
void EW_insert_line(BUFFER *buffptr, int x);
void EW_delete_line(BUFFER *buffptr, int code);
void EW_update_lines(BUFFER *buffptr, int new_cursor_x, int cursor_line, int lines_put, int wrap_pos, bool last_line);
bool EW_reposition_cursor(BUFFER *buffptr, int new_cursor_y);
void EW_redraw_window(BUFFER *buffptr);

/* */





/*  EW_initial_draw()  -  Draw the initial contents of the edit window upon
 *                        loading a new file.  Write the first lines of the
 *                        buffer to the edit window and position the cursor
 *                        at position (0,0).
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  none
 */
void EW_initial_draw(BUFFER *buffptr)
{
	int x, length, pos;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* erase edit window */
	werase(edit_win);
	wmove(edit_win, 0, 0);
	/* */
	
	/* go to top of buffer */
	BF_goto_start(buffptr);
	/* */
	
	/* set cursor to (0,0) */
	cursor_x = 0;
	cursor_y = 0;
	desired_x = 0;
	ew_topline = 1;
	ew_currline = 1;
	/* */
	
	/* draw as much of edit window as possible with buffer contents */
	for (x=0; x < EWH; x++)
	{
		/* get line to draw */
		length = BF_get_line(buffptr, string);
		/* */
		
		/* draw the line now */
		for (pos=0; pos < length; pos++)
		{
			waddch(edit_win, string[pos]);
		}
		/* */
		
		/* was this the last line in the buffer? */
		if (BF_move_ahead(buffptr) == FALSE)
		{
			break;
		}
		/* */
		
		/* prepare to draw next line */
		if (x < EWH-1)
		{
			waddch(edit_win, '\n');
		}
		/* */
	}
	/* */
	
	/* reposition in buffer and in window */
	BF_goto_start(buffptr);
	wmove(edit_win, 0, 0);
	/* */
}





/*  EW_refresh()  -  Refresh the edit window to display all changes made to
 *                   it since the last refresh.
 *
 *  Parameters:  none
 *
 *  Returns:  none
 *
 *  Notes:  None of the other edit window handler functions will actually
 *          update the user's terminal to reflect and of the changes they
 *          may make.  This function must be called in order to actually
 *          display the changes made by any of the other functions.
 *
 *          The reason for this decision is to allow several changes to be
 *          made to the edit window before actually updating it, which may
 *          optimize the speed of the actual redraw on the terminal.
 */
void EW_refresh()
{
	/* simply refresh the edit window */
	wrefresh(edit_win);
	/* */
}





/*  EW_touch()  -  Set the condition so that the next time the edit window
 *                 is refreshed it will redraw portions which have been
 *                 covered up by another window drawn on top of it.
 *
 *  Parameters:  none
 *
 *  Returns:  none
 */
void EW_touch()
{
	/* 'touch' the edit window so it will be redisplayed after being covered by another window */
	touchwin(edit_win);
	/* */
}





/*  EW_update_position()  -  Use the current line position in the buffer
 *                           and the current cursor position in the edit
 *                           window to redraw the screen to show the new
 *                           position where the cursor is.  This may
 *                           involve changing the contents of the edit
 *                           window to show a new portion of the buffer if
 *                           necessary.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  none
 */
void EW_update_position(BUFFER *buffptr)
{
	int length, ew_newline, pos, x, numlines;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	ew_newline = BF_get_line_number(buffptr);
	
	/* no scrolling necessary? */
	if ((ew_newline >= ew_topline) && (ew_newline <= ew_topline+EWH-1))
	{
		/* find new position */
		cursor_y = ew_newline - ew_topline;
		ew_currline = ew_newline;
		/* */
		
		wmove(edit_win, cursor_y, cursor_x);
		return;
	}
	/* */

	/* cannot use any of the lines currently on the screen? */
	if ((ew_newline <= ew_topline-EWH) || (ew_newline >= ew_topline+2*EWH-1))
	{
		/* go to new top line */
		ew_currline = ew_newline;
		numlines = BF_number_lines(buffptr);
		if (ew_newline > numlines - (EWH-1))
		{
			ew_topline = numlines - (EWH-1);
		}
		else
		{
			ew_topline = ew_currline;
		}
		BF_goto_line(buffptr, ew_topline);
		/* */

		/* erase edit window */
		werase(edit_win);
		wmove(edit_win, 0, 0);
		/* */

		/* set new cursor position */
		cursor_y = ew_currline - ew_topline;
		/* */

		/* draw contents of edit window */
		for (x=0; x < 22; x++)
		{
			/* get line to draw */
			length = BF_get_line(buffptr, string);
			/* */

			/* draw the line now */
			for (pos=0; pos < length; pos++)
			{
				waddch(edit_win, string[pos]);
			}
			/* */

			/* prepare to draw next line */
			BF_move_ahead(buffptr);
			if (x < EWH-1)
			{
				waddch(edit_win, '\n');
			}
			/* */
		}
		/* */

		/* reposition in buffer and in window */
		BF_goto_line(buffptr, ew_currline);
		wmove(edit_win, cursor_y, cursor_x);
		/* */
	
		return;
	}
	/* */
	
	/* need to scroll back? */
	if (ew_newline < ew_topline)
	{
		/* get ready to scroll back */
		BF_goto_line(buffptr, ew_topline);
		/* */
		ew_currline = ew_topline;
		
		/* scroll up adding lines at the top */
		do
		{
			wmove(edit_win, 0, 0);
			winsertln(edit_win);	/* insert line at top of window */
			BF_move_back(buffptr);
			length = BF_get_line(buffptr, string);	/* get line to draw */
			for (pos=0; pos < length; pos++)
			{
				waddch(edit_win, string[pos]);
			}
			ew_currline--;
		}
		while (ew_currline > ew_newline);
		/* */
		
		ew_topline = ew_currline;
		cursor_y = 0;
		wmove(edit_win, cursor_y, cursor_x);
		return;
	}
	/* */
	
	/* need to scroll ahead? */
	ew_currline = ew_topline+EWH-1;
	BF_goto_line(buffptr, ew_currline);
	
	/* scroll down adding lines at the bottom */
	do
	{
		scroll(edit_win);	/* insert line at bottom of window */
		wmove(edit_win, EWH-1, 0);
		BF_move_ahead(buffptr);
		length = BF_get_line(buffptr, string);	/* get line to draw */
		for (pos=0; pos < length; pos++)
		{
			waddch(edit_win, string[pos]);
		}
		ew_currline++;
	}
	while (ew_currline < ew_newline);
	/* */
	
	ew_topline = ew_currline-(EWH-1);
	cursor_y = EWH-1;
	wmove(edit_win, cursor_y, cursor_x);
	return;
}





/*  EW_get_cursor_pos()  -  Get the current position of the cursor in the
 *                          edit window.
 *
 *  Parameters:  pointers to integers corresponding to the cursor_x,
 *                    desired_x and cursor_y destination variables
 *
 *  Returns:  none
 *
 *  Notes:  This function allows only the values required to be selectively
 *          obtained.  If a particular value is not required, a NULL
 *          pointer may be passed in place of a pointer to an actual
 *          integer.
 */
void EW_get_cursor_pos(int *cur_x, int *des_x, int *cur_y)
{
	if (cur_x != NULL)
	{
		*cur_x = cursor_x;
	}
	if (des_x != NULL)
	{
		*des_x = desired_x;
	}
	if (cur_y != NULL)
	{
		*cur_y = cursor_y;
	}
}





/*  EW_set_cursor_pos()  -  Set the x position of the cursor in the edit
 *                          window.
 *
 *  Parameters:  The new x position of the cursor
 *               The desired x position for use in positioning after a
 *                    cursor up or cursor down
 *
 *  Returns:  none
 */
void EW_set_cursor_pos(int new_cursor_x, int new_desired_x)
{
	/* Assertions */
	assert(new_cursor_x >= 0);
	assert(new_cursor_x < LLEN);
	assert(new_desired_x >= 0);
	assert(new_desired_x < LLEN);
	/* */
	
	cursor_x = new_cursor_x;
	desired_x = new_desired_x;
}





/*  EW_insert_char()  -  Insert a character into the edit window at the
 *                       current cursor position.
 *
 *  Parameters:  The character to be inserted
 *
 *  Returns:  none
 *
 *  Notes:  This function does not take care of word wrap.
 */
void EW_insert_char(int c)
{
	/* move to (cursor_y,cursor_x) and then insert character c there */
	mvwinsch(edit_win, cursor_y, cursor_x, c);
}





/*  EW_delete_char()  -  Delete the character at the current cursor
 *                       position.
 *
 *  Parameters:  none
 *
 *  Returns:  none
 *
 *  Notes:  This function does not take care of wrapping line back up.
 */
void EW_delete_char()
{
	/* move to (cursor_y,cursor_x) and then delete the character there */
	mvwdelch(edit_win, cursor_y, cursor_x);
}





/*  EW_redraw_char()  -  Redraw the character under the cursor in the edit
 *                       window in the style selected.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               A mode which can be any of the drawing modes defined in
 *                    curses.h
 *
 *  Returns:  none
 *
 *  Notes:  Make SURE that the cursor was positioned where you want it to
 *          be via EW_update_position() BEFORE calling this function.
 *          (ie. do not just BF_move_back then EW_redraw_char)
 */
void EW_redraw_char(BUFFER *buffptr, int mode)
{
	char string[LLEN];
	int length;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	length = BF_get_line(buffptr, string);

	/* check if the cursor is at the end of the line, since we aren't */
	/* supposed to redraw the character in the buffer past the length */
	if (cursor_x >= length)
	{
		return;
	}
	/* */
	
	wattrset(edit_win, mode);	/* set draw mode specified */
	
	/* move to cursor location and redraw the character there */
	mvwaddch(edit_win, cursor_y, cursor_x, string[cursor_x]);
	/* */
	
	wattrset(edit_win, A_NORMAL);	/* set draw mode back to normal */
	
	wmove(edit_win, cursor_y, cursor_x);
}





/*  EW_insert_line()  -  Erase the current line after the specified
 *                       position, insert a line below it, then print the
 *                       next line below it, leaving the cursor at position
 *                       zero.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               The position at which to cut off the current line
 *
 *  Returns:  none
 *
 *  Notes:  This function provides the results expected when the enter key
 *          is hit while in edit mode.
 */
void EW_insert_line(BUFFER *buffptr, int x)
{
	int pos, length;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(x >= 0);
	assert(x < LLEN);
	/* */
	
	
	/* go to location of cursor and delete the rest of the line after it */
	wmove(edit_win, cursor_y, x);	
	wclrtoeol(edit_win);
	/* */

	/* reset cursor position */
	cursor_x = 0;
	desired_x = 0;
	/* */
	
	ew_currline++;	/* cursor goes to next line */
	if (cursor_y < EWH-1)
	{
		cursor_y++;
		wmove(edit_win, cursor_y, cursor_x);
		winsertln(edit_win);
	}
	else
	{
		scroll(edit_win);
		wmove(edit_win, cursor_y, cursor_x);
		ew_topline++;	/* topline has changed due to scroll */
	}
	/* */
	
	/* print the current line to the window */
	length = BF_get_line(buffptr, string);
	for (pos=0; pos < length; pos++)
	{
		waddch(edit_win, string[pos]);
	}
	/* */

	/* reposition cursor at the beginning of this line */
	wmove(edit_win, cursor_y, cursor_x);
	/* */
}





/*  EW_delete_line()  -  Delete the current line from the edit window,
 *                       and determine whether to scroll the rest of the
 *                       window up or move the cursor up instead.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               A code telling this function how to perform the line
 *                    deletion
 *
 *  Returns:  none
 *
 *  Notes:  The code required to be passed to this function is the code
 *          which is returned from a call to BF_delete_line().
 */
void EW_delete_line(BUFFER *buffptr, int code)
{
	int pos, length, line_number;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert((code >= DL_EMPTY) && (code <= DL_OK));
	/* */
	
	
	cursor_x = 0;
	desired_x = 0;
	
	/* was the only line in the buffer just deleted? */
	if (code == DL_ONLY)
	{
		wdeleteln(edit_win);
		wmove(edit_win, cursor_y, cursor_x);
		return;
	}
	/* */
	
	/* was the last line in the buffer just deleted? */
	if (code == DL_LAST)
	{
		wdeleteln(edit_win);
		
		if (cursor_y > 0)
		{
			cursor_y--;
			ew_currline--;
			wmove(edit_win, cursor_y, cursor_x);
			return;
		}
		else
		{
			/* last line in buffer was the only line at the top of the window, */
			/* so we have to draw the previous line at the top now */
			ew_topline--;
			length = BF_get_line(buffptr, string);
			for (pos=0; pos < length; pos++)
			{
				waddch(edit_win, string[pos]);
			}
			/* */
			wmove(edit_win, cursor_y, cursor_x);
			return;
		}
	}
	/* */
	
	/* any line other than the last one was deleted, so... */
	if (code == DL_OK)
	{
		wdeleteln(edit_win);
		wmove(edit_win, cursor_y, cursor_x);
		
		/* print a line from the buffer to the last line of the edit window if one exists to be put there */
		line_number = BF_get_line_number(buffptr);
		if (BF_goto_line(buffptr, ew_topline + EWH-1) == TRUE)
		{
			wmove(edit_win,EWH-1,0);
			length = BF_get_line(buffptr, string);
			for(pos=0; pos < length; pos++)
			{
				waddch(edit_win, string[pos]);
			}
			BF_goto_line(buffptr, line_number);
			wmove(edit_win, cursor_y, cursor_x);
		}
		/* */
		
		return;
	}
}





/*  EW_update_lines()  -  Update the edit window to reflect the text in it
 *                        that has changed, based on the information passed
 *                        to it.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               All of the values obtained from a call to BF_put_string()
 *
 *  Returns:  none
 *
 *  Notes:  The values returned from BF_put_string() do not need to be
 *          analyzed for any particular values before passing them on to
 *          this function.  It will determine what needs to be done to the
 *          edit window on it's own.
 */
void EW_update_lines(BUFFER *buffptr, int new_cursor_x, int cursor_line, int lines_put, int wrap_pos, bool last_line)
{
	int x;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(new_cursor_x >= 0);
	assert(new_cursor_x < LLEN);
	assert(cursor_line >= 1);
	assert(cursor_line <= lines_put);
	assert(lines_put >= 1);
	assert(wrap_pos >= 1);
	assert(wrap_pos < LLEN);
	/* */
	
	/* go to line cursor is at on screen */
	for (x=1; x < cursor_line; x++)
	{
		BF_move_back(buffptr);
	}
	/* */

	EW_update_position(buffptr);
	
	/* delete line below cursor if last line was joined with the line after it */
	if ((last_line == TRUE) && (cursor_y < EWH-1))
	{
		wmove(edit_win, cursor_y+1, 0);
		wdeleteln(edit_win);
		wmove(edit_win, cursor_y, 0);
	}
	/* */

	/* erase current line after the position at which it is to be wrapped */
	wmove(edit_win, cursor_y, wrap_pos);
	wclrtoeol(edit_win);
	/* */
	
	/* put the lines that were just inserted into the buffer into the edit window now */
	for (x=1; x < lines_put; x++)
	{
		BF_move_ahead(buffptr);
		EW_insert_line(buffptr, LLEN-1);
	}
	/* */
	
	/* reposition cursor */
	for (x=0; x < (lines_put - cursor_line); x++)
	{
		BF_move_back(buffptr);
		cursor_y--;
	}
	cursor_x = new_cursor_x;
	desired_x = new_cursor_x;
}





/*  EW_reposition_cursor()  -  Attempt to redraw the edit window so that
 *                             the cursor will be on the given y coordinate
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               The desired y coordinate of the cursor
 *
 *  Returns:  TRUE if the cursor was able to be positioned on the exact
 *                 line specified
 *            FALSE if the cursor was not able to be positioned exactly
 *                 on the desired line (it will be as close as possible
 *                 though)
 *
 *  Notes:  This function will usually only be called if for some aesthetic
 *          reason the cursor position should be set manually (ie. after a
 *          search it might be nice to have the found word positioned
 *          somewhere near the middle of the window)
 */
bool EW_reposition_cursor(BUFFER *buffptr, int new_cursor_y)
{
	int cur_line, tot_lines, bot_line, top_line;
	int scrollcnt, count, length;
	char string[LLEN];
	int pos;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(new_cursor_y >= 0);
	assert(new_cursor_y <= EWH-1);
	/* */
	
	
	/* is the cursor already on the line specified? */
	if (cursor_y == new_cursor_y)
	{
		return TRUE;
	}
	/* */

	/* can we scroll in either direction at all? */
	if (BF_number_lines(buffptr) < EWH)
	{
		return FALSE;
	}
	/* */

	cur_line = BF_get_line_number(buffptr);
	tot_lines = BF_number_lines(buffptr);
	bot_line = cur_line + (EWH-1 - cursor_y);
	if (bot_line > tot_lines)
	{
		bot_line = tot_lines;
	}
	top_line = cur_line - cursor_y;		/******  <=== CHECK THIS!!!  *****/

	/* is the cursor below the desired position? */
	if (new_cursor_y < cursor_y)
	{
		/* determine just how close we can get to the desired position */
		if ((tot_lines - bot_line) >= (cursor_y - new_cursor_y))
		{
			scrollcnt = cursor_y - new_cursor_y;
		}
		else
		{
			scrollcnt = tot_lines - bot_line;
		}
		/* */
		
		/* scroll that number of lines ahead */
		BF_goto_line(buffptr, bot_line);
		for (count=0; count < scrollcnt; count++)
		{
			scroll(edit_win);
			wmove(edit_win, EWH-1, 0);
			BF_move_ahead(buffptr);
			length = BF_get_line(buffptr, string);
			for (pos=0; pos < length; pos++)
			{
				waddch(edit_win, string[pos]);
			}
/*			BF_move_ahead(buffptr); <=== OLD POSITION */
		}
		/* */
		
		/* reposition cursor */
		BF_goto_line(buffptr, cur_line);
		cursor_y = cursor_y - scrollcnt;
		wmove(edit_win, cursor_y, cursor_x);
		ew_topline = ew_topline + scrollcnt;
		/* */
		
		if (scrollcnt == (cursor_y - new_cursor_y))
		{
			return TRUE;	/* cursor is on the exact line specified */
		}
		else
		{
			return FALSE;	/* cursor is as close as possible to specified line */
		}
	}
	/* */
	
	/* is the cursor above the desired position? */
	if (new_cursor_y > cursor_y)
	{
		/* determine just how close we can get to the desired positoin */
		if (top_line > (new_cursor_y - cursor_y))
		{
			scrollcnt = new_cursor_y - cursor_y;
		}
		else
		{
			scrollcnt = top_line - 1;
		}
		/* */

		/* scroll that number of lines back */		
		BF_goto_line(buffptr, top_line);
		for (count=0; count < scrollcnt; count++)
		{
			wmove(edit_win, 0, 0);
			winsertln(edit_win);
			BF_move_back(buffptr);
			length = BF_get_line(buffptr, string);
			for (pos=0; pos < length; pos++)
			{
				waddch(edit_win, string[pos]);
			}
/*			BF_move_back(buffptr); <===OLD POSITION */
		}
		/* */
		
		/* reposition cursor */
		BF_goto_line(buffptr, cur_line);
		cursor_y = cursor_y + scrollcnt;
		ew_topline = ew_currline - cursor_y;
		wmove(edit_win, cursor_y, cursor_x);
		/* */
		
		if (scrollcnt == (new_cursor_y - cursor_y))
		{
			return TRUE;	/* cursor is on the exact line specified */
		}
		else
		{
			return FALSE;	/* cursor is as close as possible to specified line */
		}
	}
	/* */
}





/*  EW_redraw_window()  -  Redraw the contents of the current edit window from
 *                         scratch.  Useful for block marking module.
 *
 *  Parameters:  a pointer to the main buffer control structure
 *
 *  Returns:  none
 */
void EW_redraw_window(BUFFER *buffptr)
{
	int x, length, pos;
	char string[LLEN];

	/* Assertions */
	assert(buffptr != NULL);
	/* */

	/* erase edit window */
	werase(edit_win);
	wmove(edit_win, 0, 0);
	/* */

	/* go to top line in edit window */
	BF_goto_line(buffptr, ew_topline);
	/* */

	/* draw as much of the edit window as possible from the buffer */
	for (x=0; x < EWH; x++)
	{
		/* get line to draw */
		length = BF_get_line(buffptr, string);
		/* */

		/* draw the line now */
		for (pos=0; pos < length; pos++)
		{
			waddch(edit_win, string[pos]);
		}
		/* */

		/* was this the last line in the buffer? */
		if (BF_move_ahead(buffptr) == FALSE)
		{
			break;
		}
		/* */

		/* prepare to draw the next line */
		if (x < EWH-1)
		{
			waddch(edit_win, '\n');
		}
		/* */
	}
	/* */

	/* reposition in buffer and in window */
	BF_goto_line(buffptr, ew_currline);
	wmove(edit_win, cursor_y, cursor_x);
	/* */
}

