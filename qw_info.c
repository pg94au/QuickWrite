/* QuickWrite                      */
/* (c)1993,1994 Insync Development */
/*                                 */
/* qw_info.c                       */




#include <curses.h>
#include <assert.h>
#include "include/qw_defs.h"
#include "include/qw_buffio.h"
#include "include/qw_error.h" 
#include "include/qw_termio.h"
#include "include/qw_helpstruct.h"

/* function prototypes */
int IF_display_line_number(BUFFER *buffptr);
int IF_about(void);
int IF_help(void);
int IF_buffer_status(BUFFER *buffptr, BUFFER *clipptr);
/* */

/* global variables */
extern bool buffer_modified;
/* */

/* local function prototypes */
int print_indexs(int);
void handle_keys(void);
void print_keys(char *start);
char *print_help(char *start);
int update_highlight(int, int);
/* */

/* Local Variables  
 *
 * index_background_win, index_win, index_border_win - These will create the 
 *		windows that the index for help will use.
 * help_background_win, help_win, help_border_win - These will create the 
 *		windows that the help display will use.
 * index_title - this holds the message to be printed on the title bar of 
 *		index_background_win.
 * index_status1 - this will hold the 1st line of the message to be printed on
 *		the statusbar of index_background_win.
 * index_status2 - this will hold the 2nd line of the message to be printed on
 *		the statusvar of index_background_win.
 * help_title - this will hold the help topic to be printed on the title bar of
 *		index_background_win.
 * help_status - the message to printed to the statusbar of help_background_win.
 * help_clear - this will be used to clear a line in the help_win.
 * index_clear - this will be used to clear a line in the index_win.
 * low - this will hold a value which will tell me where to start printing the
 *		indexs onto the index_win.
 * stop - this when set to 0 will allow for pages to be changed in the help.  
 * 		When set to 1 the page cannot be forwarded.
 * out - When set to 0, the mode will be set to HELP.  When set to 1, the mode
 *		to be entered is EDIT_MODE.
 */

WINDOW *index_background_win, *index_win, *index_border_win;
WINDOW *help_background_win, *help_win, *help_border_win;

char *index_title = "Help Index                                                  ";
char *index_status1 = "  Cursor Keys to select help. Enter gets help information.  ";
char *index_status2 = "  <Space/b> Forward/Back on page. <q>/<CTRL-A> quits help.  "; 
char *help_title;
char *help_status = "      <Space/b> Forward/Back one page. <q> quits back into index table.      ";
char *help_clear = "                                                                          ";
char *index_clear = "                                                            ";
int low=0,stop=0,out=0;





/*  IF_display_line_number() - This function will display the current line that the cursor is 
 *                             positioned on .
 *
 *  Parameters:  A pointer to the corresponding buffer control structure.
 *
 *  Returns:  The next mode that the editor will be in, in this case it will be in edit mode next.
 */

int IF_display_line_number(BUFFER *buffptr)
{
		int current_line_number;	/* The current line number that the cursor is on */
		char response[80];		/* To store the contents of the message to be displayed to
						   the user */

		/* Assertions */
		assert(buffptr != NULL);
		/* */

		/* print a heading on the menu bar */
		TM_write_menubar("\r\b DISPLAY LINE NUMBER                                                            ");
		/* */

		/* Obtains the current line number that the cursor is on */
		current_line_number = BF_get_line_number(buffptr);
 		/* */

		/* Storing the response a character string */
		sprintf(response, "\rThe current line number is %d.  Hit a key to continue.", current_line_number);
 		/* */

		/* Write the character string to the status bar window */
		TM_write_statusbar(response);
		/* */

		/* wait for user to enter a key */
		while (getch() == KEY_REDRAW)
		{
			TM_redraw_screen();
		}
		/* */

		return SWITCH_BACK;
}





/*  IF_about() - This function will display to the user product information such as a release date and 
 *        who developed the editor.
 *
 *  Parameters:  None
 *
 *  Returns:  The next mode that the editor will be in, in this case it will be in edit mode next.
 */

int IF_about()
{

	WINDOW *about_win;	/* a pointer to the about window */
	
	/* Draw About window if possible and print in the appropriate information */
	about_win = newwin(13,38,(LINES/2)-7,(COLS/2)-19);
	if (about_win != NULL) 
	{
		TM_write_menubar("\b\r ABOUT QUICKWRITE                                                               ");
		wattrset(about_win, A_BOLD | A_REVERSE);

		/* Enter in the appropriate information into the window */
		waddstr(about_win,"                                     \n");
		waddstr(about_win,"    QUICKWRITE SCREEN EDITOR V1.0    \n");
		waddstr(about_win,"                                     \n");
		waddstr(about_win,"   (c)1993,1994 Insync Development   \n");
		waddstr(about_win,"                                     \n");
		waddstr(about_win,"  Insync Development Team:           \n");
		waddstr(about_win,"                                     \n");
		waddstr(about_win,"  Paul Grebenc      Peter Vrhovsek   \n");
		waddstr(about_win,"  Dave Kaczanowski  Paul Koski       \n");
		waddstr(about_win,"  Tom Quiring       Deodat Persaud   \n");
		waddstr(about_win,"  Dave Gratton      Sam Elsie        \n");
		waddstr(about_win,"                                     ");
		/* */

		/* Refresh the screen to display the contents of the About window */
		wrefresh(about_win);
		/* */

                TM_write_statusbar("\rHit any key to return to the editor                                             ");

		/* Wait until user presses any key, then remove the About window from the screen */
		while (getch() == KEY_REDRAW)
		{
			TM_redraw_screen();
		}
		delwin(about_win);
		EW_touch();
		EW_refresh();
		/* */

	}
	else
	{
		/* Unable to create new window and passing error to Error Handler module */
		ER_put_error(ERROR_ABOUT);
		/* */
	}
	/* */

	/* Return the next mode the editor is to be in, Edit mode to the Menu handler */
	return EDIT_MODE;
	/* */
}





/*  IF_buffer_status() - This function will display to the user, information about the main and clipboard buffers,
 *                       such as the number of lines, the number of words, the number of bytes and whether the 
 *                       main buffer has been modified since the last save.
 *
 *  Parameters:  A pointer to the two corresponding buffer control 
 *               structures.
 *  
 *  Returns:  The next mode that the editor will be in, in this case it
 *            will be in edit mode next. 
 */

int IF_buffer_status(BUFFER *buffptr, BUFFER *clipptr)
{
	int buff_word_ct=0;	/* the word count in the main buffer */
	int buff_line_ct=0;	/* the line count in the main buffer */
	int buff_byte_ct=0;	/* the byte count in the main buffer */
	int clip_line_ct=0;	/* the line count in the clipboard buffer */
	int clip_byte_ct=0;	/* the byte count in the clipboard buffer */
	int count;		/* used in the for loop to go through a line */
	int ini_line;		/* the initial line that the buffer (main or clipboard) was on */
	int line_length;	/* the length of the line that is passed from the Buffer I/O module */
	char line[80];		/* used to store the contents of the current line in the buffer (main or clipboard) */
	WINDOW *bs_win;		/* a pointer to the buffer status window */

	/* Assertions */
	assert(buffptr != NULL);
	assert(clipptr != NULL);
	/* */

	/* Get the current line number that the buffer is in */
	ini_line = BF_get_line_number(buffptr);
	/* */

	/* Go to the beginning of the buffer if it is not already positioned there */
	BF_goto_start(buffptr);
	/* */

	/* Get the number of lines in the main buffer */
	buff_line_ct = BF_number_lines(buffptr);
	if ( ( buff_line_ct == 1 ) && ( buffptr->lines[0].length == 0 ) )
	{
		buff_line_ct = 0;
	}
	/* */

	/* If the number of lines in the buffer is greater than 100, then display a message to the user to wait */
	if (buff_line_ct > 100)
	{
		TM_write_statusbar("\rPlease wait...                                                                  ");
	}
	/* */

	/* If the buffer is not empty */
	if (buff_line_ct != 0)
	{
		/** Go through the entire buffer and collect the appropriate information **/
		do
		{
			/* do the spinning bar here */
			if (BF_get_line_number(buffptr) % 150 == 0)
			{
				TM_wait_bar(16);
			}
			/* */

			line_length = BF_get_line(buffptr,line);
			buff_byte_ct = buff_byte_ct + line_length + 1;
			count = 0;
			/*** Go through the entire line using the line length,to get the number of words in a line ***/
			while (count < line_length) 
			{
		    		if (isspace(buffptr->lines[buffptr->current_line].string[count]) == 0)
				{
                               		buff_word_ct++; 
					/**** find the end of the word, that is the next space ****/
					while (isspace(buffptr->lines[buffptr->current_line].string[count]) == 0)
					{
						
						count++; 
					}
					/**** ****/
				}
                        	else
                       		{
                        		count++;
                       		}
			}
			/*** ***/
		}
		while (BF_move_ahead(buffptr));
		/** **/
	}
	/* */

	/* Set the current position in the buffer to it's original position */
	BF_goto_line(buffptr,ini_line);
	/* */	

	/* Get the number of lines in the clipboard buffer */
	clip_line_ct = BF_number_lines(clipptr);
	if ( ( clip_line_ct == 1 ) && ( clipptr->lines[0].length == 0 ) )
	{
		clip_line_ct = 0;
	}
	/* */

	/* If the number of lines in the clipboard buffer is greater than 100, then display a message to the user to wait */
	if (clip_line_ct > 100)
	{
		TM_write_statusbar("\rPlease wait...                                                                  ");
	}
	/* */
	
	/* Go to the beginning of the clipboard buffer */
	BF_goto_start(clipptr);
	/* */
	
	/* Get the number of bytes in the clipboard buffer if it is not empty */
	if ( clip_line_ct != 0)
	{
		ini_line = BF_get_line_number(clipptr);
		do
		{
			/* do the spinning bar here */
			if (BF_get_line_number(clipptr) % 200 == 0)
			{
				TM_wait_bar(16);
			}
			/* */

			line_length = BF_get_line(clipptr,line);
			clip_byte_ct = clip_byte_ct + line_length + 1
;
		}
		while (BF_move_ahead(clipptr));
	}
	/* */
	else
	{
		/* The clipboard buffer is empty */
		clip_byte_ct = 0;
		/* */
	}

	/* Set the current position in the clipboard buffer to it's orginal position */
	BF_goto_line(clipptr,ini_line);
	/* */

	/*Draw buffer status window if possible and print in the data collected.*/
	bs_win=newwin(15,32,(LINES/2)-7,(COLS/2)-16);
	if (bs_win != NULL) 
	{
		TM_write_menubar("\b\r BUFFER STATUS                                                                  ");
		TM_write_statusbar("\rHit any key to return to the editor                                             ");

		wattrset(bs_win, A_BOLD);
		box(bs_win,'|','-');
		wmove(bs_win,1,6);
		waddstr(bs_win,"STATUS OF THE BUFFERS");
		wmove(bs_win,3,2);
		waddstr(bs_win,"BUFFER");
		wmove(bs_win,4,2);
		waddstr(bs_win,"------");
		wattrset(bs_win, A_NORMAL);
		wmove(bs_win,5,8);
		wprintw(bs_win,"Word Count = %d",buff_word_ct);
		wmove(bs_win,6,8);
		wprintw(bs_win,"Line Count = %d",buff_line_ct);
		wmove(bs_win,7,8);
		wprintw(bs_win,"Byte Count = %d",buff_byte_ct);
		wmove(bs_win,8,8);

		/** Check to see the status of the buffer and print the appropriate message **/
		if (buffer_modified) 
		{
			waddstr(bs_win,"Modified ?  YES");
		}	
		else
		{
			waddstr(bs_win,"Modified ?  NO");
		}
		/** **/

		wattrset(bs_win, A_BOLD);
		wmove(bs_win,10,2);
		waddstr(bs_win,"CLIPBOARD");
		wmove(bs_win,11,2);
		waddstr(bs_win,"---------");
		wmove(bs_win,12,8);
		wattrset(bs_win, A_NORMAL);

		/** Check to see if the clipboard buffer has changed and display appropriate message **/
		if (clip_line_ct == 0) 
		{
			waddstr(bs_win,"Clipboard is Empty");
		}	
		else
		{
			wprintw(bs_win,"Line Count = %d", clip_line_ct);
			wmove(bs_win,13,8);
			wprintw(bs_win,"Byte Count = %d", clip_byte_ct);
		}
		/** **/
		
		

		/** Refresh the screen to display the contents of the buffer status window **/
		wrefresh(bs_win);
		/** **/

		/** Wait until user presses any key then remove buffer status window **/
		hide_cursor();
		while (getch() == KEY_REDRAW)
		{
			TM_redraw_screen();
		}
		delwin(bs_win);
		EW_touch();
		EW_refresh();
		/** **/
	}
	else
	{
        	/** Unable to create new window and passing error to Error handler module **/
		ER_put_error(ERROR_BUFFSTAT_WIN);
		/** **/
	}
	/* */

	/* Return the next mode the editor is to be in, Edit mode to the Menu handler */
	return EDIT_MODE;
	/* */
}





/*  IF_help() - This will draw the initial contents of the index window and it will call and other
 * 		required functions within itself.
 *
 *  Parameters:  None
 *  
 *  Returns:  The next mode to be entered
 */
int IF_help()
{
	/* This will open up a window for the border */ 
	index_border_win=newwin(15,62,(LINES/2)-8,(COLS/2)-31);
	/* */

	/* This will see if the the pointers are valid */
	if (index_border_win == NULL) 
	{
		ER_put_error(WINDOW_ERROR);  /* Send error messae */

	 	/* This will restore the edit window to the state it was before help was required */
		EW_touch();
		EW_refresh(); 
		/* */

		return SWITCH_BACK;
	}
	
	/* This will open up a window for the background */
	index_background_win= newwin(13,60,(LINES/2)-7, (COLS/2)-30);
	/* */

	if (index_background_win == NULL)
	{
	 	ER_put_error(WINDOW_ERROR);  /* Send error message */

	  	delwin(index_border_win); /* This will delete the border window */

		/* This will restore the edit window to the state it was before help was required */
		EW_touch();
		EW_refresh(); 
		/* */

		return SWITCH_BACK;
	}

	/* This will open up a window for where the text is being displayed */
	index_win=newwin(10,60,(LINES/2)-6,(COLS/2)-30);
	/* */

	if (index_win == NULL)
	{
	 	ER_put_error(WINDOW_ERROR);  /* Send error message */

		delwin(index_border_win); /* This will delete the border window */
		delwin(index_background_win); /* This will delete the background window */

		/* This will restore the edit window to the state it was before help was required */
		EW_touch();
		EW_refresh(); 
		/* */

		return SWITCH_BACK;
	}

	/* Draw a box around the window pointed to by *border_win */
	wattrset(index_border_win, A_BOLD);
	box(index_border_win, '|', '-');
	wattrset(index_border_win, A_NORMAL);
	/* */

	/* This will print the title bar and starus line on window pointed by *index_background_win */
	wmove(index_background_win,0,0);
	wattrset(index_background_win,A_REVERSE|A_BOLD);
	waddstr(index_background_win,index_title);
	wmove(index_background_win,11,0);
	wattrset(index_background_win,A_REVERSE);
	waddstr(index_background_win,"      Cursor Keys to select topic.  Then press <");
	wattrset(index_background_win,A_REVERSE|A_BOLD);
	waddstr(index_background_win,"Enter");
	wattrset(index_background_win,A_REVERSE);
	waddstr(index_background_win,">      ");
	wmove(index_background_win,12,0);
	waddstr(index_background_win,"   <");
	wattrset(index_background_win,A_REVERSE|A_BOLD);
	waddstr(index_background_win,"Space/b");
	wattrset(index_background_win,A_REVERSE);
	waddstr(index_background_win,"> Forward/Back one page <");
	wattrset(index_background_win,A_REVERSE|A_BOLD);
	waddstr(index_background_win,"q/CTRL-A");
	wattrset(index_background_win,A_REVERSE);
	waddstr(index_background_win,"> quits help.   ");  
	/* */

	/* This will put a message up onto the menubar */
	TM_write_menubar("\r\b HELP                                                                           ");  
	/* */

	/* This will blank out the statusbar */
	TM_write_statusbar("\r Please make a selection for a topic in which you need help.                     ");
	/* */

	handle_keys();  /* Accept input from the user */

	/* This will delete the 3 windows that were opened up */
	delwin(index_border_win);
	delwin(index_win);
	delwin(index_background_win);  
	/* */

	/* This will restore the edit window to the state it was before help was required */
	EW_touch();
	EW_refresh();  
	/* */

	/* This will put the editor back into edit mode */
	return SWITCH_BACK;
	/* */
}





/*  print_indexs() - This will print out the appropriate part of the index table onto the index window.
 *
 *  Parameters:  This requires a number which tells the function what index is to be highlighted.
 *		 It also uses the global variable low, which tells the function where to start in the
 *		 index table.
 *
 *  Returns:  the position where the highlighted bar is
 */
int print_indexs(int where)
{
	int x,high; /* x is for general use; high is for the index of a topic located at the bottom of the
		       screen */

	/* This will cause the index screen to wrap to the end of the index table when you go beyond the
	   last index */
	if (low>=MAX)
		low=0;
	/* */

	/* This will cause the index screen to wrap to the end of the index table when you go beyond the
	   first index */	
	if (low<0)
		low = (MAX) - (MAX % 10);
	/* */

	/* This will cause the index screen to wrap to the beginning when you are moving the cursor beyond
	   the last index */
	if (where>=MAX)
	{
		low=0;
		where=0;
	}
	/* */

	/* This will cause the index screen to wrap to the end when you are moving the cursor before the
	   first index */
	if (where<0)
		where=MAX-1;
	/* */

	/* This will check to see if a full screen of indexs can be displayed */
	if ((low+9)>MAX)
		high=MAX-1;
	else
		high=low+9;
	/* */

	/* This will blank out the entire window before writting to it */
	for (x=0; x<=9; x++)
	{
		wmove(index_win,x,0);
		wprintw(index_win,index_clear);
	}
	/* */

	/* This will print the indexs on the screen */
	for (x=low;x<=high;x++)
	{
		wmove(index_win,x%10,0);
		wprintw(index_win,"%2d) %s",x+1,index[x]); 
	} 
	/* */

	/* This will highlight the first item of the index list */
	wmove(index_win,where%10,4);
	wstandout(index_win);
	wprintw(index_win,"%s",index[where]);
	wstandend(index_win); 
	wrefresh(index_border_win);
	wrefresh(index_background_win);  
	wrefresh(index_win); 
	/* */

	/* This will return what line the current cursor is on for selection */
	return where;
	/* */
}	





/*  handle_keys() - This will handle the key inputs from the user to navigate through the index table.
 *
 *  Parameters:  None
 * 
 *  Returns:  None
 */
void handle_keys(void)
{

	int prev,where,option,flag=0;

	/*  prev us used to hold the position where the cursor was before it was moved
	 *  where is used to hold the current position of where the cursor is
 	 *  option is used to hold the input
 	 *  flag is used to exit help when set to 1
 	 */

	prev=where=flag;

	/* This will print the help index and update the current cursor position */
	where=print_indexs(where);
	prev=where;
	/* */

	for(;;)
	{
		option=getch();

		switch(option)
		{
			case KEY_REDRAW:
				/* redraw the screen for the user */
				TM_redraw_screen();
				/* */
				break;
			case KEY_SWITCH:
			case 'q':
			case 'Q':
				low=0;
				flag=1;
				break;
			case ' ':
				/* Let where and low point to the next screen */
				where=low+=10;
				/* */

				where=print_indexs(where);

				/* Let prev be equal to the top of the screen */
				prev=where;
				/* */

				break;
			case 'b':
			case 'B':
				/* Let where and low point to the previous screen */
				where=low-=10;
				/* */

				where=print_indexs(where);

				/* Let prev be equal to the top of the screen */
				prev=where;
				/* */

				break;
			case KEY_DOWN:
				/* Let where point to the next item */
				where++; 
				/* */

				/* Check to see if we need to go to the next screen */
				if (((where-low)>9)||(where>=MAX))
				{
					/* We need to go to the next screen */
					low+=10;
					where=print_indexs(where);
					prev=where;
					/* */

				}
				else
					/* We do not need to go to the next screen just update the
					   highlight position */
					prev=update_highlight(where,prev);
					/* */

				/* */

				break;
			case KEY_UP:
				/* Let where point to the previous item */
				where--;
				/* */

				/* Check to see if we need to go to the previous screen */
				if (((where-low)<0)||(where<0))
				{
					/* We need to go to the previous screen */
					low-=10;
					where=print_indexs(where);
					prev=where;
					/* */

				}
				else
					/* We do not need to go to the previous screen just update the
					   highlight position */
					prev=update_highlight(where,prev);
					/* */

				/* */

				break;
			case KEY_RETURN:
			case KEY_ENTER:
				/* This will opein up a window for the border */
				help_border_win= newwin(22,78,(LINES/2)-11,(COLS/2)-39);
				/* */

				/* Let help_title have the appropriate subject heading in it. */
				help_title=index[where];
				/* */

				/* This will see if the pointers are valid */
				if (help_border_win == NULL)
				{
					ER_put_error(WINDOW_ERROR);  /* Send error code */

					/* This will restore the eit window to the state it was before help
				   	was required */
					EW_touch();
				   	EW_refresh(); 
					/* */

					return;
				}

				/* This will opein up a window for the background */
				help_background_win= newwin(20,76,(LINES/2)-10,(COLS/2)-38);
				/* */

				if (help_background_win == NULL)
				{
					ER_put_error(WINDOW_ERROR);  /* Send error code */

					delwin(help_border_win); /* This will delete the border window */

					/* This will restore the edit window to the state it was before help
					   was required */
					EW_touch();
					EW_refresh(); 
					/* */

					return; 
				}
	
				/* This will opein up a window for where the text is to be displayed */
				help_win= newwin(18,76,(LINES/2)-9,(COLS/2)-38);
				/* */

				if (help_win == NULL)
				{
				 	ER_put_error(WINDOW_ERROR);  /* Send error code */

					delwin(help_border_win); /* This will delete the border window */
					delwin(help_background_win); /* This will delete the background
									window */

					/* This will restore the edit wondow to the state it was before help
					   was required */
					EW_touch();
					EW_refresh(); 
					/* */

					return; 
				}
				/* */

				/* Draw a box around the window pointed to by *index_border_win */
				wattrset(help_border_win, A_BOLD);
				box(help_border_win, '|', '-');
				wattrset(help_border_win, A_NORMAL);
				/* */

				/* This will print the title bar and status line on the window pointed to
				   by *help_background_win */
				wmove(help_background_win, 0, 0);
				wattrset(help_background_win,A_BOLD|A_REVERSE);
				waddstr(help_background_win, help_title);
				waddstr(help_background_win,"                    "); 
				wmove(help_background_win, 19, 0);
				wattrset(help_background_win,A_REVERSE);
				waddstr(help_background_win,"     <");
				wattrset(help_background_win,A_BOLD|A_REVERSE);
				waddstr(help_background_win,"Space/b");
				wattrset(help_background_win,A_REVERSE);
				waddstr(help_background_win,"> Forward/Back one page.  <");
				wattrset(help_background_win,A_BOLD|A_REVERSE);
				waddstr(help_background_win,"q");
				wattrset(help_background_win,A_REVERSE);
				waddstr(help_background_win,"> quits back into index table.     ");
				/* */

				/* This will refresh the windows so that my changes made can be seen */
				wrefresh(help_border_win);
				wrefresh(help_background_win); 
				/* */

				/* This will accept input from the user to look through the help document */
				print_keys(text[where]);
				/* */

				/* if this is true, then get back into EDIT_MODE */
				if (out==1)
				{
					/* delete the appropriate windows */
					delwin(help_border_win);
					delwin(help_background_win);
					delwin(help_win);
					delwin(index_border_win);
					delwin(index_background_win);
					delwin(index_win);
					/* */

					/* reset all variables that control  what happens in help*/
					out=0;
					flag=1;
					low=0;
					/* */

					break;
				}
				/* */

				/* Write to the statusbar from the edit window */
				TM_write_statusbar("\r Please make a selection for a topic in which you need help.                     ");
				/* */
		
				/* This will restore the edit window and index window to the state it was
				   before the help document was put on the screen */
				delwin(help_border_win);
				delwin(help_background_win);
				delwin(help_win);
				EW_touch();
				EW_refresh();
				touchwin(index_border_win);
				wrefresh(index_border_win);
				touchwin(index_background_win);
				wrefresh(index_background_win);
				touchwin(index_win);
				wrefresh(index_win);
				/* */

				break;
		}

		/* This will check to see if we can exit help */
		if (flag)
			/* Yes we can exit */
			break;
			/* */
		/* */

	}
}





/*  update_highligh() - This will unhighlight a line in the index table while highlighting another line
 *			in the index table.
 *
 *  Parameters:  where- This is the position that you want highlighted in the index table.
 *		 prev-  This is the position that you want unhighlighted in the index table.
 *
 *  Returns:  This will return the position of the line that is highlighted
 */
int update_highlight(int where, int prev)
{
	/* This will unhighlight the previous position */
	wmove(index_win,prev%10,4);
	wprintw(index_win,"%s",index[prev]);
	/* */

	/* This will highlight the position pointed to by where */
	wmove(index_win,where%10,4);
	wstandout(index_win);
	wprintw(index_win,"%s",index[where]);
	wstandend(index_win);
	wmove(index_background_win,0,59);
	wrefresh(index_win);
	wrefresh(index_background_win); 
	/* */

	/* This will return the postion of where the cursor is */
	return where;
	/* */
}





/*  print_keys() - This will handle the keys from the user when they are viewing the help document.
 *
 *  Parameters:  Requires a pointer which points to the help document to be displayed.
 *
 *  Returns:  None
 */
void print_keys(char *start)
{
/* option is used to hold the users input
 * flag is used to exit help when set to 1
 * *end is used to point to the last character printed on the help screen
 */
	int option,flag=0;
	char *end;

	/* Let the function print_help print the index table starting from start and 
	   returning the position of the last index to be put on the screen */
	end= print_help(start);
	/* */

	for (;;)
	{
		option= getch();
		switch(option)
		{
			case KEY_REDRAW:
				/* redraw the screen for the user */
				TM_redraw_screen();
				/* */

				break;
			case 'q':
			case 'Q':
				flag=1;
				stop=0;
				break;
			case KEY_SWITCH:
				flag=1;
				stop=0;
				out=1;
				break;
			case ' ':
				/* Blank out the status bar, because we are no loner on first page of
				   the the help document */
				TM_write_statusbar("\b\r                                                                               "); 
				/* */

				/* If this is true we are at the last page, do not allow the user to
				   go to the next page. */
				if (stop==1)
				{
					/* Write to the statusbar */
					TM_write_statusbar("\b\r END OF DOCUMENT.                                                              ");
					/* */

					break;
				}
				/* */

				/* Let start point to the next character after what end points to */
				start= end++;
				/* */

				/* If their are more pages in the help document allow the user to view them. */
				if (stop==0)
					end = print_help(start);
				break;
				/* */

			case 'b':
                        case 'B':
				/* Write to the statusbar */
				TM_write_statusbar("\b\r                                                                               ");
				/* */

				/* If this is true we are already viewing the first page of the document so
				   we will not allow the user to look at the previous page. */
				if (*start == '\v')
				{
					/* Write to the statusbar. */
					TM_write_statusbar("\b\r BEGINNING OF DOCUMENT.                                                        ");
					/* */

					break;
				}
				/* */

				start-=2;

				/* If this was true, this means we are at the end of the document, since we are
				   going back a page we will not be at the end of the document so reset this variable. */
				if (stop==1)
					stop=0;
				/* */

				for(;;)
				{
					/* This will print out the page from the help documentation */
					if (*start == '\v')
					{
						end=print_help(start);
               		                       	break;
					}
					if (*start=='\b')
					{
						start++;
						end=print_help(start);
						break;
					}
					/* */

					/* This will continously decrement unless it finds the codes that represent page break
					   or beginning of document which are '\b' and '\v' respectively. */
					start--;
					/* */

				}
                                break;
		}

		/* Check to see if the loop can be exited */
		if (flag)
			/* Yes we can exit */
			break;
		/* */

	}
}





/*  print_help() - This will print the help document onto the window and take into consideration how many
 *		   lines it has already printed on the screen.
 *
 *  Parameters:  It requires a pointer which points to the first character to print onto the screen.
 * 
 *  Returns:  It returns a pointer which points to the last character that was displayed on the screen.
 */
char *print_help(char *start)
{
/* line is used to keep track how many lines have been printed on the screen */
/* x is a general purpose variable. */
	int line=0,x;

	/* This will clear the help screen */
	for (x=0;x<=17;x++)
        	{
                	wmove(help_win,x,0);
                        wprintw(help_win,help_clear);
                }
        /* */

	wmove(help_win,0,0);
	for (;;)
	{
		/* Check to see if we encountered a page break, if so stop printing to that screen. */
		if (*start == '\b')
		{
			wmove(help_background_win,0,75);
			wrefresh(help_border_win); 
			wrefresh(help_win); 
			wrefresh(help_background_win);
			start++;
			return start;
		}
		/* */

		/* This will check to see if the document is over */
		if (*start == '\t')
		{	
			/* Yes the document is over. */

			/* Write to the statusbar. */
			TM_write_statusbar("\b\r END OF DOCUMENT.                                                              ");
			/* */

			wmove(help_background_win,0,75);
			wrefresh(help_win);
			wrefresh(help_background_win);
			
			/* Set to one signifying that the end of the document os on the screen. */
			stop=1;
			/* */
			return start;
		}
		else
		{
			/* No the document is not over. */

			/* If the character is a new line then incrment the number of lines */
			if (*start == '\n')
				line++;
			/* */

			/* Turn on reverse */
			if (*start == '\r')
				wattrset(help_win, A_BOLD);
			/* */

 			/* Turn on normal mode */
			else if (*start == '\f')
				wattrset(help_win, A_NORMAL);
			/* */

			/* Check to see if the first page is being read. */
			else if (*start == '\v')
				TM_write_statusbar("\b\r BEGINNING OF DOCUMENT.                                                        "); 
			/* */

			else
			{
				/* This will put the characters onto the screen */
				waddch(help_win, *start);
				/* */
			}
			start++;
		}
		/* */
	}
}

