/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_menu.c
 */



#include <curses.h>
#include "include/qw_defs.h"
#include "include/qw_buffio.h"
#include "include/qw_cursor.h"
#include "include/qw_info.h"
#include "include/qw_block.h"
#include "include/qw_search.h"
#include "include/qw_termio.h"
#include "include/qw_ewhand.h"
#include "include/qw_error.h"


/* function prototypes */

int MH_menu_handler(void);
int select_menu(void);
WINDOW *draw_menu(int currheading);
void draw_menu_headings(void);
void MH_update_menus(void);
void make_option_available(int option);
void make_option_unavailable(int option);

/* */


/* define structure for menu contents and information */

struct menustruct {
	int left,right;		/* which to go to next */
	bool avail;		/* is this menu available now */
	int availcount;		/* number of options currently available */
	int xpos;		/* the xpos where this menu starts */
	int width;		/* width of the longest menu option */
	int numoptions;		/* the number of options under this heading */
	char *name;		/* pointer to name of menu heading */
	char *options[10];	/* pointers to menu option names */
	bool optavail[10];	/* which of these options are available */
	int return_code[10];	/* code to return if option selected */
};

/* */


/* define constants used in the menu handler */

#define MENU_SELECT	200
#define HEADINGS	201
#define OPTIONS		202

/* */


extern WINDOW *menubar_win;

extern BUFFER *buffptr, *clipptr;

unsigned long menu_on = 0, menu_off = 0;

extern bool marked_block;


#include "include/qw_menustruct.h"	/* information on menu structure, etc. */





/*  MH_menu_handler  -  Calls menu_select which returns the option selected.
 *                   This function then executes that function and returns
 *                   to edit mode (after possibly more than 1 function is
 *                   called).
 *
 *  Parameters:  none
 *
 *  Returns:  EDIT_MODE - after a selection has been made and executed
 *            QUIT      - if user chooses so
 */
int MH_menu_handler()
{
	/* start off in the internal menu select mode */
	int mode = MENU_SELECT;
	/* */

	/* update the menus if any changes have been made to them */
	MH_update_menus();

	menu_on = 0;
	menu_off = 0;
	/* */

	/* print all menu heading in the title bar & write status bar */
	draw_menu_headings();
	TM_write_statusbar("\rUse \bcursor keys\n\r to navigate menus, or hit \bCTRL-A\n\r to return to edit mode");
	/* */


	/* run in a continuous loop until a value returned by a user selection is finally found */
	for(;;)
	{
		/* check the current mode in order to determine how to deal with it */
		switch(mode)
		{
			/* all the possible mode values, and how to deal with them */
			case MENU_SELECT:
				mode = select_menu();
				break;
			case OPEN:
				mode = FL_open_file(buffptr);
				break;
			case SAVE:
				mode = FL_save_file(buffptr);
				break;
			case SAVE_AS:
				mode = FL_save_file_as(buffptr);
				break;
			case SAVE_CLIPBOARD_AS:
				mode = FL_save_clipboard_as(clipptr);
				break;
			case ABOUT:
				mode = IF_about();
				break;
			case CUT:
				mode = BL_cut_block(buffptr, clipptr);
				break;
			case COPY:
				mode = BL_copy_block(buffptr, clipptr);
				break;
			case PASTE:
				mode = BL_paste_block(buffptr, clipptr);
				break;
			case INSERT:
				mode = BL_insert_block(buffptr, clipptr);
				break;
			case MARK_BLOCK:
				/* make sure to check marked_block */
				if (marked_block == FALSE)
				{
					mode = BL_mark_block(buffptr, clipptr);
				}
				else
				{
					mode = BL_continue_marking(buffptr, clipptr);
				}
				break;
			case SEARCH:
				mode = SR_search(buffptr);
				break;
			case SEARCH_NEXT:
				mode = SR_search_next(buffptr);
				break;
			case SEARCH_AND_REPLACE:
				mode = SR_search_replace(buffptr);
				break;
			case DISPLAY_LINE_NUMBER:
				mode = IF_display_line_number(buffptr);
				break;
			case JUMP_TO_LINE:
				mode = CP_jump_to_line(buffptr);
				break;
			case BUFFER_STATUS:
				mode = IF_buffer_status(buffptr, clipptr);
				break;
			case HELP_ME:
				mode = IF_help();
				break;
			case EDIT_MODE:
				return EDIT_MODE;
				break;
			case MENU_MODE:
				return MENU_MODE;
				break;
			case SWITCH_BACK:
				/* check if we were previously marking */
				if (marked_block == TRUE)
				{
					mode = MARK_BLOCK;
				}
				else
				{
					mode = EDIT_MODE;
				}
				/* */
				break;
			case QUIT:
				if (FL_quit(buffptr) == QUIT)
				{
					return QUIT;
				}
				mode = EDIT_MODE;
				break;
			/* */
		}
		/* */
	}
	/* */
}





/*  select_menu()  -  Select the menu that you wish to see, then hit DOWN to
 *                    select from it.  This function handles both the
 *                    selection of a menu heading and the selection of an
 *                    option from that menu.
 *
 *  Parameters:  none
 *
 *  Returns:  the code corresponding to the option selected from the menu,
 *            or EDIT_MODE if menu mode was terminated without making a
 *            selection.
 *
 *  Notes:  This function is internal to the Menu Handler module.
 */
int select_menu()
{
	WINDOW *menu_win;
	int currheading, prevheading;
	int currselected, prevselected;
	int c;
	int mode = HEADINGS;	/* start off in headings as opp. OPTIONS */


	/* find the first available menu heading */
	currheading = 0;
	while (menus[currheading].avail == FALSE)
		currheading++;	/* >= one heading must be available! */
	prevheading = currheading;
	/* */

	/* stay in the loop until a selection is finally made */
	/* within the loop the state can change between selecting a menu heading and selecting a menu option */
	for(;;)
	{
		/* select heading in this loop, until a menu is selected, then get user to select an option from it */
		do
		{
			/* unhighlight previous heading */
			if (currheading != prevheading)
			{
				wmove(menubar_win,0,menus[prevheading].xpos);
				wattrset(menubar_win,A_STANDOUT|A_BOLD);
				waddstr(menubar_win,menus[prevheading].name);
				wattrset(menubar_win,A_NORMAL);
			}
			/* */
         
			/* highlight current heading */
			wmove(menubar_win,0,menus[currheading].xpos);
			wattrset(menubar_win,A_BOLD);
			waddstr(menubar_win,menus[currheading].name);
			wattrset(menubar_win,A_NORMAL);
			hide_cursor();
			wrefresh(menubar_win);
			/* */

			/* get an input from the user to determine what to do next */
			c = getch();
			switch(c)
			{
				case KEY_REDRAW:
					/* redraw the screen for the user */
					TM_redraw_screen();
					/* */
					break;
				case KEY_LEFT:
					/* go to the menu heading to the left (the next 'available' menu heading) */
					prevheading = currheading;
					do
						currheading = menus[currheading].left;
					while (menus[currheading].avail != TRUE);
					/* */
					break;
				case KEY_RIGHT:
					/* go to the menu heading to the right (the next 'available' menu heading) */
					prevheading = currheading;
					do
						currheading = menus[currheading].right;
					while (menus[currheading].avail != TRUE);
					/* */
					break;
				case KEY_DOWN:
				case KEY_ENTER:
				case KEY_RETURN:
					/* switch from menu heading selection to menu option selection */
					prevheading = currheading;
					mode = OPTIONS;
					/* */
					break;
				case KEY_SWITCH:
					/* just return to edit mode, user decided to break out without making a selection */
					return SWITCH_BACK;
					/* */
					break;
			}
			/* */
		}
		while (mode == HEADINGS);
		/* */

		/* draw window for menu on screen */
		menu_win = draw_menu(currheading);
		if (menu_win == NULL)
		{
			return SWITCH_BACK;
		}
		/* */

		/* find first available option (not all options are available) */
		currselected = 0;
		while (menus[currheading].optavail[currselected] == FALSE)
			currselected++;
		prevselected = currselected;
		/* */

		/* select options from the menu selected */
		do
		{
			/* unhighlight the previously selected menu option */
			wmove(menu_win,prevselected+1,1);
			wattrset(menu_win,A_BOLD);
			waddstr(menu_win,menus[currheading].options[prevselected]);
			wattrset(menu_win,A_NORMAL);
			/* */
         
			/* highlight the currently selected menu option */
			wmove(menu_win,currselected+1, 1);
			wattrset(menu_win,A_STANDOUT|A_BOLD);
			waddstr(menu_win,menus[currheading].options[currselected]);
			wattrset(menu_win,A_NORMAL);
			hide_cursor();
			wrefresh(menu_win);
			/* */

			/* get an input from the user to determine what to do next */
			c = getch();
			switch(c)
			{
				case KEY_REDRAW:
					/* redraw the screen for the user */
					TM_redraw_screen();
					/* */
					break;
				case KEY_LEFT:
					/* break out of menu option selection and go to the menu heading to the left */
					delwin(menu_win);
					EW_touch();
					EW_refresh();
					do
						currheading = menus[currheading].left;
					while (menus[currheading].avail != TRUE);
					mode = HEADINGS;
					/* */
					break;
				case KEY_RIGHT:
					/* break out of menu option selection and go to the menu heading to the right */
					delwin(menu_win);
					EW_touch();
					EW_refresh();
					do
						currheading = menus[currheading].right;
					while (menus[currheading].avail != TRUE);
					mode = HEADINGS;
					/* */
					break;
				case KEY_DOWN:
					/* go to the next menu option below (skipping unavailable ones) */
					prevselected = currselected;
					do
						currselected++;
					while ((menus[currheading].optavail[currselected] == FALSE) && (currselected < menus[currheading].numoptions));
					if (currselected == menus[currheading].numoptions) /* check if user was at last menu option */
						currselected = prevselected;
					/* */
					break;
				case KEY_UP:
					/* go to the previous menu option, or back to heading mode if user hits top */
					prevselected = currselected;
					do
						currselected--;
					while ((menus[currheading].optavail[currselected] == FALSE) && (currselected > -1));
					if (currselected == -1)
					{
						delwin(menu_win);
						EW_touch();
						EW_refresh();
						mode = HEADINGS;
					}
					/* */
					break;
				case KEY_SWITCH:
					/* return to edit mode since user has decided not to make a selection */
					delwin(menu_win);
					EW_touch();
					EW_refresh();
					return SWITCH_BACK;
					/* */
					break;
				case KEY_ENTER:
				case KEY_RETURN:
					/* return the code corresponding to the selection just made by the user */
					delwin(menu_win);
					EW_touch();
					EW_refresh();
					return menus[currheading].return_code[currselected];
					/* */
					break;
			}
		}
		while (mode == OPTIONS);
		/* */
	}
	/* */
}





/*  draw_menu()  -  open the window for current menu to be drawn in, and
 *                  print all of the options into it
 *
 *  Parameters:  none
 *
 *  Returns:  a pointer to the window that was opened for the menu
 *
 *  Notes:  This function is internal to the Menu Handler module.
 */
WINDOW *draw_menu(int currheading)
{
	WINDOW *menu_win;
	int x;


	/* open the window for the menu options */
	menu_win = newwin(menus[currheading].numoptions + 2,
				menus[currheading].width + 2,
				1, menus[currheading].xpos);
	if (menu_win == NULL)
	{
	ER_put_error(WINDOW_ERROR);
	return NULL;
	}
	/* */

	/* put a box around the edge of this window just opened */
	wattrset(menu_win, A_BOLD);
	box(menu_win, '|', '-');
	wattrset(menu_win, A_NORMAL);
	/* */

	/* print all menu options into this box */
	for (x=0; x < menus[currheading].numoptions; x++)
	{
		wmove(menu_win, x+1, 1);
		if (menus[currheading].optavail[x] == TRUE)
			wattrset(menu_win, A_BOLD);
		else
			wattrset(menu_win, A_NORMAL);
		waddstr(menu_win, menus[currheading].options[x]);
	}
	wattrset(menu_win, A_NORMAL);
	/* */

	wrefresh(menu_win);

	return menu_win;
}





/*  draw_menu_headings()  -  Draw the main menu headings in the menu bar on
 *                           top of the screen, unhighlighted
 *
 *  Parameters:  none
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to the Menu Handler module.
 */
void draw_menu_headings()
{
	int x;


	/* draw the titlebar with separators only */
	wmove(menubar_win,0,0);
	wattrset(menubar_win, A_BOLD|A_REVERSE);
	waddstr(menubar_win, "|      |      |        |       |      |                                         ");
	wattrset(menubar_win, A_NORMAL);
	/* */

	/* print the menu headings in the titlebar */
	for(x=0; x<5; x++)
	{
		wmove(menubar_win, 0,menus[x].xpos);
		if (menus[x].avail == TRUE)
			wattrset(menubar_win, A_BOLD|A_REVERSE);
		else
			wattrset(menubar_win, A_DIM|A_REVERSE);
		waddstr(menubar_win, menus[x].name);
		wattrset(menubar_win, A_NORMAL);
	}
	/* */

	wrefresh(menubar_win);
}





/*  update_menus()  -  Update the window options with respect to options
 *                     that should now be available/unavailable, based on
 *                     the contents of menu_on and menu_off, as set by the
 *                     rest of the editor.
 *
 *  Parameters:  none
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to the Menu Handler module.
 */
void MH_update_menus()
{
	/* make selected options available */
	if (menu_on & M_OPEN)
	{
		make_option_available(OPEN);
	}
	if (menu_on & M_SAVE)
	{
		make_option_available(SAVE);
	}
	if (menu_on & M_SAVE_AS)
	{
		make_option_available(SAVE_AS);
	}
	if (menu_on & M_SAVE_CLIPBOARD_AS)
	{
		make_option_available(SAVE_CLIPBOARD_AS);
	}
	if (menu_on & M_ABOUT)
	{
		make_option_available(ABOUT);
	}
	if (menu_on & M_QUIT)
	{
		make_option_available(QUIT);
	}
	if (menu_on & M_CUT)
	{
		make_option_available(CUT);
	}
	if (menu_on & M_COPY)
	{
		make_option_available(COPY);
	}
	if (menu_on & M_PASTE)
	{
		make_option_available(PASTE);
	}
	if (menu_on & M_INSERT)
	{
		make_option_available(INSERT);
	}
	if (menu_on & M_MARK_BLOCK)
	{
		make_option_available(MARK_BLOCK);
	}
	if (menu_on & M_SEARCH)
	{
		make_option_available(SEARCH);
	}
	if (menu_on & M_SEARCH_NEXT)
	{
		make_option_available(SEARCH_NEXT);
	}
	if (menu_on & M_SEARCH_REPLACE)
	{
		make_option_available(SEARCH_AND_REPLACE);
	}
	if (menu_on & M_DISPLAY_LINE_NUMBER)
	{
		make_option_available(DISPLAY_LINE_NUMBER);
	}
	if (menu_on & M_JUMP_TO_LINE)
	{
		make_option_available(JUMP_TO_LINE);
	}
	if (menu_on & M_BUFFER_STATUS)
	{
		make_option_available(BUFFER_STATUS);
	}
	if (menu_on & M_HELP)
	{
		make_option_available(HELP_ME);
	}
	/* */

	/* make selected options unavailable */
	if (menu_off & M_OPEN)
	{
		make_option_unavailable(OPEN);
	}
	if (menu_off & M_SAVE)
	{
		make_option_unavailable(SAVE);
	}
	if (menu_off & M_SAVE_AS)
	{
		make_option_unavailable(SAVE_AS);
	}
	if (menu_off & M_SAVE_CLIPBOARD_AS)
	{
		make_option_unavailable(SAVE_CLIPBOARD_AS);
	}
	if (menu_off & M_ABOUT)
	{
		make_option_unavailable(ABOUT);
	}
	if (menu_off & M_QUIT)
	{
		make_option_unavailable(QUIT);
	}
	if (menu_off & M_CUT)
	{
		make_option_unavailable(CUT);
	}
	if (menu_off & M_COPY)
	{
		make_option_unavailable(COPY);
	}
	if (menu_off & M_PASTE)
	{
		make_option_unavailable(PASTE);
	}
	if (menu_off & M_INSERT)
	{
		make_option_unavailable(INSERT);
	}
	if (menu_off & M_MARK_BLOCK)
	{
		make_option_unavailable(MARK_BLOCK);
	}
	if (menu_off & M_SEARCH)
	{
		make_option_unavailable(SEARCH);
	}
	if (menu_off & M_SEARCH_NEXT)
	{
		make_option_unavailable(SEARCH_NEXT);
	}
	if (menu_off & M_SEARCH_REPLACE)
	{
		make_option_unavailable(SEARCH_AND_REPLACE);
	}
	if (menu_off & M_DISPLAY_LINE_NUMBER)
	{
		make_option_unavailable(DISPLAY_LINE_NUMBER);
	}
	if (menu_off & M_JUMP_TO_LINE)
	{
		make_option_unavailable(JUMP_TO_LINE);
	}
	if (menu_off & M_BUFFER_STATUS)
	{
		make_option_unavailable(BUFFER_STATUS);
	}
	if (menu_off & M_HELP)
	{
		make_option_unavailable(HELP_ME);
	}
	/* */
}





/*  make_option_available()  -  allow menu option corresponding to the return
 *                              code passed to be available for user selection
 *
 *  Parameters:  The code corresponding the the menu option which is to be
 *               enabled.  This code is identical to the code which would
 *               be returned if the option were selected by the user from
 *               the menus.
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to the Menu Handler module.
 */
void make_option_available(int option)
{
	bool found=FALSE;
	int heading, selection;

	/* find the location of this option (since we're not given it's position in the */
	/* data structure, but only it's corresponding return code */
	heading = 0;
	do
	{
		selection = 0;
		do
		{
			if (menus[heading].return_code[selection] == option)
				found = TRUE;
			else
				selection++;
		}
		while ((found == FALSE) &&
			(selection <= menus[heading].numoptions));
		if (found == FALSE)
			heading++;
	}
	while (found == FALSE);
	/* */

	/* increment count of available menu options if necessary */
	if (menus[heading].optavail[selection] == FALSE)
		menus[heading].availcount++;
	/* */
   
	/* make this option available now */
	menus[heading].optavail[selection] = TRUE;
	menus[heading].avail = TRUE;
	/* */
}





/*  make_option_unavailable()  -  make option passed by corresponding return code
 *                                unavailable with respect to menu selection
 *
 *  Parameters:  The code corresponding the the menu option which is to be
 *               enabled.  This code is identical to the code which would
 *               be returned if the option were selected by the user from
 *               the menus.
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to the Menu Handler module.
 */
void make_option_unavailable(int option)
{
	bool found=FALSE;
	int heading, selection;

	/* find the location of this option (since we're not given it's position in the */
	/* data structure, but only it's corresponding return code */
	heading = 0;
	do
	{
		selection = 0;
		do
		{
			if (menus[heading].return_code[selection] == option)
				found = TRUE;
			else
				selection++;
		}
		while ((found == FALSE) &&
			(selection <= menus[heading].numoptions));
		if (found == FALSE)
			heading++;
	}
	while (found == FALSE);
	/* */

	/* decrement count of available options if necessary */
	if (menus[heading].optavail[selection] == TRUE)
		menus[heading].availcount--;
	/* */
   
	/* make this option unavailable */
	menus[heading].optavail[selection] = FALSE;
	/* */
   
	/* make the corresponding menu heading unavailable if none of it's options are available */
	if (menus[heading].availcount == 0)
		menus[heading].avail = FALSE;
	/* */
}
