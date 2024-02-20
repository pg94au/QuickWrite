/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_cursor.c
 */


#include <assert.h>
#include "include/qw_defs.h"
#include "include/qw_buffio.h"
#include "include/qw_ewhand.h"



/* function prototypes */

int CP_move_right(BUFFER *buffptr);
int CP_move_left(BUFFER *buffptr);
int CP_move_up(BUFFER *buffptr);
int CP_move_down(BUFFER *buffptr);
int CP_beginning_line(BUFFER *buffptr);
int CP_end_line(BUFFER *buffptr);
int CP_page_up(BUFFER *buffptr);
int CP_page_down(BUFFER *buffptr);
int CP_top_document(BUFFER *buffptr);
int CP_bottom_document(BUFFER *buffptr);
int CP_jump_to_line(BUFFER *buffptr);

/* */


/* define return values of cursor positioning functions */

#define CUR_OK			0
#define CUR_WRAP		1
#define CUR_NO_EFFECT		2
#define CUR_FAIL		3

/* */





/*  CP_move_right()  -  Move the cursor one character to the right,
 *                      wrapping to the next line if necessary.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        just moved right
 *            CUR_WRAP :      moved to next line
 *            CUR_NO_EFFECT : couldn't move right (no change)
 */
int CP_move_right(BUFFER *buffptr)
{
	int xpos, length;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get line from buffer */
	length = BF_get_line(buffptr, string);
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* don't need to wrap? */
	if (xpos < length)
	{
		EW_set_cursor_pos(xpos+1, xpos+1);
		EW_update_position(buffptr);
		
		return CUR_OK;
	}
	/* */
	
	/* can we wrap? */
	if (BF_move_ahead(buffptr) == TRUE)
	{
		EW_set_cursor_pos(0,0);
		EW_update_position(buffptr);
		
		return CUR_WRAP;
	}
	else
	{
		return CUR_NO_EFFECT;
	}
	/* */
}





/*  CP_move_left()  -  Move the cursor one character to the left,
 *                     wrapping to the previous line if necessary.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        just moved left
 *            CUR_WRAP :      moved to previous line
 *            CUR_NO_EFFECT : couldn't move left (no change)
 */
int CP_move_left(BUFFER *buffptr)
{
	int xpos, length;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* don't need to wrap? */
	if (xpos > 0)
	{
		EW_set_cursor_pos(xpos-1, xpos-1);
		EW_update_position(buffptr);
		
		return CUR_OK;
	}
	/* */
	
	/* can we wrap? */
	if (BF_move_back(buffptr) == TRUE)
	{
		length = BF_get_line(buffptr, string);
		EW_set_cursor_pos(length, length);
		EW_update_position(buffptr);
		
		return CUR_WRAP;
	}
	else
	{
		return CUR_NO_EFFECT;
	}
	/* */
}





/*  CP_move_up()  -  Move the cursor one character up.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : couldn't move up (no change)
 */
int CP_move_up(BUFFER *buffptr)
{
	int xpos, xdesired, length;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* can we go up? */
	if (BF_move_back(buffptr) == FALSE)
	{
		return CUR_NO_EFFECT;
	}
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, &xdesired, NULL);
	/* */
	
	/* get line from buffer */
	length = BF_get_line(buffptr, string);
	/* */
	
	/* determine new x position */
	if (xdesired <= length)
	{
		EW_set_cursor_pos(xdesired, xdesired);
	}
	else
	{
		EW_set_cursor_pos(length, xdesired);
	}
	/* */
	
	EW_update_position(buffptr);
	
	return CUR_OK;
}





/*  CP_move_down()  -  Move the cursor one character down.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : couldn't move down (no change)
 */
int CP_move_down(BUFFER *buffptr)
{
	int xpos, xdesired, length;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* can we go down? */
	if (BF_move_ahead(buffptr) == FALSE)
	{
		return CUR_NO_EFFECT;
	}
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, &xdesired, NULL);
	/* */
	
	/* get line from buffer */
	length = BF_get_line(buffptr, string);
	/* */
	
	/* determine new x position */
	if (xdesired <= length)
	{
		EW_set_cursor_pos(xdesired, xdesired);
	}
	else
	{
		EW_set_cursor_pos(length, xdesired);
	}
	/* */
	
	EW_update_position(buffptr);
	
	return CUR_OK;
}





/*  CP_beginning_line()  -  Move the cursor to the beginning of the current
 *                          line.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : already at beginning of line (no change)
 */
int CP_beginning_line(BUFFER *buffptr)
{
	int xpos;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* need to move cursor? */
	if (xpos > 0)
	{
		EW_set_cursor_pos(0,0);
		EW_update_position(buffptr);
		
		return CUR_OK;
	}
	else
	{
		return CUR_NO_EFFECT;
	}
	/* */
}





/*  CP_end_line()  -  Move the cursor to the end of the current line.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : already at end of line (no change)
 */
int CP_end_line(BUFFER *buffptr)
{
	int xpos, length;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* get line from buffer */
	length = BF_get_line(buffptr, string);
	/* */
	
	/* need to move cursor? */
	if (xpos < length)
	{
		EW_set_cursor_pos(length, length);
		EW_update_position(buffptr);
		
		return CUR_OK;
	}
	else
	{
		return CUR_NO_EFFECT;
	}
	/* */
}





/*  CP_page_up()  -  Move the cursor one window full of text if the cursor
 *                   is at the top of the window.  Otherwise simply move
 *                   the cursor to the top of the edit window.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : couldn't move up (no change)
 */
int CP_page_up(BUFFER *buffptr)
{
	int ypos, count;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* can we go up at all? */
	if (BF_at_start(buffptr) == TRUE)
	{
		return CUR_NO_EFFECT;
	}
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(NULL, NULL, &ypos);
	/* */
	
	/* do we just need to put the cursor to the top of the window? */
	if (ypos > 0)
	{
		for (count=0; count < ypos; count++)
		{
			CP_move_up(buffptr);
		}
		EW_update_position(buffptr);
		
		return CUR_OK;
	}
	/* */
	
	/* go up one windowful (minus one line), or until we hit the top of the buffer */
	for (count=0; count < EWH-1; count++)
	{
		CP_move_up(buffptr);	/* if we hit the top this function just won't have any effect */
	}
	EW_update_position(buffptr);
	/* */
	
	return CUR_OK;
}





/*  CP_page_down()  -  Move the cursor one window full of text if the
 *                     cursor is at the bottom of the window.  Otherwise
 *                     simply move the cursor to the bottom of the edit
 *                     window.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : couldn't move down (no change)
 */
int CP_page_down(BUFFER *buffptr)
{
	int ypos, count;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* can we go down at all? */
	if (BF_at_end(buffptr) == TRUE)
	{
		return CUR_NO_EFFECT;
	}
	/* */
	
	/* get cursor position */
	EW_get_cursor_pos(NULL, NULL, &ypos);
	/* */
	
	/* do we just need to put the cursor to the bottom of the window? */
	if (ypos < EWH-1)
	{
		for (count=ypos; count < EWH-1; count++)
		{
			CP_move_down(buffptr);
		}
		EW_update_position(buffptr);
		
		return CUR_OK;
	}
	/* */
	
	/* go down one windowful (minus one line), or until we hit the bottom */
	for (count=0; count < EWH-1; count++)
	{
		CP_move_down(buffptr);	/* if we hit bottom this function just won't have any effect */
	}
	EW_update_position(buffptr);
	/* */
	
	return CUR_OK;
}





/*  CP_top_document()  -  Move the cursor to the top of the document.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : already at top (no change)
 */
int CP_top_document(BUFFER *buffptr)
{
	int xpos;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* at top of buffer already? */
	if ((BF_at_start(buffptr) == TRUE) && (xpos == 0))
	{
		return CUR_NO_EFFECT;
	}
	/* */
	
	/* go to top of buffer */
	BF_goto_start(buffptr);
	/* */
	
	/* set cursor x position */
	EW_set_cursor_pos(0,0);
	/* */
	
	EW_update_position(buffptr);
	
	return CUR_OK;
}





/*  CP_bottom_document()  -  Move the cursor to the bottom of the document.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  CUR_OK :        no problem
 *            CUR_NO_EFFECT : already at bottom (no change)
 */
int CP_bottom_document(BUFFER *buffptr)
{
	int xpos, length;
	char string[LLEN];
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* get cursor position */
	EW_get_cursor_pos(&xpos, NULL, NULL);
	/* */
	
	/* at bottom of buffer already? */
	if ((BF_at_end(buffptr) == TRUE) && (xpos == BF_get_line(buffptr, string)))
	{
		return CUR_NO_EFFECT;
	}
	/* */
	
	/* go to end of buffer */
	BF_goto_end(buffptr);
	/* */

	/* get line from buffer */
	length = BF_get_line(buffptr, string);
	/* */
	
	/* set cursor position */
	EW_set_cursor_pos(length, length);
	/* */
	
	EW_update_position(buffptr);
	
	return CUR_OK;
}





/*  CP_jump_to_line()  -  Move the cursor to the specified line.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               The line in the buffer to move the cursor to
 *
 *  Returns:  EDIT_MODE
 */
int CP_jump_to_line(BUFFER *buffptr)
{
	int line;
	char string[LLEN];


	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	/* draw title bar and prompt */
	TM_write_menubar("\r\b JUMP TO LINE                                                                   ");
	TM_write_statusbar("\rJump to Line:");
	/* */

	/* get line to jump to from user */
	TM_read_value(14, 9, &line);
	/* */
	
	/* obviously illegal line? */
	if (line < 1)
	{
		TM_write_statusbar("\b\rLine out of range!                                                              ");
		getch();

		return EDIT_MODE;
	}
	/* */
	
	/* already at this line? */
	if (BF_get_line_number(buffptr) == line)
	{
		sprintf(string,"\b\rAlready at line %-9d                                                      ",line);
		TM_write_statusbar(string);
		getch();

		return EDIT_MODE;
	}
	/* */
	
	/* go to line if it exists */
	if (BF_goto_line(buffptr, line) == FALSE)
	{
		TM_write_statusbar("\b\rLine out of range!                                                              ");
		getch();

		return EDIT_MODE;
	}
	/* */
	
	/* set cursor_position */
	EW_set_cursor_pos(0,0);
	EW_update_position(buffptr);
	EW_reposition_cursor(buffptr, EWH/2-1);
	EW_update_position(buffptr);
	/* */
	
	return EDIT_MODE;
}
