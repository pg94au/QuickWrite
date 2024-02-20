/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_search.c
 */



#include <string.h>
#include <curses.h>
#include <assert.h>
#include "include/qw_defs.h"
#include "include/qw_termio.h"
#include "include/qw_cursor.h"


/*  function prototypes */

int SR_search(BUFFER *buffptr);
int SR_search_next(BUFFER *buffptr);
int SR_search_replace(BUFFER *buffptr);
bool findkey(BUFFER *buffptr, char *key, bool askwrap, int endline, int endpos);

/* */

/* global variables */

char key[51];	/* search key, entered by user */

/* */




/*  SR_search()  -  Prompts the user for a search key and highlights the next
 *                  occurence of that string in the buffer.
 *
 *  Parameters:  pointer to the buffer
 *
 *  Returns:  EDIT_MODE
 */
int SR_search(BUFFER *buffptr)
{
	int keylength, x, linelength, xpos;
	int origline, origpos, currline;
	char line[80];
	int endline, endpos;


	/* Assertions */
	assert(buffptr != NULL);
	/* */


	/* get the search key from the user */
	TM_write_menubar("\r\b SEARCH                                                                         ");
	TM_write_statusbar("\rSearch for:                                                                     ");

	if (TM_read_string(12,50,key) == FALSE)
	{
		return EDIT_MODE;
	}
	/* */

	/* inform user of search */
	TM_write_statusbar("\rSearching...                                                                    ");
	/* */

	/* get the line and position to stop searching at */
	endline = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&endpos, NULL, NULL);
	/* */

	/* search through the buffer for the entered key */
	if (findkey(buffptr, key, TRUE, endline, endpos) == FALSE)
	{
		TM_write_statusbar("\r\bString not found.  Hit any key.                                                 ");
		getch();

		return EDIT_MODE;
	}
	/* */

	/* notify user that key was found */
	currline = BF_get_line_number(buffptr);
	sprintf(line, "\rString found on line %d.  Hit any key.", currline);
	TM_write_statusbar(line);
	/* */

	/* highlight the found key */
	origline = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&origpos, NULL, NULL);
	EW_update_position(buffptr);

	keylength = strlen(key);
	linelength = BF_get_line(buffptr, line);

	for (x=0; x < keylength; x++)
	{
		EW_redraw_char(buffptr, A_BOLD);
		CP_move_right(buffptr);
		EW_get_cursor_pos(&xpos, NULL, NULL);
		if ((xpos == linelength) && (x < keylength-1))
		{
			CP_move_right(buffptr);
			linelength = BF_get_line(buffptr, line);
		}
	}
	EW_refresh();
	/* */

	/* wait for key press */
	getch();
	/* */

	/* unhighlight key */
	BF_goto_line(buffptr, origline);
	EW_set_cursor_pos(origpos, origpos);
	EW_update_position(buffptr);

	linelength = BF_get_line(buffptr, line);

	for (x=0; x < keylength; x++)
	{
		EW_redraw_char(buffptr, A_NORMAL);
		CP_move_right(buffptr);
		EW_get_cursor_pos(&xpos, NULL, NULL);
		if ((xpos == linelength) && (x < keylength-1))
		{
			CP_move_right(buffptr);
			linelength = BF_get_line(buffptr, line);
		}
	}
	EW_refresh();
	/* */

	return EDIT_MODE;
}





/*  SR_search_next()  -  Prompts the user for a search string if one has not
 *                       been entered previously.  Then continues to find the
 *                       next occurence of this string in the buffer as long
 *                       as the user wants to.
 *
 *  Parameters:  a pointer to the buffer
 *
 *  Returns:  EDIT_MODE
 */
int SR_search_next(BUFFER *buffptr)
{
	int keylength, x, linelength, xpos;
	int origline, origpos, currline;
	int c='n', cont;
	char line[80];
	int foundline, foundpos, lastfoundline, lastfoundpos;
	bool found = FALSE;
	int endline, endpos;


	/* Assertions */
	assert(buffptr != NULL);
	/* */


	/* write the statusbar */
	TM_write_menubar("\r\b SEARCH NEXT                                                                    ");
	/* */

	/* see if there is a previously entered key, and if the user wants to use it */
	if (key[0] != NULL)
	{
		sprintf(line, "\rSearch for `%s'?  (Y/n)",key);
		TM_write_statusbar(line);

		/* get the response from the user */
		do
		{
			c = getch();

			/* user can abort if desired */
			if (c == KEY_SWITCH)
			{
				return EDIT_MODE;
			}
			/* */

			/* accept an enter key or space as a yes */
			if ((c == KEY_ENTER) || (c == KEY_RETURN))
			{
				c = 'y';
			}
			/* */

			c = tolower(c);
		}
		while ((c != 'y') && (c != 'n'));
		/* */
	}

	/* get search string from user if desired */
	if (c == 'n')
	{
		TM_write_statusbar("\rSearch for:                                                                     ");

		if (TM_read_string(12,50,key) == FALSE)
		{
			return EDIT_MODE;
		}
	}
	/* */

	/* do the searches in a loop as long as the user wants to */
	do
	{
		/* inform user of search */
		TM_write_statusbar("\rSearching...                                                                    ");
		/* */

		/* get the position at which to stop this next search */
		endline = BF_get_line_number(buffptr);
		EW_get_cursor_pos(&endpos, NULL, NULL);
		/* */

		/* search through the buffer for the entered key */
		if (findkey(buffptr, key, TRUE, endline, endpos) == FALSE)
		{
			TM_write_statusbar("\r\bString not found.  Hit any key.                                                 ");
			getch();

			return EDIT_MODE;
		}
		/* */

		/* see if this find is the one that was previously just found */
		foundline = BF_get_line_number(buffptr);
		EW_get_cursor_pos(&foundpos, NULL, NULL);
		if ((found == TRUE) && (foundline == lastfoundline) && (foundpos == lastfoundpos))
		{
			TM_write_statusbar("\rNo more occurences of the string were found.  Hit a key.                        ");
			getch();

			return EDIT_MODE;
		}
		else
		{
			lastfoundline = foundline;
			lastfoundpos = foundpos;
			found = TRUE;
		}
		/* */

		/* notify user that key was found */
		currline = BF_get_line_number(buffptr);
		sprintf(line, "\rString found on line %d.  Find next occurence?  (Y/n)", currline);
		TM_write_statusbar(line);
		/* */

		/* highlight the found key */
		origline = BF_get_line_number(buffptr);
		EW_get_cursor_pos(&origpos, NULL, NULL);
		EW_update_position(buffptr);

		keylength = strlen(key);
		linelength = BF_get_line(buffptr, line);

		for (x=0; x < keylength; x++)
		{
			EW_redraw_char(buffptr, A_BOLD);
			CP_move_right(buffptr);
			EW_get_cursor_pos(&xpos, NULL, NULL);
			if ((xpos == linelength) && (x < keylength-1))
			{
				CP_move_right(buffptr);
				linelength = BF_get_line(buffptr, line);
			}
		}
		EW_refresh();
		/* */

		/* wait for key press */
		do
		{
			cont = getch();

			/* user can abort if desired */
			if (cont == KEY_SWITCH)
			{
				cont = 'n';
			}
			/* */

			/* accept the enter key as a yes */
			if ((cont == KEY_ENTER) || (cont == KEY_RETURN) || (cont == ' '))
			{
				cont = 'y';
			}
			/* */

			cont = tolower(cont);
		}
		while ((cont != 'y') && (cont != 'n'));
		/* */

		/* unhighlight key */
		BF_goto_line(buffptr, origline);
		EW_set_cursor_pos(origpos, origpos);
		EW_update_position(buffptr);

		linelength = BF_get_line(buffptr, line);

		for (x=0; x < keylength; x++)
		{
			EW_redraw_char(buffptr, A_NORMAL);
			CP_move_right(buffptr);
			EW_get_cursor_pos(&xpos, NULL, NULL);
			if ((xpos == linelength) && (x < keylength-1))
			{
				CP_move_right(buffptr);
				linelength = BF_get_line(buffptr, line);
			}
		}
		EW_refresh();
		/* */
	}
	while (cont == 'y');
	/* */

	return EDIT_MODE;
}





/*  SR_search_replace()  -  Prompts user for both search and replace strings,
 *                          then finds search next search string and asks user
 *                          whether or not to replace it with replace string
 *                          as long as user wants to continue.
 *
 *  Parameters:  pointer to the buffer
 *
 *  Returns:  EDIT_MODE
 */
int SR_search_replace(BUFFER *buffptr)
{
	int origline, origpos, finalline, finalpos, lastline, lastpos;
	bool wrap=FALSE;
	char searchkey[51], replacekey[51], line[80], c;
	int skeylen, rkeylen, linelength;
	int currline, currpos, xpos, x;
	int prevlength;
	char prevline[80];

	/* Assertions */
	assert(buffptr != NULL);
	/* */

	/* get position in buffer at which this function was called */
	origline = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&origpos, NULL, NULL);
	finalline = origline;
	finalpos = origpos;
	lastline = origline;
	lastpos = origpos;
	/* */

	/* get the search and replae keys from the user */
	TM_write_menubar("\r\b SEARCH AND REPLACE                                                             ");
	TM_write_statusbar("\rSearch for:");
	if (TM_read_string(12, 50, searchkey) == FALSE)
	{
		return EDIT_MODE;
	}
	TM_write_statusbar("\rReplace with:");
	if (TM_read_string(14, 50, replacekey) == FALSE)
	{
		return EDIT_MODE;
	}
	/* */

	/* check if search and replace strings are identical */
	if (strcmp(searchkey, replacekey) == 0)
	{
		TM_write_statusbar("\b\rSearch and replace strings are identical, nothing to do!  Hit a key.            ");
		getch();

		return EDIT_MODE;
	}
	/* */

	/* get the key length for use in highlighting, inserting and deleting */
	skeylen = strlen(searchkey);
	rkeylen = strlen(replacekey);
	/* */

	/* search/replace in a loop until user finished or user terminates */
	for (;;)
	{
		/* inform user of search */
		TM_write_statusbar("\rSearching...");
		/* */

		/* search through the buffer for the entered key */
		if (findkey(buffptr, searchkey, (wrap ^ TRUE), origline, origpos) == FALSE)
		{
			TM_write_statusbar("\r\bSearch string not found.  Hit a key.                                            ");
			getch();

			BF_goto_line(buffptr, finalline);
			EW_set_cursor_pos(finalpos, finalpos);
			EW_update_position(buffptr);

			return EDIT_MODE;
		}
		/* */

		/* see if findkey() had to wrap to find a match */
		currline = BF_get_line_number(buffptr);
		EW_get_cursor_pos(&currpos, NULL, NULL);
		if ((currline < lastline) || ((currline == lastline) && (currpos < lastpos)))
		{
			wrap = TRUE;
		}
		/* */

		/* has this occurence of the search string been found already? */
		if (((wrap == TRUE) && ((currline > origline) || ((currline == origline) && (currpos >= origpos)))) || ((currline == lastline) && (currpos == lastpos)))
		{
			TM_write_statusbar("\rFinished searching.  Hit a key.");
			getch();
			BF_goto_line(buffptr, finalline);
			EW_set_cursor_pos(finalpos, finalpos);
			EW_update_position(buffptr);

			return EDIT_MODE;
		}
		/* */

		/* this is the first time we've found this string, so store it's position */
		lastline = currline;
		lastpos = currpos;
		/* */

		/* notify user that key was found */
		sprintf(line, "\rString found on line %d.  Replace this string?  \b(Y/n)\n\r  Quit  \b(Q)\n\r", currline);
		TM_write_statusbar(line);
		/* */

		/* highlight the found key */
		linelength = BF_get_line(buffptr, line);
		EW_update_position(buffptr);
		for (x=0; x < skeylen; x++)
		{
			EW_redraw_char(buffptr, A_BOLD);
			CP_move_right(buffptr);
			EW_get_cursor_pos(&xpos, NULL, NULL);
			if ((xpos == linelength) && (x < skeylen-1))
			{
				CP_move_right(buffptr);
				linelength = BF_get_line(buffptr, line);
			}
		}
		EW_refresh();
		/* */

		/* there is a new final cursor position after this function terminates */
		finalline = BF_get_line_number(buffptr);
		EW_get_cursor_pos(&finalpos, NULL, NULL);
		/* */

		/* get user's input response */
		do
		{
			c = getch();
			c = tolower(c);

			/* user can abort if desired */
			if (c == KEY_SWITCH)
			{
				c = 'q';
			}
			/* */
			/* accept Enter or Space as a yes response */
			if ((c == KEY_ENTER) || (c == KEY_RETURN) || (c == ' '))
			{
				c = 'y';
			}
			/* */
		}
		while ((c != 'y') && (c != 'n') && (c != 'q'));
		/* */

		/* if the user's response was to quit... */
		if (c == 'q')
		{
			/* unhighlight the string */
			BF_goto_line(buffptr, lastline);
			EW_set_cursor_pos(lastpos, lastpos);
			EW_update_position(buffptr);
			linelength = BF_get_line(buffptr, line);

			for(x=0; x < skeylen; x++)
			{
				EW_redraw_char(buffptr, A_NORMAL);
				CP_move_right(buffptr);
				EW_get_cursor_pos(&xpos, NULL, NULL);
				if ((xpos == linelength) && (x < rkeylen-1))
				{
					CP_move_right(buffptr);
					linelength = BF_get_line(buffptr, line);
				}
			}
			EW_refresh();
			/* */

			/* go to final position */
			BF_goto_line(buffptr, finalline);
			EW_set_cursor_pos(finalpos, finalpos);
			EW_update_position(buffptr);
			/* */

			return EDIT_MODE;
		}
		/* */

		/* if user's response was to not replace the string */
		if (c == 'n')
		{
			/* unhighlight the found string */
			BF_goto_line(buffptr, lastline);
			EW_set_cursor_pos(lastpos, lastpos);
			EW_update_position(buffptr);
			linelength = BF_get_line(buffptr, line);

			for (x=0; x < skeylen; x++)
			{
				EW_redraw_char(buffptr, A_NORMAL);
				CP_move_right(buffptr);
				EW_get_cursor_pos(&xpos, NULL, NULL);
				if ((xpos == linelength) && (x < skeylen-1))
				{
					CP_move_right(buffptr);
					linelength = BF_get_line(buffptr, line);
				}
			}
			EW_refresh();
			/* */

			/* go to final position */
			BF_goto_line(buffptr, finalline);
			EW_set_cursor_pos(finalpos, finalpos);
			EW_update_position(buffptr);
			/* */
		}
		/* */

		/* if user's response was to replace this string... */
		if (c == 'y')
		{
			/* delete the search string */
			BF_goto_line(buffptr, finalline);
			EW_set_cursor_pos(finalpos, finalpos);
			EW_update_position(buffptr);

			for (x=0; x < skeylen; x++)
			{
				TX_backspace_key(buffptr);
				EW_get_cursor_pos(&xpos, NULL, NULL);
				if ((xpos == 0) && (x < skeylen-1))
				{
					BF_move_back(buffptr);
					prevlength = BF_get_line(buffptr, prevline);
					BF_move_ahead(buffptr);

					if (prevlength < 79)
					{
						TX_backspace_key(buffptr);
					}
				}
			}
			/* */

			/* insert the replace string */
			for (x=0; x < rkeylen; x++)
			{
				TX_insert_character(buffptr, replacekey[x]);
			}
			/* */

			/* highlight the replaced string */
			BF_goto_line(buffptr, lastline);
			EW_set_cursor_pos(lastpos, lastpos);
			EW_update_position(buffptr);
			linelength = BF_get_line(buffptr, line);

			if (lastpos == linelength)
			{
				CP_move_right(buffptr);
			}

			for (x=0; x < rkeylen; x++)
			{
				EW_redraw_char(buffptr, A_BOLD);
				CP_move_right(buffptr);
				EW_get_cursor_pos(&xpos, NULL, NULL);
				if ((xpos == linelength) && (x < rkeylen-1))
				{
					CP_move_right(buffptr);
					linelength = BF_get_line(buffptr, line);
				}
			}
			EW_refresh();
			/* */

			/* this is the new final position now */
			finalline = BF_get_line_number(buffptr);
			EW_get_cursor_pos(&finalpos, NULL, NULL);
			/* */

			TM_write_statusbar("\rHit a key to continue, or \bQ\n\r to quit.");

			/* get user's response */
			c = getch();
			c = tolower(c);
			/* */

			/* unhighlight the replaced key */
			BF_goto_line(buffptr, lastline);
			EW_set_cursor_pos(lastpos, lastpos);
			EW_update_position(buffptr);
			linelength = BF_get_line(buffptr, line);

			if (lastpos == linelength)
			{
				CP_move_right(buffptr);
			}

			for (x=0; x < rkeylen; x++)
			{
				EW_redraw_char(buffptr, A_NORMAL);
				CP_move_right(buffptr);
				EW_get_cursor_pos(&xpos, NULL, NULL);
				if ((xpos == linelength) && (x < rkeylen-1))
				{
					CP_move_right(buffptr);
					linelength = BF_get_line(buffptr, line);
				}
			}
			EW_refresh();
			/* */

			/* did the user want to quit? */
			if (c == 'q')
			{
				BF_goto_line(buffptr, finalline);
				EW_set_cursor_pos(finalpos, finalpos);
				EW_update_position(buffptr);

				return EDIT_MODE;
			}
			/* */
		}
		/* */
	}
	/* */
}





/*  findkey()  -  From the current location in the buffer, try to find an
 *                occurence of key.  Stop searching once position in buffer
 *                corresponding to endline, endpos is reached.
 *
 *  Parameters:  pointer to the buffer
 *               string to be searched for
 *               whether or not to ask when if this function reaches the end
 *                   of the buffer if it should start at the top again
 *                   (TRUE or FALSE)
 *               the position (line and position) at which to stop searching
 *
 *  Returns:  TRUE if found, FALSE otherwise
 *
 *  Notes:  If a string is found, the current location in the buffer will be
 *          the beginning of the found string, else it will be where the
 *          search began.
 */
bool findkey(BUFFER *buffptr, char *key, bool askwrap, int endline, int endpos)
{
	int startline, startpos, searchline;
	char line[80], string[180], *ptr, c;
	bool wrap = FALSE;
	int len, offset, mark;
	int currline, currpos;


	/* Assertions */
	assert(buffptr != NULL);
	assert(key != NULL);
	/* */

	/* store the starting position to check when we've searched everything */
	startline = BF_get_line_number(buffptr);
	EW_get_cursor_pos(&startpos, NULL, NULL);
	/* */

	/* search until we find the key or have searched all text */
	for (;;)
	{
		/* see if we've gone as far as necessary in this search */
		currline = BF_get_line_number(buffptr);
		EW_get_cursor_pos(&currpos, NULL, NULL);

		if ((endline > startline) || ((endline == startline) && (endpos > startpos)))
		{
			if ((wrap == TRUE) || (currline > endline) || ((currline == endline) && (currpos > endpos)))
			{
				return FALSE;
			}
		}
		else
		{
			if ((wrap == TRUE) && ((currline > endline) || ((currline == endline) && (currpos > endpos))))
			{
				return FALSE;
			}
		}
		/* */

		/* update the circling bar when necessary */
		if (currline % 100 == 0)
		{
			TM_wait_bar(15);
		}
		/* */

		/* see if we have just wrapped, and prompt user if so */
		if ((((wrap == TRUE) && (BF_get_line_number(buffptr) == 1)) && (askwrap == TRUE)) && ((startline != 1) || (startpos != 0)))
		{
			TM_write_statusbar("\rContinue searching from the top of the document?  (Y/n)                         ");

			/* get user's response */
			do
			{
				c = getch();
				c = tolower(c);

				if (c == KEY_SWITCH)
				{
					c = 'n';
				}
				if ((c == KEY_ENTER) || (c == KEY_RETURN) || (c == ' '))
				{
					c = 'y';
				}
			}
			while ((c != 'y') && (c != 'n'));
			/* */

			if (c == 'n')
			{
				BF_goto_line(buffptr, startline);
				EW_set_cursor_pos(startpos, startpos);
				EW_update_position(buffptr);
				return FALSE;
			}

			TM_write_statusbar("\rSearching...                                                                    ");
		}
		/* */

		/** search from where we are now **/
		/* store the # of the line we're searching from currently */
		searchline = BF_get_line_number(buffptr);
		/* */

		/* is this startline? */
		if (searchline == startline)
		{
			len = BF_get_line(buffptr, line);

			if (wrap == FALSE)
			{
				/* get the first line into string */
				strncpy(string, line+startpos, len-startpos);
				offset = len-startpos;
				mark = offset;
				/* */

				/* get the 2nd line into string if possible */
				if (BF_at_end(buffptr) == TRUE)
				{
					BF_goto_start(buffptr);
					wrap = TRUE;
				}
				else
				{
					BF_move_ahead(buffptr);
					len = BF_get_line(buffptr, line);
					strncpy(string+offset, line, len);
					offset = offset + len;
				}
				/* */

				/* search through this string now */
				string[offset] = NULL;
				ptr = strstr(string, key);
				if ((ptr != NULL) && (ptr-string < mark))
				{
					BF_goto_line(buffptr, searchline);
					EW_set_cursor_pos(startpos+ptr-string, startpos+ptr-string);
					EW_update_position(buffptr);
					EW_reposition_cursor(buffptr, 10);
					return TRUE;
				}
				/* */
			}
			else
			{
				/* get this line into string */
				strncpy(string, line, startpos);
				offset = startpos;
				/* */

				/* search through this string now */
				string[offset] = NULL;
				ptr = strstr(string, key);
				if (ptr != NULL)
				{
					BF_goto_line(buffptr, searchline);
					EW_set_cursor_pos(ptr-string, ptr-string);
					EW_update_position(buffptr);
					EW_reposition_cursor(buffptr, 10);
					return TRUE;
				}
				else
				{
					BF_goto_line(buffptr, startline);
					EW_set_cursor_pos(startpos, startpos);
					EW_update_position(buffptr);
					return FALSE;
				}
				/* */
			}
		}
		/* end - is this startline? */

		/* else this is any line other than startline */
		else
		{
			len = BF_get_line(buffptr, line);

			/* get the first line into string */
			strncpy(string, line, len);
			offset = len;
			mark = offset;
			/* */

			/* get 2nd line into string if possible */
			if (BF_at_end(buffptr) == TRUE)
			{
				BF_goto_start(buffptr);
				wrap = TRUE;
			}
			else
			{
				BF_move_ahead(buffptr);
				len = BF_get_line(buffptr, line);
				strncpy(string+offset, line, len);
				offset = offset+len;
			}
			/* */

			/* search through this string now */
			string[offset] = NULL;
			ptr = strstr(string, key);
			if ((ptr != NULL) && (ptr-string < mark))
			{
				BF_goto_line(buffptr, searchline);
				EW_set_cursor_pos(ptr-string, ptr-string);
				EW_update_position(buffptr);
				EW_reposition_cursor(buffptr, 10);
				return TRUE;
			}
		}
		/* end - else this is any line other than startline */
	}
	/* */
}

