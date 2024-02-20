/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_text.c
 */


#include <assert.h>
#include "include/qw_defs.h"
#include "include/qw_buffio.h"
#include "include/qw_cursor.h"
#include "include/qw_menu.h"

#define TABLEN	8

/*extern bool buffer_modified;*/
extern unsigned long menu_on, menu_off;
extern bool buffer_modified;


/* function prototypes */

bool TX_insert_character(BUFFER *buffptr, char c);
bool TX_delete_key(BUFFER *buffptr);
bool TX_backspace_key(BUFFER *buffptr);
bool TX_enter_key(BUFFER *buffptr);
bool TX_delete_line(BUFFER *buffptr);
bool TX_tab_key(BUFFER *buffptr);

/* */





/*  TX_insert_character()  -  Insert a character into the document,
 *                            updating the buffer and the edit window.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               The character to be inserted
 *
 *  Returns:  TRUE if successful
 *            FALSE if unsuccessful (couldn't allocate more buffer space)
 */
bool TX_insert_character(BUFFER *buffptr, char c)
{
	int xpos, length, pos, cursor_line, lines_put, wrap_pos, cursor_x;
	char string[LLEN+1];
	bool last_line;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get line from buffer */
	length = BF_get_line(buffptr, string);
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* check if this is the first character put into an empty buffer */
	if ((BF_number_lines(buffptr) == 1) && (length == 0))
	{
		menu_on = menu_on | M_SAVE | M_SAVE_AS | M_MARK_BLOCK | M_SEARCH | M_SEARCH_REPLACE | M_SEARCH_NEXT;
		menu_off = NULL;
		MH_update_menus();
	}
	/* */
	
	/* insert the character into the array of characters */
	for (pos = length-1; pos >= xpos; pos--)
	{
		string[pos+1] = string[pos];
	}
	string[xpos] = c;
	length++;
	/* */
	
	cursor_x = xpos + 1;	/* new cursor x position */

	
	/* put string back into buffer */
	if (BF_put_string(buffptr, string, length, &cursor_x, &cursor_line, &lines_put, &wrap_pos, &last_line) == FALSE)
	{
		return FALSE;
	}
	/* */
	
	/* update edit window */
	if (xpos < LLEN-1)	/* insert the character first if it's on the current line */
	{
		EW_insert_char(c);
	}
	if (lines_put > 1)	/* need to do more than just insert the one character */
	{
		EW_update_lines(buffptr, cursor_x, cursor_line, lines_put, wrap_pos, last_line);
	}
	/* */
	
	/* set cursor position */
	EW_set_cursor_pos(cursor_x, cursor_x);
	/* */

	/* update new cursor position */
	EW_update_position(buffptr);
	/* */
	
	/* buffer has just been modified */
	buffer_modified = TRUE;
	/* */
	
	return TRUE;
}





/*  TX_delete_key()  -  Delete the character at the current location of the
 *                      cursor, both in the buffer and on the screen.
 *                      Deletes the current line if it was previously
 *                      empty and it is not the only line in the buffer.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if successful
 *            FALSE if unsuccessful (buffer empty)
 */
bool TX_delete_key(BUFFER *buffptr)
{
	int xpos, length, pos, nextlength, linenumber;
	char string[2*LLEN], nextline[LLEN];
	int cur_y, cur_x;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get line number */
	linenumber = BF_get_line_number(buffptr);
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* get current line */
	length = BF_get_line(buffptr, string);
	/* */
	
	/* can we just simply delete a character without having to bring the next line up? */
	if (xpos < length)
	{
		/* delete the character from the array of characters */
		for (pos = xpos; pos < length - 1; pos++)
		{
			string[pos] = string[pos+1];
		}
		length--;
		/* */
		BF_put_line(buffptr, string, length);
		EW_delete_char();
		
		/* check if buffer empty and set menu options accordingly */
		length = BF_get_line(buffptr, string);
		if ((BF_number_lines(buffptr) == 1) && (length == 0))
		{
			menu_on = NULL;
			menu_off = M_SAVE | M_SAVE_AS | M_MARK_BLOCK | M_SEARCH | M_SEARCH_REPLACE | M_SEARCH_NEXT;
			MH_update_menus();
		}
		/* */

		buffer_modified = TRUE;
		
		return TRUE;
	}
	/* */
	
	/* need to wrap previous line up */
	if (BF_at_end(buffptr) == TRUE)	/* at end of buffer, can't delete */
	{
		return FALSE;
	}
	/* */
	
	/* get next line then delete it */
	BF_move_ahead(buffptr);
	nextlength = BF_get_line(buffptr, nextline);
	EW_get_cursor_pos(NULL, NULL, &cur_y);
	EW_update_position(buffptr);
	TX_delete_line(buffptr);
	EW_update_position(buffptr);
	/* */
	
	/* go back to original line */
	BF_goto_line(buffptr, linenumber);
	EW_update_position(buffptr);
	if (cur_y == EWH-1)
	{
		EW_reposition_cursor(buffptr, EWH-1);
		EW_update_position(buffptr);
	}
	/* */
	
	/* add next line to original line */
	EW_set_cursor_pos(length, length);
	EW_update_position(buffptr);
	for (pos=0; pos < nextlength; pos++)
	{
		TX_insert_character(buffptr, nextline[pos]);
	}
	/* */

	/* reset cursor position */
	for (pos=0; pos < nextlength; pos++)
	{
		EW_get_cursor_pos(&cur_x, NULL, NULL);
		if (cur_x == 0)
		{
			CP_move_left(buffptr);
		}

		CP_move_left(buffptr);
	}
	/* */

	/* check if buffer empty now */
	length = BF_get_line(buffptr, string);
	if ((BF_number_lines(buffptr) == 1) && (length == 0))
	{
		menu_on = NULL;
		menu_off = M_SAVE | M_SAVE_AS | M_MARK_BLOCK | M_SEARCH | M_SEARCH_REPLACE | M_SEARCH_NEXT;
		MH_update_menus();
	}
	/* */
	
	buffer_modified = TRUE;
	
	return TRUE;
}





/*  TX_backspace_key()  -  Delete the character to the left of the cursor,
 *                         both in the buffer and on the screen.
 *                         Bring current line up to previous line if a
 *                         backspace is done from the first cursor x
 *                         position on a line.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if successful
 *            FALSE if unsuccessful (buffer empty)
 */
bool TX_backspace_key(BUFFER *buffptr)
{
	int origline, origcur_y, origcur_x;
	int afterline, aftercur_y, aftercur_x;

	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	/* get the original position of the cursor on screen & in the buffer */
	origline = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&origcur_x, NULL, &origcur_y);
	/* */

	/* can we backspace at all? */
	if (CP_move_left(buffptr) == CUR_NO_EFFECT)
	{
		return FALSE;
	}
	/* */
	
	TX_delete_key(buffptr);

	/* get the position of the cursor on screen & in the buffer after the backspace */
	afterline = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&aftercur_x, NULL, &aftercur_y);
	/* */

	/* see if anything was done, if not, then do the fix */
	if ((origline == afterline) && (origcur_y == aftercur_y) && (origcur_x == aftercur_x))
	{
		CP_move_left(buffptr);
		CP_move_left(buffptr);
		TX_delete_key(buffptr);
		TX_delete_key(buffptr);
	}
	/* */
	
	return TRUE;
}





/*  TX_enter_key()  -  Perform the operation of the carriage return.
 *                     This involves inserting a new line into the buffer,
 *                     moving all characters after the cursor to the next
 *                     line, and then updating the screen.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if successful
 *            FALSE if unsuccessful (couldn't allocate the new line)
 */
bool TX_enter_key(BUFFER *buffptr)
{
	int xpos, length, newlength = 0, pos;
	char line[LLEN], newline[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* check if the buffer was previously empty */
	length = BF_get_line(buffptr, line);
	if ((BF_number_lines(buffptr) == 1) && (length == 0))
	{
		menu_on = M_SAVE | M_SAVE_AS | M_MARK_BLOCK | M_SEARCH | M_SEARCH_REPLACE | M_SEARCH_NEXT;
		menu_off = NULL;
		MH_update_menus();
	}
	/* */

	/* buffer has been modified */
	buffer_modified = TRUE;
	/* */

	/* insert the new line into the buffer */
	if (BF_insert_line_after(buffptr) == FALSE)
	{
		return FALSE;
	}
	BF_move_back(buffptr);
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* get line from buffer */
	length = BF_get_line(buffptr, line);
	/* */
	
	/* copy portion of current line to next if necessary (enter key hit when in the middle of the line) */
	if (xpos < length)
	{
		for (pos = xpos; pos < length; pos++)
		{
			newline[newlength++] = line[pos];
		}
		length = xpos;
		
		BF_put_line(buffptr, line, length);	/* chop the current line in the buffer where the cursor was */
	}
	/* */
	
	/* put what was after the cursor into the newly inserted line */
	BF_move_ahead(buffptr);
	BF_put_line(buffptr, newline, newlength);
	/* */
	
	/* update edit window */
	EW_insert_line(buffptr, xpos);
	/* */
	
	return TRUE;
}





/*  TX_delete_line()  -  Delete the current line from the buffer as well as
 *                       on the screen.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if successful
 *            FALSE if unsuccessful (buffer empty)
 */
bool TX_delete_line(BUFFER *buffptr)
{
	int code, length;
	char line[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* delete line from the buffer */
	code = BF_delete_line(buffptr);
	/* */
	
	/* was the buffer empty already? */
	if (code == DL_EMPTY)
	{
		return FALSE;
	}
	/* */
	
	/* update screen */
	EW_delete_line(buffptr, code);
	/* */

	/* check if buffer is now empty */
	length = BF_get_line(buffptr, line);
	if ((BF_number_lines(buffptr) == 1) && (length == 0))
	{
		menu_on = NULL;
		menu_off = M_SAVE | M_SAVE_AS | M_MARK_BLOCK | M_SEARCH | M_SEARCH_REPLACE | M_SEARCH_NEXT;
		MH_update_menus();
	}
	/* */

	buffer_modified = TRUE;
	
	return TRUE;
}





/*  TX_tab_key()  -  Insert spaces into the document until the next tab
 *                   stop is reached.  If past the last tab stop on the
 *                   current line, simply perform a carriage return.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if successful
 *            FALSE if unsuccessful (couldn't allocate new line)
 */
bool TX_tab_key(BUFFER *buffptr)
{
	int xpos, count;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get cursor x position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* cannot tab further on this line?  then just go to the next line */
	if (xpos > LLEN-TABLEN-1)
	{
		return (TX_enter_key(buffptr));
	}
	/* */
	
	/* insert spaces until the next tab stop */
	if (xpos % TABLEN == 0)
	{
		TX_insert_character(buffptr, ' ');
		xpos++;
	}

	for (count = xpos; (count % TABLEN != 0); count++)
	{
		TX_insert_character(buffptr, ' ');
	}
	/* */
	
	return TRUE;
}

