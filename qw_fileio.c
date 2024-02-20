/*  QuickWrite - (c)1993,1994 Insync Development
 *
 *  qw_fileio.c
 *
 */

/* include the following header files */

#include <stdio.h>
#include <assert.h>
#include <curses.h>
#include "include/qw_defs.h"
#include "include/qw_buffio.h"
#include "include/qw_menu.h"


/* define constants for this module */

#define SAVE_IT   100	/* used to determine which filename will be requested
			from the user */
#define OPEN_IT   101


/* Function Prototypes */

bool check_filename(void);
bool check_string(char *string);
bool FL_open_named_file(BUFFER *buffptr, char *name[64]);
bool open_it(BUFFER *buffptr);
bool readdata(BUFFER *buffptr, FILE *ioptr, char mode);
int  FL_open_file(BUFFER *buffptr);
bool get_user_filename(int choice);
int  print_it(BUFFER *buffptr);
bool check_status(BUFFER *buffptr);
int  FL_save_file(BUFFER *buffptr);
int  FL_save_file_as(BUFFER *buffptr);
void replace_tab(char *string);
int  FL_quit(BUFFER *buffptr);
int  FL_save_clipboard_as(BUFFER *buffptr);
bool save_it(BUFFER *buffptr, int status);
void write_data(BUFFER *buffptr, FILE *ioptr, char mode);
bool FL_get_filename(char *name);


/* variable global to file I/O */

bool filename_given=FALSE, buffer_modified,file_exist= FALSE;
char filename[64], filename_backup[64];


/* variables modified from other modules */

extern unsigned long menu_on, menu_off;


/* functions in this module:  Any functions beginning with FL_ can be called
 * from other modules, all others are only used internally 
 */


/* FL_quit() -  Quits the program , asking if the user is sure and if buffer 
 *		is modified the user is asked whether they wish to save
 *
 * Parameters : pointer to the buffer
 *
 * Returns    : EDIT_MODE or QUIT
 */

int FL_quit(BUFFER *buffptr)
{
	char string[] = "\rAre you sure you want to quit?  (y/N)";
	int c;

	assert (buffptr !=NULL);
	
	/* print the menu bar and the statusbar */
	TM_write_menubar("\b\r QUIT                                                                           ");
	TM_write_statusbar(string);	/* displays question on the screen */
	/* */

	/* get the user's response */
	do
	{
		c = getch();

		if ((c == KEY_RETURN) || (c == KEY_ENTER) || (c == ' '))
		{
			c = 'n';
		}

		c = tolower(c);
	}
	while ((c != 'y') && (c != 'n'));
	/* */

	/* handle user's response */
	if (c == 'y')
	{
		if (check_status(buffptr) == FALSE)
		{
			return EDIT_MODE;
		}
		else
		{
			return QUIT;
		}
	}
	/* */

	return EDIT_MODE;
}


/* FL_save_clipboard_as() - Saves clipboard buffer to the requested filename.
 * 			This function uses the same funtions as save file
 *			(so some variables must be restored once function is
 *			completed.
 *
 * Parameters :         Pointer to the clipboard buffer
 *
 * Returns    :         EDIT_MODE
 */

int FL_save_clipboard_as(BUFFER *buffptr)
{
	bool temp;	/* used to store current status (get_user_filename
		changes status and filename, which we don't want to be done
		here, so we must change it back when this function is
		finished */
	assert(buffptr !=NULL);
	temp = filename_given;		/* stores current status */

	/* print the menu bar */
	TM_write_menubar("\b\r SAVE CLIPBOARD AS                                                              ");
	/* */

	if (get_user_filename(SAVE_IT) == FALSE)  /* gets filename used for
			clipboard, false if user cancels */
	{
		strcpy(filename,filename_backup);  /* if user cancels then
			restore filename */
		filename_given = temp;		/* restore old status */
		return EDIT_MODE;
	}
	save_it(buffptr,SAVE_CLIPBOARD_AS);	/* otherwise save file */
	strcpy(filename,filename_backup); /* restore filename */
        filename_given = temp;          /* restore old status */
        return EDIT_MODE;
}		
	

/* check_status() -     If the buffer has been modified, the user is asked
 *                      whether they would like to save it 
 *
 * Parameters :         A pointer to the buffer (passed along to save operation)
 *
 * Returns    :         TRUE if user wishes to continue operation
 *                      FALSE if user wishes to cancel operation
 */
	 
bool check_status(BUFFER *buffptr)
{
	char string[] = "\rSave changes?  (Y/n)";
	int c;

	assert (buffptr != NULL);

	if (buffer_modified)	/* has buffer changed since last save or open */
	{
		TM_write_statusbar(string);	/* ask for a save */

		/* get user's response */
		do
		{
			c = getch();
			c = tolower(c);

			if ((c == KEY_ENTER) || (c == KEY_RETURN) || (c == ' '))
			{
				c = 'y';
			}
		}
		while ((c != 'n') && (c != 'y'));
		/* */

		switch (c)
		{
			case 'y':
				if (filename_given !=TRUE)  /* if no filename 
						already given */
					if (get_user_filename(SAVE_IT) == FALSE)
						return FALSE; /* cancel */ 	
				if (save_it(buffptr, SAVE) == FALSE)

					return FALSE; /* if operation fails */
				return TRUE;	/* continue */
				break;
			case 'n':	/* if no then continue */
				return TRUE;
				break;
			default:	/* else assume user wishes to cancel */
				return FALSE;	/* cancel */
		}
	}

	return TRUE;	/* buffer has not changed so continue */
}


/* FL_save_file_as() -  Requests a filename from user, and writes the buffer
 *                      to that file
 *
 * Parameters :         Pointer to the buffer
 *
 * Returns    :         EDIT_MODE
 */

int FL_save_file_as(BUFFER *buffptr)
{
	assert (buffptr !=NULL);

	/* print the menubar heading */
	TM_write_menubar("\r\b SAVE FILE AS                                                                   ");
	/* */

		/* get the user's filename */
	if (get_user_filename(SAVE_IT) == TRUE) /* if user didn't cancel */
		save_it(buffptr, SAVE_AS);	/* start actual save */
	return EDIT_MODE;	/* else user cancels so return */
}


/* FL_save_file() -     Checks for a filename, writes buffer to file  
 *
 * Parameters :         Pointer to the buffer
 *
 * Returns    :         EDIT_MODE
 */

int FL_save_file(BUFFER *buffptr)
{
	assert (buffptr !=NULL);

	/* print the menubar heading */
	TM_write_menubar("\r\b SAVE FILE                                                                      ");
	/* */

	if (filename_given ==FALSE)  /* if no current filename is stored */
	{
		FL_save_file_as(buffptr);  /* call save as, which gets 
			filename from user */
		return EDIT_MODE;
	}
	save_it(buffptr, SAVE);	/* start actual save */

	return EDIT_MODE;	/* returns when done */
}


/* save_it() -     Opens file and prepares to save data to the file
 *
 * Parameters : Pointer to the buffer
 *
 * Returns    : TRUE if save was successful 
 *		FALSE if save failed
 */

bool save_it(BUFFER *buffptr, int status)
{
	FILE *ioptr;
	char mode;	/* read or write mode ("r" or "w")*/
	char line[80];	/* to build the string to tell user the file is being saved */
	int c;

	assert(buffptr != NULL);

	mode = 'w';	/* we want to write */
		
	/* get a pointer to the opened file */
	ioptr = (FILE *)UX_open_file(filename_backup,mode,&file_exist);
	if (ioptr == (FILE *)NULL)	/* is pointer NULL */
	{
		if (file_exist)
		{
			TM_write_statusbar("\r\bYou do not have write permission for that file, hit any key to continue        ");
			getch();
			return FALSE; 	
		}

	}

	/* if the file exists and you have correct permissions */
	if ((file_exist) && ((status == SAVE_AS) || (status == SAVE_CLIPBOARD_AS)))
	{
		TM_write_statusbar("\rOverwrite file? (y/N) ");
		/* get user's response*/
		do
		{
			c=getch();
			if ((c==KEY_RETURN) || (c==KEY_ENTER) ||(c==' '))
			{
				c = 'n';
			}
			c = tolower(c);
		}
		while ((c != 'y') && (c!='n'));

		/* handle user's response */
		if (c != 'y')
		{
			UX_close_file(ioptr);
			return FALSE;
		}
	}
	if (BF_number_lines(buffptr) >500)  /* if it's a big file */
	{
		sprintf(line, "\rWriting \"%s\"...", filename_backup);
		TM_write_statusbar(line);
	}




	write_data(buffptr,ioptr,mode);	/* do physical writing to file */

	buffer_modified = FALSE;	/* buffer no longer modified */

	BF_goto_start(buffptr);		/* set up buffer and draw screens */

	return TRUE;
}


/* write_data() -       Does physical writing of data to the disk file  
 *
 * Parameters :         Pointer to the buffer
 *                      Pointer to the file
 *                      Mode (read or write)
 *
 * Returns    :         None
 */

void write_data(BUFFER *buffptr, FILE *ioptr, char mode)
{
	char string[81];	/* stores line from buffer */
	int length;		/* length of line */
	int current_location;	/* stores current location to return to */

	assert (buffptr != NULL);
	assert (ioptr != NULL);

	current_location = BF_get_line_number(buffptr);	/* store location */

	BF_goto_start(buffptr);		/* goto top line */
	do	/* loop through entire buffer */
	{
		length = BF_get_line(buffptr, string)-1; /* get line from
			buffer returning the length of line */
		string[++length] = '\n';	/* add linefeed and null */
		string[++length] = '\0';
		fputs(string,ioptr);		/* send it to file */
	}
	while (BF_move_ahead(buffptr));	/* goto next line until end */

	if (ferror(ioptr) !=0)	/* was there an IO error */
	{
		UX_find_IO_error(ioptr,mode);	/* find what it was */
		clearerr(ioptr);	/* clear the error for next time */
	}
	else
	{
		UX_close_file(ioptr);
	}

	BF_goto_line(buffptr,current_location);	/* return to original location
			in the buffer */

	buffer_modified = FALSE;	/* the buffer is not modified now */
}

		 

/* check_filename() -   Checks filename for invalid characters
 *
 * Parameters :         None
 r
 * Returns       :      TRUE if filename is valid
 *                      FALSE if filename is invalid
 */

bool check_filename(void)
{
	int length;	/* length of filename */
	int i;		/* used for looping */
	length = strlen(filename);	/* get length */
	if (length == 0)
		return FALSE;	/* if length 0, no filename (eg. return key) */
	
	/* loop through all characters in filename checking for validity */
	for (i=0;i<length;i++)
	{
		switch (filename[i]) /* for every character check for the
				following characters */
		{
			case 38: /*&*/ case 42: /***/ case 60: /*<*/
			case 62: /*>*/ case 63: /*?*/ case 124: /*|*/
				return FALSE;	/* if found, return invalid */
		}
	}
	return TRUE;	/* else it must be a valid filename */
}


/* FL_open_file() -     Opens file for reading and checks if filename is 
 *			already given
 *
 * Parameters :         Pointer to the buffer
 *
 * Returns    :         EDIT_MODE
 */

int FL_open_file(BUFFER *buffptr)
{
	assert (buffptr != NULL);

	/* print the menubar heading */
	TM_write_menubar("\r\b OPEN FILE                                                                      ");
	/* */

	if (check_status(buffptr) == FALSE)	/* checks status of buffer */
	{
		return EDIT_MODE;	/* if cancel then return */
	} 
	if (get_user_filename(OPEN_IT) == TRUE)	/* request filename */
		open_it(buffptr);	/* if legal filename then continue */
	return EDIT_MODE;	/* else return */
}


/* get_user_filename() - Requests filename from the user, checking
 *                      for validity
 *
 * Parameters :         None
 *
 * Returns    :         TRUE if filename is entered and valid 
 *                      FALSE if user wishes to cancel operation or invalid
 *			filename
 */

bool get_user_filename(int choice)
{
	char open_string[] = "\rFile to open:";
	char save_string[] = "\rSave file as:";

	if (choice == SAVE_IT)	/* wants filename for save */
		TM_write_statusbar(save_string);
	else	/* must want filename for open */
		TM_write_statusbar(open_string);

	if (TM_read_string(14,64,filename) == FALSE)
	{
		return FALSE; /* if user cancels operation */
	}
	if (check_filename() == FALSE) /* if invalid display error */
	{
		TM_write_statusbar("\r\bYour filename contains invalid characters, hit any key to continue             ");

		getch();
		filename_given=FALSE;	/* no filename stored */
		return FALSE;	/* returns as invalid filename */
	}

	strcpy(filename_backup,filename);	/* make backup of filename */
	filename_given = TRUE;	/* filename is stored */
	return TRUE;	/* user wishes to continue and filename is valid */
}


/* check_string() - Checks string for non-text characters
 *
 * Parameters :         Pointer to a string
 *
 * Returns    :         TRUE if string is valid
 *                      FALSE if string contains invalid characters
 */

bool check_string(char *string)
{	
	int i=0;	/* used for looping */
	assert (string !=NULL);

	/* loop through entire string */
	while ((string[i] != NULL) && (i <=255))
	{ 
		/* check for invalid characters */
		if((string[i] <32) || (string[i] >126))
		{
			if (string[i] != 9) 	/* is it a tab */
			{
				return FALSE;	/* has invalid characters */
			}
			/* must be a tab, so continue */
		} 
		i++;
	}
	replace_tab(string); 	/* replace all tabs with spaces ( make sure
			that tabs are on 8 character boundaries */
	return TRUE;	/* string is alright */
}


/* Replace_tab() -	replaces any occurance of tabs in a string with
 *			eight spaces
 *
 * Parameters :		A pointer to the string
 *
 * Returns    :		Nothing
 */

void replace_tab(char *string)
{
	int i,x,  /* used to loop through both strings at different places */
	j,	/* used as a loop counter */
	new_loc,	/* next tab stop */
	diff;	/* difference between next tab stop and current location */

	char temp[263];	/* holds new string with replaced tabs */
	x = 0;	/* start with both strings at 0 */
	/* loop through every character in the original string */
	for(i=0;i<255;i++)
	{
		if (string[i] == NULL)
		{
			temp[x]=NULL;
			break;
		}

		if (string[i] == 9)	/* if a tab is found */
		{
			new_loc = (x/8+1)*8;	/* calc next tab stop */
				
			diff = new_loc-x;	/* number of spaces to insert */
			if (new_loc > 255)	/* if a tab causes you to go
					past the end of the line */
				break; 		/* exit out of loop */
			for(j=0;j<diff;j++)	/* insert diff # of spaces */	
			{
				temp[x]=' ';	/* insert a space */
				x++;
			}		

		}
		else	/* it's not a tab so copy character from the original
			string (string) to the new string (temp) */
		{
		temp[x]=string[i];
		x++;
		}
	}
	strcpy(string,temp);	/* done so copy new string back to original */	
}


/* FL_open_named_file() -       Opens file with name given as program argument
 *                              upon startup, reads data into buffer
 *
 * Parameters :                 Pointer to the buffer
 *                              Pointer to filename
 *
 * Returns    :                 TRUE if the load was successful
 *				FALSE if file load unsuccessful
 */

bool FL_open_named_file(BUFFER *buffptr, char *name[64])
{
	assert(buffptr!= NULL);
	assert(name!=NULL);
	filename_given = TRUE;	/* the name has been given on the command line*/
	strcpy(filename, name);	/* copy name given to filename */
	strcpy(filename_backup, filename);	/* make backup */
	return open_it(buffptr);	/* open file for writiing (this is 
		the same as what's done for open file) */
}


/* open_it() -     Opens file and reads data into buffer
 *
 * Parameters : Pointer to the buffer
 *
 * Returns    : TRUE if successful
 *		FALSE if unsuccessful 
 */

bool open_it(BUFFER *buffptr)
{
	char mode;	/* mode used to open file */
	FILE *ioptr;	/* the pointer to the file */
	bool status;	/* stores success or failure to be returned */
	char line[80];	/* to build the string to tell user file is being loaded */

	assert(buffptr!=NULL);

	sprintf(line, "\rOpening \"%s\"...", filename_backup);
	TM_write_statusbar(line);

	mode = 'r';	/* we want to read from the file */

	ioptr = (FILE *)UX_open_file(filename_backup,mode,&file_exist);  /* get a pointer
		to the file that was opened */
	if(ioptr == (FILE *)NULL)	/* if file could not be opened, we
		will empty the buffer and keep the filename as a new file (used
		when saving the buffer */
	{
		if (file_exist != TRUE)
		{
			TM_write_statusbar("\r\bThat file doesn't exist, emptying buffer, hit any key to continue              ");
			getch();
			filename_given = TRUE;
			BF_empty_buffer(buffptr);
			BF_goto_start(buffptr);	/* goes to top line and set up screen */
			EW_initial_draw(buffptr);
		}
		else
		{
			TM_write_statusbar("\r\bYou do not have permission to read that file, hit any key to continue          ");
			getch();
		}
		return FALSE;	/* returns that the file couldn't be read */
	}

	status = readdata(buffptr, ioptr, mode);  /* does the read succeed */

	if (status)	/* if successful */
	{
		buffer_modified = FALSE;	/* buffer is not modified */
		BF_goto_start(buffptr);		/* go to top line */
		EW_initial_draw(buffptr);	/* sets up the screen */
	}
	menu_on = M_SAVE | M_SAVE_AS | M_MARK_BLOCK | M_SEARCH | M_SEARCH_REPLACE | M_SEARCH_NEXT;      /* set the new menu options */
	menu_off = NULL;
	MH_update_menus();

	return status;	/* returns whether read was successful */
}


/* readdata() -        Reads data from a file into the buffer, checking to
 *                      ensure that it is a text file.  If there is an error
 *                      or it's not a text file, then buffer will be emptied
 *                      and user will be returned to edit mode.
 *
 * Parameters :         Pointer to the buffer
 *                      Pointer to the file
 *                      Mode (read or write)
 *
 * Returns    :         TRUE if read was successful
 *			FALSE if unseccussful 
 */

bool readdata(BUFFER *buffptr, FILE *ioptr, char mode)
{
	char string[255];	/* max string length to be read in is set to
		255 characters */
	int length;	/* holds length of string that was read */
	bool ok;	/* stores if it's a valid string */
	
	assert(buffptr !=NULL);
	assert(ioptr != NULL);
	BF_initiate_load(buffptr);	/* starts the initial buffer load */
	while (fgets(string, 255, ioptr) != NULL)  /* while the end of the
		file has not been reached, read in 255 characters from the
		file and store it in string */
	{
		string[strlen(string)-1] = NULL;/* remove the carriage return */
		ok = check_string(string);	/* check for invalid characters
			and replace tabs with spaces */
		length= strlen(string);	/* length of the string */
		if (!ok)	/* if string has invalid charcters */
		{
			TM_write_statusbar("\r\bThis is not a text file, and will not be opened, hit any key to continue       "); /* write error */
			getch();
			BF_finish_load(buffptr);	/* finish the buffer load */    
			BF_empty_buffer(buffptr);	/* empty the buffer */
			filename_given = FALSE;	/* we don't want to be able to
				save and overwrite a binary file later */
			UX_close_file(ioptr);
			return FALSE;	/* return an unsuccessful read */
		}

		/* can the string be put into the buffer */
		if (BF_load_string(buffptr, string, length) == FALSE)
		{	/* cannot enter string into buffer */
/*			ER_put_error(OUT_OF_MEM);	
  must uncomment this when it is all put together*/
			filename_given=FALSE;	/* we don't want to be able to save to a pertailly loaded file */
			BF_finish_load(buffptr);  /* finish the buffer load */
			BF_empty_buffer(buffptr); /* empty the buffer */
			UX_close_file(ioptr);
			return FALSE;	/* return unsuccessful read */
		}
	}
	if (ferror(ioptr) != 0)	/* was there a read error */
	{	
		UX_find_IO_error(ioptr,mode); /* if yes find it */	
		BF_empty_buffer(buffptr);	/* empty the buffer */	
		clearerr(ioptr);	/* clear all errors for the next time */
		return FALSE;	/* return unsuccessful read */
	}
	else
	{
		UX_close_file(ioptr);
	}

	/* else read was successful */
	BF_finish_load(buffptr);	/* finish the buffer load */
	buffer_modified = FALSE;	/* buffer is not modified now */
	return TRUE;	/* return successful read */
}

/* FL_get_filename() -  Copies current filename in to the address that
 *			is passed to it 
 *
 * Parameters :         Pointer to a string containing the filename
 *
 * Returns    :         TRUE if filename exists
 *                      FALSE if filename doesn't exist
 */

bool FL_get_filename(char *name)
{
        /* if filename exits, then copy it and return true */
        if (filename_given == TRUE)
	{
                strcpy (name, filename);	/* copy filename */
                return TRUE;
	}

        return FALSE;   /* otherwise return false */
}

