/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_block.c
 */


#include <curses.h>
#include <assert.h>
#include "include/qw_defs.h"
#include "include/qw_termio.h"
#include "include/qw_buffio.h"
#include "include/qw_cursor.h"




/* structure used to store the position of a mark within the buffer */

struct mark {
	int line, pos;
};

/* */


/* function prototypes */

int BL_mark_block(BUFFER *buffptr, BUFFER *clipptr);
int BL_continue_marking(BUFFER *buffptr, BUFFER *clipptr);
int BL_cut_block(BUFFER *buffptr, BUFFER *clipptr);
int BL_copy_block(BUFFER *buffptr, BUFFER *clipptr);
int BL_paste_block(BUFFER *buffptr, BUFFER *clipptr);
int BL_insert_block(BUFFER *buffptr, BUFFER *clipptr);
void draw_block(BUFFER *buffptr);
void unmark_block(BUFFER *buffptr);
void redraw_chars(BUFFER *buffptr, struct mark *mark1, struct mark *mark2, int mode);

/* */


/* global variables */

bool marked_block = FALSE;

extern unsigned long menu_on, menu_off;

/* */

/* local variables */

struct mark first, second;

/* */





/*  BL_mark_block()  -  Handle the marking of a block for later cutting or
 *                      copying (via the available menu options).  Highlighted
 *                      text is drawn in reverse on screen.
 *
 *                      Any of the regularly available cursor movement keys
 *                      in edit mode are also available in blocck marking
 *                      mode, with the exception of the Jump To Line menu
 *                      option.
 *
 *                      To select cut or copy once the desired block is marked,
 *                      swith to menu mode as normal.  To quit marking without
 *                      having to cut or copy a block, hit any key whose
 *                      function is otherwise unspecified.
 *
 *  Parameters:  pointer to the main buffer control structure
 *
 *  Returns:  MENU_MODE if menu mode was selected
 *            EDIT_MODE if block marking was aborted
 */
int BL_mark_block(BUFFER *buffptr, BUFFER *clipptr)
{
	int pos, c, length;
	char string[80];


	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	/* draw the title and status bars */
	TM_write_menubar("\b\r MARK BLOCK                                                                     ");
	TM_write_statusbar("\rUse \bcursor keys\n\r to mark block.  Hit \bCTRL-A\n\r to select from menus.               ");
	/* */
	
	/* set mark positions */
	first.line = BF_get_line_number(buffptr);
	second.line = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&pos, &pos, NULL);
	first.pos = pos;
	second.pos = pos;
	/* */
	
	/* mark this first character */
	EW_redraw_char(buffptr, A_REVERSE);
	EW_refresh();
	/* */
	
	/* handle the user's cursoring around to mark a block */
	for (;;)
	{
		c = getch();
		
		switch (c)
		{
			case KEY_REDRAW:
				/* redraw the screen for user */
				TM_redraw_screen();
				/* */
				break;
			case KEY_RIGHT:
			case KEY_RIGHT2:
				if (CP_move_right(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_LEFT:
			case KEY_LEFT2:
				if (CP_move_left(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_UP:
			case KEY_UP2:
				if (CP_move_up(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_DOWN:
			case KEY_DOWN2:
				if (CP_move_down(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_BLINE:
				if (CP_beginning_line(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_ELINE:
				if (CP_end_line(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_PAGE_UP:
			case KEY_PAGE_UP2:
				if (CP_page_up(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_PAGE_DOWN:
			case KEY_PAGE_DOWN2:
				if (CP_page_down(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_HOME:
			case KEY_HOME2:
				if (CP_top_document(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_END:
			case KEY_END2:
			case KEY_END3:
				if (CP_bottom_document(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_SWITCH:
				marked_block = TRUE;	/* there is a marked block */
				/* set the changes to be made to the menus */
				menu_on = M_CUT | M_COPY;
				menu_off = M_OPEN | M_SAVE | M_SAVE_AS | M_SAVE_CLIPBOARD_AS | M_ABOUT | M_QUIT | M_PASTE | M_INSERT | M_SEARCH;
				menu_off = menu_off | M_SEARCH_NEXT | M_SEARCH_REPLACE | M_JUMP_TO_LINE | M_BUFFER_STATUS;
				/* */
				MH_update_menus();
				return MENU_MODE;
				break;
			default:
				/* set the changes to the menus back to normal */
				menu_on = M_OPEN | M_SAVE | M_SAVE_AS | M_ABOUT | M_QUIT;
				menu_on = menu_on | M_SEARCH | M_SEARCH_NEXT | M_SEARCH_REPLACE;
				menu_on = menu_on | M_JUMP_TO_LINE | M_BUFFER_STATUS;
				length = BF_get_line(clipptr, string);
				if ((BF_number_lines(clipptr) != 1) || ((BF_number_lines(clipptr) != 1) && (length != 0)))
				{
					menu_on = menu_on | M_SAVE_CLIPBOARD_AS | M_PASTE | M_INSERT;
				}
				menu_off = M_CUT | M_COPY;
				MH_update_menus();
				/* */

				/* any other key hit than the above and we abort marking */
				unmark_block(buffptr);	/* reset the display (remove the reverse text) */
				marked_block = FALSE;	/* nothing was marked */
				return EDIT_MODE;
				break;
		}
	}
}





/*  BL_continue_marking()  -  Continues marking after BL_mark_block() has been
 *                            used.  This function is called in the following
 *                            situation:  Block marking mode is entered, a
 *                            switch is made to menu mode, and then a switch
 *                            is made back out of menu mode.  This function
 *                            ensures that the user is brought back to block
 *                            marking mode with the block that was marked
 *                            previously still marked now.
 *
 *  Parameters:  a pointer to the buffer control struture
 *
 *  Returns:  MENU_MODE if the menu mode is selected
 *            EDIT_MODE if block marking is aborted
 */
int BL_continue_marking(BUFFER *buffptr, BUFFER *clipptr)
{
	int c, length;
	char string[80];


	/* Assertions */
	assert(buffptr != NULL);
	/* */

	/* set up the title and status bars */
	TM_write_menubar("\b\r MARK BLOCK                                                                     ");
	TM_write_statusbar("\rUse cursor keys to mark block.  Hit \bCTRL-A\n\r to select from menus.               ");
	EW_refresh();	/* put cursor in edit window */
	/* */
	
	/* all we have to do is pick up exactly where BL_mark_block() left off */

	/* handle the user's cursoring around to mark a block */
	for (;;)
	{
		c = getch();
		
		switch (c)
		{
			case KEY_REDRAW:
				/* redraw the screen for the user */
				TM_redraw_screen();
				/* */
				break;
			case KEY_RIGHT:
			case KEY_RIGHT2:
				if (CP_move_right(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_LEFT:
			case KEY_LEFT2:
				if (CP_move_left(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_UP:
			case KEY_UP2:
				if (CP_move_up(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_DOWN:
			case KEY_DOWN2:
				if (CP_move_down(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();	
				}
				break;
			case KEY_BLINE:
				if (CP_beginning_line(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_ELINE:
				if (CP_end_line(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_PAGE_UP:
			case KEY_PAGE_UP2:
				if (CP_page_up(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_PAGE_DOWN:
			case KEY_PAGE_DOWN2:
				if (CP_page_down(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_HOME:
			case KEY_HOME2:
				if (CP_top_document(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_END:
			case KEY_END2:
			case KEY_END3:
				if (CP_bottom_document(buffptr) != CUR_NO_EFFECT)
				{
					draw_block(buffptr);
				}
				else
				{
					beep();
				}
				break;
			case KEY_SWITCH:
				marked_block = TRUE;	/* there is a marked block */
				/* set the changes to be made to the menus */
				menu_on = M_CUT | M_COPY;
				menu_off = NULL;
				MH_update_menus();
				/* */
				return MENU_MODE;
				break;
			default:
				/* set the changes to the menus back to normal */
				menu_on = M_OPEN | M_SAVE | M_SAVE_AS | M_ABOUT | M_QUIT;
				menu_on = menu_on | M_SEARCH | M_SEARCH_NEXT | M_SEARCH_NEXT;
				menu_on = menu_on | M_JUMP_TO_LINE | M_BUFFER_STATUS;
				length = BF_get_line(clipptr, string);
				if ((BF_number_lines(clipptr) != 1) || ((BF_number_lines(clipptr) == 1) && (length != 0)))
				{
					menu_on = menu_on | M_SAVE_CLIPBOARD_AS | M_PASTE | M_INSERT;
				}
				menu_off = M_CUT | M_COPY;
				MH_update_menus();
				/* */

				/* any other key hit than the above and we abort marking */
				unmark_block(buffptr);	/* reset the display (remove the reverse text) */
				marked_block = FALSE;	/* nothing was marked */
				return EDIT_MODE;
				break;
		}
	}
}





/*  draw_block()  -  This function determines how to optimally redraw the
 *                   new marked area on the screen based on the current
 *                   position of the cursor in relation to it's previous
 *                   position and the first mark.  Actual drawing is then
 *                   done via redraw_chars().
 *
 *  Parameters:  a pointer to the main buffer control structure
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to the Block Manipulation Module
 */
void draw_block(BUFFER *buffptr)
{
	struct mark drawmark1, drawmark2;
	int topline, ypos, pos, dpos;
	char string[LLEN];

	/* Assertions */
	assert(buffptr != NULL);
	/* */

	/* store new mark position */
	second.line = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&pos, &dpos, NULL);
	second.pos = pos;
	/* */

	/* determine from where we need to start highlighting on screen */
	EW_get_cursor_pos(NULL, NULL, &ypos);
	topline = BF_get_line_number(buffptr) - ypos;

	if (first.line < topline)
	{
		drawmark1.line = topline;
		drawmark1.pos = 0;
	}
	else
	{
		if (first.line > topline + EWH-1)
		{
			drawmark1.line = topline + EWH-1;
			BF_goto_line(buffptr, drawmark1.line);
			drawmark1.pos = BF_get_line(buffptr, string);
			BF_goto_line(buffptr, second.line);
		}
		else
		{
			drawmark1.line = first.line;
			drawmark1.pos = first.pos;
		}
	}
	/* */

	/* get the position in the buffer to highlight to */
	drawmark2.line = second.line;
	drawmark2.pos = second.pos;
	/* */

	/* draw this area highlighted on the screen */
	EW_redraw_window(buffptr);
	redraw_chars(buffptr, &drawmark1, &drawmark2, A_REVERSE);
	/* */

	/* put the cursor back */
	BF_goto_line(buffptr, second.line);
	EW_set_cursor_pos(pos, dpos);
	EW_update_position(buffptr);
	/* */

	EW_refresh();
}





/*  unmark_block()  -  Unmark block after a copy, or after block marking mode
 *                     has been aborted.
 *
 *  Parameters:  a pointer to the main buffer control structure
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to this module
 */
void unmark_block(BUFFER *buffptr)
{
	/* clear the previously marked block */
	EW_redraw_window(buffptr);
	/* */
	
	EW_refresh();
}





/*  redraw_chars()  -  Redraw the characters between the two marks specified
 *                     in whatever drawing mode is selected.
 *
 *  Parameters:  a pointer to the main buffer control structure
 *               pointers to the two marks
 *               the drawing mode as specified in <curses.h>
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to this module
 */
void redraw_chars(BUFFER *buffptr, struct mark *mark1, struct mark *mark2, int mode)
{
	struct mark currmark;

	/* determine in which order to draw the block (forwards or backwards) */
	/* This will depend on whether the second mark is before or after the first mark in the buffer */
	if ((mark1->line < mark2->line) || ((mark1->line == mark2->line) && (mark1->pos <= mark2->pos)))
	{
		/* go to first position */
		BF_goto_line(buffptr, mark1->line);
		EW_set_cursor_pos(mark1->pos, mark1->pos);
		EW_update_position(buffptr);
		/* */
		
		/* redraw all chars */
		do
		{
			EW_redraw_char(buffptr, mode);
			CP_move_right(buffptr);

			currmark.line = BF_get_line_number(buffptr);
			EW_get_cursor_pos(&currmark.pos, NULL, NULL);
		}
		while ((currmark.line < mark2->line) || ((currmark.line == mark2->line) && (currmark.pos < mark2->pos)));
		if ((currmark.line == mark2->line) && (currmark.pos == mark2->pos))
		{
			EW_redraw_char(buffptr, mode);
		}
		/* */
	}
	else
	{
		/* go to second position */
		BF_goto_line(buffptr, mark1->line);
		EW_set_cursor_pos(mark1->pos, mark1->pos);
		EW_update_position(buffptr);
		/* */
		
		/* redraw all chars */
		do
		{
			EW_redraw_char(buffptr, mode);
			CP_move_left(buffptr);

			currmark.line = BF_get_line_number(buffptr);
			EW_get_cursor_pos(&currmark.pos, NULL, NULL);
		}
		while ((currmark.line > mark2->line) || ((currmark.line == mark2->line) && (currmark.pos > mark2->pos)));
		EW_redraw_char(buffptr, mode);
		/* */
	}
}





/*  BL_cut_block()  -  Cut the marked block from the main buffer and put it
 *                     into the clipboard buffer.
 *                     This function actually copies the contents of the
 *                     marked area to the clipboard (first erasing its
 *                     previous contents).  It then removes the marked area
 *                     from the main buffer.
 *
 *  Parameters:  a pointer to the main and clipboard buffer control structures
 *
 *  Returns:  EDIT_MODE
 */
int BL_cut_block(BUFFER *buffptr, BUFFER *clipptr)
{
	int temp, length, pos, linelen;
	char line[LLEN];
	bool large = FALSE;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(clipptr != NULL);
	/* */
	
	
	/* empty the clipboard buffer */
	BF_empty_buffer(clipptr);
	/* */
	
	/* swap marks if necessary to get them in the order they appear in the buffer */
	if ((first.line > second.line) || ((first.line == second.line) && (first.pos > second.pos)))
	{
		temp = first.line;
		first.line = second.line;
		second.line = temp;
		temp = first.pos;
		first.pos = second.pos;
		second.pos = temp;
	}
	/* */

	/* if this is a large block to be cut, inform the user that something is happening */
	if (second.line - first.line > 1000)
	{
		TM_write_menubar("\b\r CUT BLOCK                                                                      ");
		TM_write_statusbar("\rPlease wait...                                                                  ");
		large = TRUE;
	}
	/* */
	
	/* copy marked characters to the clipboard buffer */
	BF_goto_line(buffptr, first.line);
	
	if (first.line == second.line)
	{
		/* the marked block reside only on one line */
		linelen = BF_get_line(buffptr, line);
		if (second.pos < linelen)
		{
			BF_put_line(clipptr, line+first.pos, second.pos - first.pos + 1);
		}
		else
		{
			BF_put_line(clipptr, line+first.pos, second.pos - first.pos);
			BF_insert_line_after(clipptr);
		}
		/* */
	}
	else
	{
		/* set the clipboard buffer into loading mode for improved performance */
		BF_initiate_load(clipptr);
		/* */

		/* copy part of the first line to the clipboard */
		length = BF_get_line(buffptr, line);
		BF_load_string(clipptr, line+first.pos, length - first.pos);
		/* */
		
		/* copy all lines between the first and second marks */
		BF_move_ahead(buffptr);
		while (BF_get_line_number(buffptr) < second.line)
		{
			/* do the spinning bar if large block */
			if (large == TRUE)
			{
				if (BF_get_line_number(buffptr) % 300 == 0)
				{
					TM_wait_bar(16);
				}
			}
			/* */

			length = BF_get_line(buffptr, line);
			BF_load_string(clipptr, line, length);
			BF_move_ahead(buffptr);
		}
		/* */
		
		/* copy part of the last marked line to the clipboard */
		linelen = BF_get_line(buffptr, line);
		if (second.pos < linelen)
		{
			BF_load_string(clipptr, line, second.pos + 1);
			BF_load_string(clipptr, "", 0);
		}
		else
		{
			BF_load_string(clipptr, line, second.pos);
		}
		/* */

		/* put clipboard buffer back into regular mode */
		BF_finish_load(clipptr);
		/* */
	}
	/* */
	
	/* delete the marked characters from the buffer and window */
	menu_on = M_SAVE | M_SAVE_AS | M_MARK_BLOCK | M_SEARCH | M_SEARCH_NEXT | M_SEARCH_REPLACE;
	menu_off = NULL;
	MH_update_menus();

	BF_goto_line(buffptr, first.line);
	EW_set_cursor_pos(first.pos, first.pos);
	EW_update_position(buffptr);
	
	if (first.line == second.line)
	{
		/* the entire block to be deleted resides on only one line */
		for (pos = first.pos; pos <= second.pos; pos++)
		{
			TX_delete_key(buffptr);
		}
		/* */
	}
	else
	{
		/* delete all lines between the first and last */
		BF_delete_lines(buffptr, first.line+1, second.line-1);
		EW_redraw_window(buffptr);
		/* */

		/* delete part of the first line */
		if (first.pos == 0)
		{
			TX_delete_line(buffptr);
		}
		else
		{
			length = BF_get_line(buffptr, line);
			for (pos = first.pos; pos < length; pos++)
			{
				TX_delete_key(buffptr);
			}
			CP_move_right(buffptr);	/* move to the next line */
		}
		/* */
		
		/* delete marked characters from last line */
		for (pos=0; pos <= second.pos; pos++)
		{
			TX_delete_key(buffptr);
		}
		/* */

		/* do a backspace now, just to wrap the last line after the cut up */
		if (first.pos > 0)
		{
			TX_backspace_key(buffptr);
		}
		/* */
	}
	/* */
	
	/* set changes to menus */
	menu_on = menu_on | M_OPEN | M_SAVE_CLIPBOARD_AS | M_ABOUT | M_QUIT | M_PASTE | M_INSERT | M_JUMP_TO_LINE | M_BUFFER_STATUS;
	menu_off = M_CUT | M_COPY;
	MH_update_menus();
	/* */

	marked_block = FALSE;
	
	return EDIT_MODE;
}





/*  BL_copy_block()  -  Copy the marked area to the clipboard, erasing its
 *                      previous contents, and then unmark the marked block
 *                      on the screen.
 *
 *  Parameters:  pointers to the main and clipboard buffer control structures
 *
 *  Returns:  EDIT_MODE
 */
int BL_copy_block(BUFFER *buffptr, BUFFER *clipptr)
{
	int temp, length, linenum, pos, dpos, linelen;
	char line[LLEN];
	bool large = FALSE;
		
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(clipptr != NULL);
	/* */
	
	
	/* empty clipboard buffer */
	BF_empty_buffer(clipptr);
	/* */
	
	/* store the cursor position so we can go back to it later */
	linenum = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&pos, &dpos, NULL);
	/* */
	
	/* swap marks if necessary to get them in the order they appear in the buffer */
	if ((first.line > second.line) || ((first.line == second.line) && (first.pos > second.pos)))
	{
		temp = first.line;
		first.line = second.line;
		second.line = temp;
		temp = first.pos;
		first.pos = second.pos;
		second.pos = temp;
	}
	/* */

	/* if this is a large block that is being copied, inform the user that something is happening */
	if (second.line - first.line > 500)
	{
		TM_write_menubar("\r\b COPY BLOCK                                                                     ");
		TM_write_statusbar("\rPlease wait...                                                                  ");
		large = TRUE;
	}
	/* */

	/* copy marked characters to the clipboard buffer */
	BF_goto_line(buffptr, first.line);
	
	if (first.line == second.line)
	{
		/* the marked block reside only on one line */
		/* if it does, we don't really need to take any measures to speed things up */
		linelen = BF_get_line(buffptr, line);
		if (second.pos < linelen)
		{
			BF_put_line(clipptr, line+first.pos, second.pos - first.pos + 1);
		}
		else
		{
			BF_put_line(clipptr, line+first.pos, second.pos - first.pos);
			BF_insert_line_after(clipptr);
		}
		/* */
	}
	else
	{
		/* prepare to copy lines into the clipboard via the quicker 'loading' method */
		BF_initiate_load(clipptr);
		/* */

		/* copy part of the first line to the clipboard */
		length = BF_get_line(buffptr, line);
		BF_load_string(clipptr, line+first.pos, length - first.pos);
		/* */
		
		/* copy all lines between the first and second marks */
		BF_move_ahead(buffptr);
		while (BF_get_line_number(buffptr) < second.line)
		{
			/* if this is a large block, do the spinning bar */
			if (large == TRUE)
			{
				if (BF_get_line_number(buffptr) % 300 == 0)
				{
					TM_wait_bar(16);
				}
			}
			/* */

			length = BF_get_line(buffptr, line);
			BF_load_string(clipptr, line, length);
			BF_move_ahead(buffptr);
		}
		/* */
		
		/* copy part of the last marked line to the clipboard */
		linelen = BF_get_line(buffptr, line);
		if (second.pos < linelen)
		{
			BF_load_string(clipptr, line, second.pos + 1);
		}
		else
		{
			BF_load_string(clipptr, line, second.pos);
			BF_load_string(clipptr, "", 0);
		}
		/* */

		/* put clipboard buffer back into regular mode */
		BF_finish_load(clipptr);
		/* */
	}
	/* */
	
	/* unhighlight the marked text on the window */
	EW_redraw_window(buffptr);
	/* */
	
	/* move the cursor back to it's initial position */
	BF_goto_line(buffptr, linenum);
	EW_set_cursor_pos(pos, dpos);
	EW_update_position(buffptr);
	EW_refresh();
	/* */
	
	/* set the changes to the menus */
	menu_on = M_OPEN | M_SAVE | M_SAVE_AS | M_SAVE_CLIPBOARD_AS | M_ABOUT | M_QUIT | M_PASTE | M_INSERT | M_MARK_BLOCK | M_SEARCH;
	menu_on = menu_on | M_SEARCH_NEXT | M_SEARCH_REPLACE | M_JUMP_TO_LINE | M_BUFFER_STATUS;
	menu_off = M_CUT | M_COPY;
	MH_update_menus();
	/* */

	marked_block = FALSE;
	
	return EDIT_MODE;
}





/*  BL_paste_block()  -  Insert the contents of the clipboard buffer into the
 *                       main buffer at the current cursor position.
 *                       This function will insert carriage returns into the
 *                       main buffer as they exist in the clipboard buffer.
 *
 *  Parameters:  pointers to the main and clipboard buffer control structures
 *
 *  Returns:  EDIT_MODE
 */
int BL_paste_block(BUFFER *buffptr, BUFFER *clipptr)
{
	int endpos, endline;
	int origline, origpos;
	int firstline;
	char line[LLEN];
	int length, x, count;

	/* Assertions */
	assert(buffptr != NULL);
	assert(clipptr != NULL);
	/* */

	/* goto the top of the clipboard buffer */
	BF_goto_start(clipptr);
	/* */

	/* store current position in the buffer */
	origline = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&origpos, NULL, NULL);
	/* */

	/* if clipboard < 3 lines */
	if (BF_number_lines(clipptr) < 3)
	{
		/* put 1st line into main buffer */
		length = BF_get_line(clipptr, line);
		for (x=0; x < length; x++)
		{
			TX_insert_character(buffptr, line[x]);
		}
		if (BF_at_end(clipptr) == FALSE)
		{
			TX_enter_key(buffptr);
		}
		else
		{
			return EDIT_MODE;
		}
		/* */

		/* put 2nd line into the main buffer */
		BF_move_ahead(clipptr);
		length = BF_get_line(clipptr, line);
		for (x=0; x < length; x++)
		{
			TX_insert_character(buffptr, line[x]);
		}
		/* */

		return EDIT_MODE;
	}
	/* */

	/* insert a clipboard of > 2 lines into main buffer */
	else
	{
		/* if the clipboard is large, let the user know what is happening */
		if (BF_number_lines(clipptr) > 200)
		{
			TM_write_menubar("\r\b PASTE BLOCK                                                                    ");
			TM_write_statusbar("\rPlease wait...                                                                  ");
		}
		/* */
		
		/* insert the 1st line into the main buffer */
		length = BF_get_line(clipptr, line);
		for (x=0; x < length; x++)
		{
			TX_insert_character(buffptr, line[x]);
		}
		/* */

		/* store the current line number */
		firstline = BF_get_line_number(buffptr);
		/* */

		/* insert an insert key */
		TX_enter_key(buffptr);
		/* */

		/* insert the last clipboard line into the document */
		BF_goto_end(clipptr);
		length = BF_get_line(clipptr, line);
		for (x=0; x < length; x++)
		{
			TX_insert_character(buffptr, line[x]);
		}
		EW_get_cursor_pos(&endpos, NULL, NULL);
		/* */

		/* put the middle lines straight into the buffer now */
		BF_goto_line(clipptr, 2);
		BF_goto_line(buffptr, firstline);

		count = BF_number_lines(clipptr) - 2;

		for (x=0; x < count; x++)
		{
			/* if this is a large paste, let the user know what is happening */
			if (count > 250)
			{
				if (x % 200 == 0)
				{
					TM_wait_bar(16);
				}
			}
			/* */

			BF_insert_line_after(buffptr);
			length = BF_get_line(clipptr, line);
			BF_put_line(buffptr, line, length);
			BF_move_ahead(clipptr);
		}

		endline = BF_get_line_number(buffptr) + 1;
		/* */

		/* update the window now */
		BF_goto_line(buffptr, origline);
		EW_set_cursor_pos(origpos, origpos);
		EW_redraw_window(buffptr);
		BF_goto_line(buffptr, endline);
		EW_set_cursor_pos(endpos, endpos);
		EW_update_position(buffptr);
		/* */

		return EDIT_MODE;
	}
	/* */
}





/*  BL_insert_block()  -  This function inserts the contents of the
 *                        clipboard buffer into the main buffer at the
 *                        current cursor position.  It will not, however,
 *                        insert any carriage returns that exist in the
 *                        clipboard buffer.  This is done to allow normal
 *                        word-wrapping.
 *
 *  Parameters:  pointers to the main and clipboard buffer control structures
 *
 *  Returns:  EDIT_MODE
 */
int BL_insert_block(BUFFER *buffptr, BUFFER *clipptr)
{
        int pos, length;
        char line[LLEN];
	bool large = FALSE;


        /* Assertions */
        assert(buffptr != NULL);
        assert(clipptr != NULL);
        /* */

        /* go to the top of the clipboard buffer before beginning to paste */
        BF_goto_start(clipptr);
        /* */

	/* if the clipboard is large, let the user know what is happening */
	if (BF_number_lines(clipptr) > 100)
	{
		TM_write_menubar("\r\b INSERT BLOCK                                                                   ");
		TM_write_statusbar("\rPlease wait...                                                                  ");
		large = TRUE;
	}
	/* */

        /* insert a space in front of the cursor to ensure wrapping occurs as expected */
        TX_insert_character(buffptr, ' ');
        CP_move_left(buffptr);
        /* */

        /* insert the contents of the clipboard into the buffer */
        for (;;)
        {
		/* if this is a large clipboard, do the spinning bar */
		if ((large == TRUE) && (BF_get_line_number(clipptr) % 100 == 0))
		{
			TM_wait_bar(16);
		}
		/* */

                /* insert the current line in the clipboard into the main buffer */
                length = BF_get_line(clipptr, line);
                for (pos=0; pos < length; pos++)
                {
                        TX_insert_character(buffptr, line[pos]);
                }
                /* */

                /* insert a carriage return into the main buffer only if there will be another */
                /* line of text from the clipboard buffer to be put into it */
                if (BF_move_ahead(clipptr) != TRUE)
                {
                        break;  /* no more lines in clipboard to inserted so we exit */
                }
                /* */
        }
        /* */

        /* remove the space we initially inserted after the cursor */
        TX_delete_key(buffptr);
        /* */

        return EDIT_MODE;
}

