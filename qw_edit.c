/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_edit.c
 */



#include <curses.h>
#include "include/qw_defs.h"
#include "include/qw_termio.h"
#include "include/qw_cursor.h"
#include "include/qw_text.h"
#include "include/qw_fileio.h"



extern BUFFER *buffptr;



/*  function prototypes */

int EH_edit_handler(void);

/* */





/*  EH_edit_handler()  -  Handle user input while in the edit mode of the
 *                        program.  Call all appropriate functions based on
 *                        user input.  Switch to menu selection mode once
 *                        the user has selected the proper key sequence to
 *                        do so.
 *
 *  Parameters:  none
 *
 *  Returns:  MENU_MODE when the user chooses to switch from edit mode
 *                 to edit mode
 */
int EH_edit_handler()
{
	int c;	/* holds the character read in from the user */
	char report[80],filename[65];
	
	
	/* set up the display upon entering edit mode */
	TM_write_menubar("\r\bQuickWrite - (c)1993,1994 Insync Development                                    ");
	TM_write_statusbar("\rHit \bCTRL-A\n\r to select from the menu options");
	EW_refresh();
	/* */
	
	/* handle user input while in edit mode */
	for (;;)
	{
		c = getch();	/* get character from user */
		
		/* determine what to do based one user's input */
		switch(c)
		{
			case KEY_SWITCH:
				/* user wishes to go to menu mode */
				return MENU_MODE;
				/* */
				break;
			case KEY_REDRAW:
				/* redraw the entire screen for the user */
				TM_redraw_screen();
				/* */
				break;
			case KEY_FILENAME:
				/* print the current filename to the status bar */
				if (FL_get_filename(filename) == TRUE)
				{
					sprintf(report, "\r\"%s\"",filename);
					TM_write_statusbar(report);
				}
				else
				{
					TM_write_statusbar("\rNo filename specified.");
				}
				getch();
				TM_write_statusbar("\rHit \bCTRL-A\n\r to select from the menu options.");
				EW_refresh();
				/* */
				break;
			case KEY_LEFT:
			case KEY_LEFT2:
				/* cursor left */
				if (CP_move_left(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* couldn't move left */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_RIGHT:
			case KEY_RIGHT2:
				/* cursor right */
				if (CP_move_right(buffptr) == CUR_NO_EFFECT)
				{
					beep(); /* couldn't move right */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_UP:
			case KEY_UP2:
				/* cursor up */
				if (CP_move_up(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* couldn't move up */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_DOWN:
			case KEY_DOWN2:
				/* cursor down */
				if (CP_move_down(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* couldn't move down */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_PAGE_UP:
			case KEY_PAGE_UP2:
				/* cursor page up */
				if (CP_page_up(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* couldn't page up */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_PAGE_DOWN:
			case KEY_PAGE_DOWN2:
				/* cursor page down */
				if (CP_page_down(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* couldn't page down */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_HOME:
			case KEY_HOME2:
				/* top of document (HOME key) */
				if (CP_top_document(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* already at top */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_END2:
			case KEY_END3:
				/* bottom of document (END key) */
				if (CP_bottom_document(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* already at end */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_BLINE:
				/* cursor to beginning of line */
				if (CP_beginning_line(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* already at beginning of line */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_ELINE:
				/* cursor to end of line */
				if (CP_end_line(buffptr) == CUR_NO_EFFECT)
				{
					beep();	/* already at end of line */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_BACKSPACE:
			case KEY_BACKSPACE2:
				/* delete previous character */
				if (TX_backspace_key(buffptr) == FALSE)
				{
					beep();	/* no character to erase */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_DELETE:
			case KEY_DELETE2:
				/* delete current character */
				if (TX_delete_key(buffptr) == FALSE)
				{
					beep();	/* no character to erase */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_ENTER:
			case KEY_RETURN:
				/* insert a carriage return into document */
				if (TX_enter_key(buffptr) == FALSE)
				{
					beep();	/* couldn't add new line! */
					beep();
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_TAB:
				/* insert spaces to next tab stop */
				if (TX_tab_key(buffptr) == FALSE)
				{
					beep();	/* couldn't add new line (if necessary)! */
					beep();
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			case KEY_DELETE_LINE:
				/* delete current line from document */
				if (TX_delete_line(buffptr) == FALSE)
				{
					beep();	/* no line to delete */
				}
				else
				{
					EW_refresh();
				}
				/* */
				break;
			default:
				/* was the key read in printable or not? */
				if ((c >= 32) && (c <= 126))
				{
					if (TX_insert_character(buffptr, c) == FALSE)
					{
						beep();	/* couldn't add character! */
						beep();
					}
					else
					{
						EW_refresh();
					}
				}
				else
				{
					beep();	/* unrecognized character entered */
				}
				/* */
				break;
		}
	}
}

