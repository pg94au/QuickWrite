/*  QuickWrite - (c)1993,1994 Insync Development
 *
 *  qw_buffio.c
 */


#include <stdio.h>
#include <assert.h>
#include "include/qw_defs.h"


/* define values for internal memory buffer size */

#ifndef MULT
#define MULT 5
#endif

#define MEMSIZE		MULT*100
#define UPDATE_SIZE	MULT*75
#define AHEAD_POS	MULT*25
#define MIDDLE_POS	MULT*37
#define BACK_POS	MULT*50

/* */


/* define the structure of a single line of text */

struct linestruct {
	char string[LLEN];	/* store for a line of text */
	int length;		/* count of chars on corresponding line */
};

/* */


/* define the structure of a buffer control structure */

struct bufferstruct {
	int total_lines;	/* total line stored */
	int top_line;		/* number of the first line in the memory buffer */
	int current_line;	/* offset in memory buffer of the current line */
	int buffer_lines;	/* number of lines currently held in memory buffer */
	int prev_lines;		/* number of lines read into memory buffer during last swap */
	FILE *fp;		/* pointer to file for the disk swap file */
	char filename[19];	/* actual name of the temp file on disk */
	struct linestruct lines[MEMSIZE];	/* internal memory buffer */
	bool dirty;		/* clean/dirty state of memory buffer */
};

typedef struct bufferstruct BUFFER;    /* typedef for convenience */

/* */


/* define return values of BF_delete_line() */

#define DL_OK		1
#define DL_LAST		0
#define DL_ONLY		-1
#define DL_EMPTY	-2
#define DL_ERR		-3

/* */


/* function prototypes */

BUFFER *BF_create_buffer(void);
void BF_free_buffer(BUFFER *buffptr);
bool BF_move_ahead(BUFFER *buffptr);
bool BF_move_back(BUFFER *buffptr);
bool BF_goto_line(BUFFER *buffptr, int line);
int BF_get_line_number(BUFFER *buffptr);
int BF_number_lines(BUFFER *buffptr);
bool BF_goto_start(BUFFER *buffptr);
bool BF_goto_end(BUFFER *buffptr);
bool BF_at_start(BUFFER *buffptr);
bool BF_at_end(BUFFER *buffptr);
int BF_get_line(BUFFER *buffptr, char *lineptr);
bool BF_insert_line_after(BUFFER *buffptr);
int BF_delete_line(BUFFER *buffptr);
void BF_put_line(BUFFER *buffptr, char *string, int length);
bool BF_empty_buffer(BUFFER *buffptr);
bool BF_put_string(BUFFER *buffptr, char *string, int length, int *cursor_x, int *cursor_line, int *lines_put, int *wrap_pos, bool *last_line);
void copy_line_to_buffer(BUFFER *buffptr, char *string, int first, int last);
int find_wrap_position(char *string, int startpos, int length);
bool update_buffer(BUFFER *buffptr, int line, int position);
bool BF_initiate_load(BUFFER *buffptr);
bool BF_finish_load(BUFFER *buffptr);
bool copy_line_to_tempfile(BUFFER *buffptr, char *string, int first, int last);
bool BF_load_string(BUFFER *buffptr, char *string, int length);
bool BF_delete_lines(BUFFER *buffptr, int first, int last);

/* */





/*  BF_create_buffer()  -  Allocate a new buffer control structure and
 *                         initialize it.  Involves allocating memory for
 *                         the structure, opening a disk file, and setting
 *                         initial values.
 *
 *  Parameters:  none
 *
 *  Returns:  A pointer to the control structure just initialized if
 *                 successful, else returns NULL
 *
 *  Notes:  The pointer returned by this function is to be the only means
 *          of reference the user will have to access the buffer, and
 *          such accesses must be made ONLY via the provided Buffer I/O
 *          functions.
 */
BUFFER *BF_create_buffer()
{
	BUFFER *newbuff;

	/* Allocate space for the buffer info */
	newbuff = (BUFFER *)malloc(sizeof(BUFFER));
	if (newbuff == NULL)	/* could not allocate control structure */
	{
		return NULL;
	}
	
	/* set initial values for control structure */
        newbuff->total_lines = 1;
	newbuff->current_line = 0;
	newbuff->top_line = 1;
	newbuff->buffer_lines = 1;
	newbuff->prev_lines = 0;
	strcpy(newbuff->filename, "/usr/tmp/.QWXXXXXX");
	newbuff->dirty = TRUE;
	/* */
	
	/* open the temporary disk file */
	newbuff->fp = (FILE *)fopen(mktemp(newbuff->filename),"w+");
	if (newbuff->fp == NULL)	/* could not open file */
	{
		free(newbuff);
		return NULL;
	}
	/* */
	
	return newbuff;
}





/*  BF_free_buffer()  -  Free the contents of the specified buffer.  This
 *                       will include both the temporary disk file created,
 *                       and the memory being used by the control block.
 *
 *  Parameters:  A pointer to the control structure of the buffer to free
 *                    (the value which was returned by BF_create_buffer())
 *
 *  Returns:  None
 */
void BF_free_buffer(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* close and delete file */
	fclose(buffptr->fp);
	unlink(buffptr->filename);
	/* */
	
	/* free control structure */
	free(buffptr);
	/* */
}





/*  BF_move_ahead()  -  Move ahead one line in the buffer.  This function
 *                      will attempt to make the line currently pointed to
 *                      in the buffer one previous from it's current value.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the function was successful
 *            FALSE if the function fails, either due to the fact that
 *                 the current line is already the first one in the buffer,
 *                 or because there was an error swapping to disk.
 */
bool BF_move_ahead(BUFFER *buffptr)
{
	int line;


	/* Assertions */
	assert(buffptr != NULL);
	/* */
		
	
	/* can we not go ahead? */
	if ((buffptr->top_line + buffptr->current_line) == buffptr->total_lines)
	{
		return FALSE;
	}
	/* */
	
	/* do we not need to access the disk? */
	if (buffptr->current_line < buffptr->buffer_lines - 1)
	{
		buffptr->current_line++;
		return TRUE;
	}
	/* */
	
	/* goto new line, swapping the buffer to the disk and back */
	line = buffptr->top_line + buffptr->current_line + 1;

	return (update_buffer(buffptr, line, AHEAD_POS));
	/* */
}





/*  BF_move_back()  -  Move back one line in the buffer.  This function
 *                     will attempt to make the line currently pointed to
 *                     in the buffer one following it's current value.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the function was successful
 *            FALSE if the function fails, either due to the fact that
 *                 the current line is already the last one in the buffer,
 *                 or because there was an error swapping to disk.
 */
bool BF_move_back(BUFFER *buffptr)
{
	int line;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
		
	
	/* can we not go back? */
	if ((buffptr->top_line + buffptr->current_line) == 1)
	{
		return FALSE;
	}
	/* */
	
	/* do we not need to access the disk? */
	if (buffptr->current_line > 0)
	{
		buffptr->current_line--;
		return TRUE;
	}
	/* */
	
	/* goto new line, swapping the buffer to disk & back */
	line = buffptr->top_line + buffptr->current_line - 1;
	
	return (update_buffer(buffptr, line, BACK_POS));
	/* */
}





/*  BF_goto_line()  -  Move to the specified line in the buffer.  This
 *                     function will attempt to make the line currently
 *                     pointed to in the buffer the one specified, in terms
 *                     of ordering lines starting with the first line being
 *                     number 1.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the function was successful
 *            FALSE if the function fails, either due to the fact that
 *                 the line specified is out of the range of the buffer,
 *                 or because there was an error swapping to disk.
 */
bool BF_goto_line(BUFFER *buffptr, int line)
{
	/* Assertions */
	assert(buffptr != NULL);
	assert(line >= 1);
	/* */
	
	
	/* does this line exist? */
	if (line > buffptr->total_lines)
	{
		return FALSE;
	}
	/* */
	
	/* are we already at the line specified? */
	if (line == (buffptr->top_line + buffptr->current_line))
	{
		return TRUE;
	}
	/* */
	
	/* do we not need to access the disk? */
	if ((line >= buffptr->top_line) && (line < buffptr->top_line + buffptr->buffer_lines))
	{
		buffptr->current_line = line - buffptr->top_line;
		return TRUE;
	}
	/* */
	
	/* goto new line, swapping buffer to disk */
	return (update_buffer(buffptr, line, MIDDLE_POS));
}





/*  BF_get_line_number()  -  Get the number of the line currently pointed
 *                           to in the buffer as an ordinal value, based on
 *                           it's position with respect to the first line
 *                           in the buffer.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  The number of the current line
 */
int BF_get_line_number(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	return (buffptr->top_line + buffptr->current_line);
}





/*  BF_number_lines()  -  Get the total number of lines currently being
 *                        stored in the buffer.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  The total number of lines stored in the buffer
 */
int BF_number_lines(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	return (buffptr->total_lines);
}





/*  BF_goto_start()  -  Make the first line in the buffer the line which is
 *                      the one currently pointed to.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the function was successful
 *            FALSE if the function fails because there was an error
 *                 swapping to disk.
 */
bool BF_goto_start(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	return (BF_goto_line(buffptr, 1));
}





/*  BF_goto_end()  -  Make the last line in the buffer the line which is
 *                    the one currently pointed to.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the function was successful
 *            FALSE if the function fails because there was an error
 *                 swapping to disk.
 */
bool BF_goto_end(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	return (BF_goto_line(buffptr, BF_number_lines(buffptr)));
}





/*  BF_at_start()  -  Check whether the line currently pointed to in the
 *                    buffer is the first line in the entire buffer.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the current line is the first one in the buffer
 *            FALSE if the current line is not the first one in the buffer
 */
bool BF_at_start(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	if (BF_get_line_number(buffptr) == 1)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}





/*  BF_at_end()  -  Check whether the line currently pointed to in the
 *                  buffer is the last line in the entire buffer.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the current line is the last one in the buffer
 *            FALSE if the current line is not the last one in the buffer
 */
bool BF_at_end(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	if (BF_get_line_number(buffptr) == BF_number_lines(buffptr))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}





/*  BF_get_line()  -  Copy the contents of the current line in the buffer
 *                    to a specified string and also return it's length.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               A pointer to the destination string into which the line
 *                    is to be copied
 *
 *  Returns:  The length of the string that was copied.
 *
 *  Notes:  This function will not ensure that the destination string is
 *          NULL terminated.  The only means of determining the length of
 *          the string copied is the length value returned.
 */
int BF_get_line(BUFFER *buffptr, char *lineptr)
{
	int x;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(lineptr != NULL);
	/* */
	
	
	/* copy the characters in the current line to lineptr */
	for (x=0; x < buffptr->lines[buffptr->current_line].length; x++)
	{
		lineptr[x] = buffptr->lines[buffptr->current_line].string[x];
	}
	/* */
	
	return (buffptr->lines[buffptr->current_line].length);
}





/*  BF_insert_line_after()  -  Insert a line into the buffer after the
 *                             current line being pointed to.  Then, make
 *                             this new line the one which is currently
 *                             being pointed to.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the new line was added to the buffer
 *            FALSE if it was not possible to add another line to the
 *                 buffer
 */
bool BF_insert_line_after(BUFFER *buffptr)
{
	int count;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* do we need to do a disk access? */
	if (buffptr->buffer_lines == MEMSIZE)
	{
		if (update_buffer(buffptr, (buffptr->top_line + buffptr->current_line), MIDDLE_POS) == FALSE)
		{
			return FALSE;
		}
	}
	/* */
	
	/* open up space for the new line */
	for (count = buffptr->buffer_lines; count > buffptr->current_line; count--)
	{
		strncpy(buffptr->lines[count].string, buffptr->lines[count-1].string, LLEN);
		buffptr->lines[count].length = buffptr->lines[count-1].length;
	}
	/* */
	
	/* clear new line */
	buffptr->lines[buffptr->current_line + 1].length = 0;
	/* */
	
	/* update counters */
	buffptr->total_lines++;
	buffptr->buffer_lines++;
	buffptr->current_line++;
	/* */

	/* this page is now dirty */
	buffptr->dirty = TRUE;
	/* */
	
	return TRUE;
}





/*  BF_delete_line()  -  Delete the line which is currently pointed to from
 *                       the buffer.  The resulting line which is being
 *                       pointed to will then depend on which line was
 *                       deleted.  It will usually be the line that was
 *                       after the deleted line.  However, if the last line
 *                       in the buffer is deleted, the current line will
 *                       then be the line previous to it.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  DL_OK : if the line deleted was neither the last line in the
 *                      in the buffer nor the only line in the buffer
 *                      at the time.  The resulting line will be the one
 *                      which was previously after the line which was
 *                      deleted.
 *            DL_LAST : if the line deleted was the last line in the buffer
 *                      at the time.  The resulting line will be the one
 *                      which was previously before the line which was
 *                      deleted.
 *            DL_ONLY : if the line deleted was the only line in the buffer
 *                      at the time.  The buffer will still be left with
 *                      one line in it, but the line will be empty.
 *            DL_EMPTY : if nothing was done because the buffer was already
 *                      empty (ie. only one line was in the buffer and it's
 *                      length was 0)
 *            DL_ERR : if a line could not be deleted because there was an
 *                      error during the disk access
 *
 *  Notes:  It is impossible to delete lines so that there will ever be
 *          zero lines in the buffer.  Whenever the only line in the buffer
 *          is deleted, it's length is simply set to zero.
 */
int BF_delete_line(BUFFER *buffptr)
{
	int count;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* is this the only line in the buffer? */
	if (buffptr->total_lines == 1)
	{
		/* is this line empty? */
		if (buffptr->lines[0].length == 0)
		{
			return DL_EMPTY;	/* buffer already empty */
		}
		/* */
		
		buffptr->lines[0].length = 0;
		buffptr->dirty = TRUE;
		
		return DL_ONLY;	/* deleted the last line in the buffer */
	}
	/* */

	/* this page is going to be dirty */
	buffptr->dirty = TRUE;
	/* */
	
	/* is this the last line in the buffer we're deleting? */
	if ((buffptr->top_line + buffptr->current_line) == buffptr->total_lines)
	{
		buffptr->buffer_lines--;
		buffptr->total_lines--;
		
		/* need to update buffer via disk swap or not? */
		if ((buffptr->buffer_lines < AHEAD_POS) && (buffptr->total_lines >= AHEAD_POS))
		{
			if (update_buffer(buffptr, (buffptr->top_line + buffptr->current_line - 1), BACK_POS) == FALSE)
			{
				buffptr->buffer_lines++;
				
				return DL_ERR;	/* couldn't delete line (disk swap failed) */
			}
		}
		else
		{
			buffptr->current_line--;
		}
		/* */

		return DL_LAST;	/* line deleted was at the end of the buffer */
	}
	/* */
	
	/* begin normal delete for some line in the middle of the buffer */

	buffptr->buffer_lines--;	/* decrement line count in buffer */
	buffptr->total_lines--;
	
	/* bring up following lines */
	for (count = buffptr->current_line; count < buffptr->buffer_lines; count++)
	{
		strncpy(buffptr->lines[count].string, buffptr->lines[count+1].string, LLEN);
		buffptr->lines[count].length = buffptr->lines[count+1].length;
	}
	/* */
	
	/* should we do a buffer swap? */
	if (buffptr->buffer_lines < AHEAD_POS)
	{
		/* we can tolerate a failure in this call, but it is not a good thing */
		update_buffer(buffptr, (buffptr->top_line + buffptr->current_line), MIDDLE_POS);
	}
	/* */
	
	return DL_OK;	/* normal line deletion occured */
}





/*  BF_put_line()  -  Put a string under 80 characters into the current
 *                   line in the buffer.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               A pointer to the source string
 *               The length of the string to be copied into the current
 *                    line.
 *
 *  Returns:  none
 *
 *  Notes:  This function is only useful when the string to be copied into
 *          the buffer is less than 80 characters long.  When copying
 *          strings longer than 80 characters into the buffer, use
 *          BF_put_string(), which will wrap longer strings into several
 *          buffer lines.
 */
void BF_put_line(BUFFER *buffptr, char *string, int length)
{
	/* Assertions */
	assert(buffptr != NULL);
	assert(string != NULL);
	assert(length >= 0);
	assert(length < 80);
	/* */
	

	/* even if the line being put in has length of zero */
	/* this is alright, copy_line_to_buffer will figure it out */
	copy_line_to_buffer(buffptr, string, 0, length-1);
	/* */
}





/*  BF_empty_buffer()  -  Empty the specified buffer.  The result will be a
 *                        buffer with only one line whose length is zero.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if the buffer was emptied
 *            FALSE if the buffer could not be emptied due to an error
 *                 during a disk access
 */
bool BF_empty_buffer(BUFFER *buffptr)
{
	/* Assertions */
	assert(buffptr != NULL);
	/* */
	
	
	/* empty buffer */
	buffptr->total_lines = 1;
	buffptr->top_line = 1;
	buffptr->buffer_lines = 1;
	buffptr->current_line = 0;
	buffptr->lines[0].length = 0;
	buffptr->dirty = TRUE;
	/* */
	
	return (update_buffer(buffptr, 1, 1));
}





/*  BF_put_string()  -  Put a string of any length into the buffer
 *                      starting at the current line in the buffer.  The
 *                      function will wrap this string into possibly
 *                      several 80 character lines if necessary.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               A pointer to the source string to be inserted into the
 *                    buffer
 *               The length of the string to be inserted into the buffer
 *               A pointer to the current position of the cursor on the
 *                    line to be inserted
 *               A pointer to cursor_line, lines_put, wrap_pos and
 *                    last_line whose values will be set by this function
 *
 *  Returns:  TRUE if the string was successfully inserted into the buffer
 *            FALSE if unsuccessful due the an error while inserting the
 *                 lines into the buffer (due to disk swap error)
 *
 *  Modified values upon exit:
 *
 *       cursor_x : contains the resulting x position of the cursor on the
 *                  line in which it ends up
 *       cursor_line : contains the line on which the cursor ends up, as a
 *                     number based on the number of lines put into the
 *                     buffer (a value of 2 means that the cursor is on the
 *                     2nd line that was put into the buffer)
 *       lines_put : contains the total number of lines which were put into
 *                   the buffer, which is the number of lines which the
 *                   given string needed to be broken into to fit into
 *                   <80 character lines
 *       wrap_pos : contains the character position within the source
 *                  string at which the first wrap took place, or the end
 *                  of the source string if no wrap was necessary
 *       last_line : contains TRUE if the final line into which the string
 *                   wraps also contains the line previously following it.
 *                   Contains FALSE if the last line put into the buffer
 *                   by this function was not joined with the line
 *                   following it.
 *
 *  Notes:  All additional lines required due to wrapping of the source
 *          string will be inserted into the buffer automatically.
 *
 *          Additionally, upon a FALSE return value, any of the variables
 *          which were to be modified by the function will contain useless
 *          values.
 */
bool BF_put_string(BUFFER *buffptr, char *string, int length, int *cursor_x, int *cursor_line, int *lines_put, int *wrap_pos, bool *last_line)
{
	int xpos, startpos, wrappos, x, linenum, len1, len2;
	bool problem = FALSE;
	char line1[LLEN], line2[LLEN];
	int startline;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(string != NULL);
	assert(length >= 0);
	assert(cursor_x != NULL);
	assert(*cursor_x >= 0);
	assert(cursor_line != NULL);
	assert(lines_put != NULL);
	assert(wrap_pos != NULL);
	assert(last_line != NULL);
	/* */
	
	
	*cursor_line = 1;	/* initialize value, line that cursor ends up on */
	
	/* maybe this can all fit on one line? */
	if (length < LLEN)
	{
		copy_line_to_buffer(buffptr, string, 0, length-1);
		*lines_put = 1;
		
		return TRUE;
	}
	/* */
	
	/* ok, this string is going to need to be wrapped */
	startline = BF_get_line_number(buffptr);
	*lines_put = 0;
	startpos = 0;
	
	while (startpos < length)
	{
		/* insert a new line if possible  (if necessary) */
		if (startpos != 0)
		{
			if (BF_insert_line_after(buffptr) == FALSE)
			{
				problem = TRUE;	/* couldn't allocate another line */
				break;
			}
		}
		*lines_put = *lines_put + 1;
		/* */
		
		/* copy next portion to buffer */
		wrappos = find_wrap_position(string, startpos, length);
		if (startpos == 0)
		{
			*wrap_pos = wrappos;	/* keep the position of the first wrap */
		}
		copy_line_to_buffer(buffptr, string, startpos, wrappos-1);
		/* */
		
		/* find the ultimate cursor position */
		if ((*cursor_x >= wrappos) && (wrappos < length))
		{
			*cursor_line = *cursor_line + 1;	/* cursor is not going to end up on the line just put in */
		}
		else
		{
			if ((*cursor_x >= startpos) && (*cursor_x <= wrappos))
			{
				xpos = *cursor_x - startpos;	/* found the ultimate cursor x position */
			}
		}
		/* */
		
		startpos = wrappos;
	}
	/* */
	
	/* is there a line after the last one inserted, and should we wrap it up? */
	if (BF_get_line_number(buffptr) < BF_number_lines(buffptr))
	{
		linenum = BF_get_line_number(buffptr);	/* store number of last line added for later */
		
		/* get the length of the last line added and the length of the line after that */
		len1 = buffptr->lines[buffptr->current_line].length;
		
		BF_move_ahead(buffptr);
		
		len2 = buffptr->lines[buffptr->current_line].length;
		/* */
		
		/* can these two lines be joined into one? */
		if (((len1 + len2) > LLEN-1) || (len2 == 0))
		{
			BF_move_back(buffptr);
			*last_line = FALSE;	/* the two lines couldn't be joined */
		}
		else
		{
			/* join the last line just added with the line after it */
			BF_get_line(buffptr, line1);
			BF_delete_line(buffptr);
			BF_goto_line(buffptr, linenum);
			BF_get_line(buffptr, line2);
			for (x=0; x < len2; x++)
			{
				line2[x+len1] = line1[x];
			}
			BF_put_line(buffptr, line2, len1+len2);
			*last_line = TRUE;	/* yes, the last two lines were joined */
		}
	}
	/* */
	
	/* cleanup (if problem) */
	if (problem == TRUE)
	{
		/* delete all of the lines that were able to be added */
		for (x=0; x < *lines_put; x++)
		{
			if (BF_delete_line(buffptr) == 1)
			{
				BF_move_back(buffptr);
			}
		}
		/* */
		
		return FALSE;
	}
	/* */
	
	/* cleanup (no problem) */
	*cursor_x = xpos;	/* store new xpos for return */
	BF_goto_line(buffptr, (startline + *cursor_line - 1));
	return TRUE;
	/* */
}





/*  copy_line_to_buffer()  -  Copy a portion of a string specified into the
 *                            current line in the buffer.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               A pointer to the source string
 *               The starting position in the source string to begin
 *                    copying from
 *               The ending position in the source string at which to
 *                    stop copying
 *
 *  Returns:  none
 *
 *  Notes:  This function is internal to the Buffer I/O module.
 *
 *          The distance between first and last must be less than 80
 *          characters.
 */
void copy_line_to_buffer(BUFFER *buffptr, char *string, int first, int last)
{
	int x, length;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(string != NULL);
	assert(first >= 0);
	/* */
	
	
	length = last - first + 1;
	
	/* copy chars to current line in buffer */
	for (x=0; ((x < length) && (x < 80)); x++)
	{
		buffptr->lines[buffptr->current_line].string[x] = string[first++];
	}
	/* */
	
	/* set length of the line */
	buffptr->lines[buffptr->current_line].length = length;
	/* */

	/* this page is now dirty */
	buffptr->dirty = TRUE;
	/* */
}





/*  find_wrap_position()  -  Find the next position in a string at which to
 *                           perform a word wrap, given a position in the
 *                           string at which to begin searching.
 *
 *  Parameters:  A pointer to the string to be checked
 *               The starting position at which to begin looking for the
 *                    next position to wrap at
 *               The length in characters of the entire string which was
 *                    passed.
 *
 *  Returns:  The next position at which to wrap, or the length of the
 *            string, if no further wrapping needs to be done in order to
 *            fit the remaining portion into 80 character lines
 *
 *  Notes:  This function is internal to the Buffer I/O module.
 *
 *          When searching for a position to wrap at, the last space before
 *          the limit of 80 characters is selected.  If such a character
 *          does not exist, the line will be cut off at the end anyway,
 *          wherever that may lie.
 */
int find_wrap_position(char *string, int startpos, int length)
{
	int pos;
	
	
	/* Assertions */
	assert(string != NULL);
	assert(startpos >= 0);
	assert(length >= startpos);
	/* */
	
	
	pos = startpos;
	
	if ((length - startpos) < LLEN)	/* can all the rest can fit in one line? */
	{
		return length;
	}
	
	/* we have to find a point to wrap at */
	pos = startpos + LLEN-2;
	while ((string[pos] != ' ') && (pos > startpos))
	{
		pos--;
	}
	
	if (string[pos] == ' ')
	{
		return pos+1;
	}
	else
	{
		return (startpos + LLEN-1);
	}
}





/*  update_buffer()  -  Update the current buffer by copying the contents
 *                      of the internal memory page to a temp file with the
 *                      rest of the buffer on disk, and then retrieving the
 *                      required new page contents.
 *
 *  Parameters:  A pointer to the corresponding buffer control structure
 *               The line which is to be the new current line in the buffer
 *                    after the page swap is complete
 *               The position within the internal page at which this
 *                    current line is preferred to reside.
 *
 *  Returns:  TRUE if successful (no errors swapping pages)
 *            FALSE if there was an error swapping page to disk.
 *
 *  Notes:  The desired position of the current line in the internal page
 *          must be less than 75, even though the page is actually 100
 *          long.  This is necessary because the last 25 lines are reserved
 *          so the page size may grow without constantly requiring a disk
 *          swap as each additional line is added to it.
 *          (the numbers used here are relative, and are multiplied by
 *          some factor to obtain optimal performance)
 *
 *          The resulting position of the current line may not necessarily
 *          be the same as the position passed as a parameter.  (ie. if the
 *          desired position is 50 and there are only 30 lines in total in
 *          the buffer at the time it will be impossible to position the
 *          current line at 50).  The resulting position will always be as
 *          close as possible to the desired position though.
 *          (The desired position feature is implemented to allow other
 *          Buffer I/O functions to attempt to optimize page handling
 *          by guessing about the user's future movements based on current
 *          movements, in order to reduce disk accesses)
 */
bool update_buffer(BUFFER *buffptr, int line, int position)
{
	char *filenameproto = "/usr/tmp/.QWXXXXXX", newfilename[19];
	FILE *newfp;
	int count, pos;
	char c;
	
	
	/* Assertions */
	assert(buffptr != NULL);
	assert(line >= 1);
	assert(line <= buffptr->total_lines);
	assert(position >= 0);
	assert(position < UPDATE_SIZE);
	/* */

	/* if the page in memory is clean, we don't need to write it back */
	if (buffptr->dirty == TRUE)
	{
		/* create the new temp file */
		strcpy(newfilename, filenameproto);
		newfp = (FILE *)fopen(mktemp(newfilename),"w+");	/* open a new file with a unique name */
		if (newfp == NULL)
		{
			return FALSE;
		}
		/* */
	
		/* go to start of prev. temp file */
		fseek(buffptr->fp, 0, SEEK_SET);
		/* */
	
		/* copy the part of the prev. temp file before what is in the memory buffer */
		for (count=1; count < buffptr->top_line; count++)
		{
			do
			{
				c = fgetc(buffptr->fp);
				if (fputc(c, newfp) == EOF)
				{
					fclose(newfp);
					unlink(newfilename);
					return FALSE;	/* error occured writing new temp file */
				}
			}
			while (c != '\n');
		}
		/* */
	
		/* skip past the lines in the prev. temp file that were held in the internal buffer */
		for (count=0; count < buffptr->prev_lines; count++)
		{
			do
			{
				c = fgetc(buffptr->fp);
			}
			while(c != '\n');
		}
		/* */
	
		/* copy the contents of the memory buffer to the new temp file */
		for (count = 0; count < buffptr->buffer_lines; count++)
		{
			for (pos=0; pos < buffptr->lines[count].length; pos++)
			{
				if (fputc(buffptr->lines[count].string[pos],newfp) == EOF)
				{
					/* error occured writing new temp file, so exit */
					fclose(newfp);
					unlink(newfilename);
					return FALSE;
				}
			}
		
			if (fputc('\n', newfp) == EOF)
			{
				/* error occured writing new temp file, so exit */
				fclose(newfp);
				unlink(newfilename);
				return FALSE;
			}
		}
		/* */
	
		/* copy the rest of the old temp file to the new temp file */
		for (count = (buffptr->top_line + buffptr->buffer_lines - 1); count < buffptr->total_lines; count++)
		{
			do
			{
				c = fgetc(buffptr->fp);
				if (fputc(c, newfp) == EOF)
				{
					/* error occured writing new temp file, so exit */
					fclose(newfp);
					unlink(newfilename);
					return FALSE;
				}
			}
			while (c != '\n');
		}
		/* */
	
		/* close & delete old temp file */
		fclose(buffptr->fp);
		unlink(buffptr->filename);
		/* */
	
		/* link new temp file into control structure */
		buffptr->fp = newfp;
		strcpy(buffptr->filename, newfilename);
		/* */

		/* reset dirty bit to clean again */
		buffptr->dirty = FALSE;
		/* */
	}
	/* */
	
	/* find new top_line */
	if (line > position)
	{
		buffptr->top_line = line - position;
	}
	else
	{
		buffptr->top_line = 1;
	}
	/* */
	
	/* find line in temp file to start reading into memory buffer at */
	fseek(buffptr->fp, 0, SEEK_SET);
	for (count=1; count < buffptr->top_line; count++)
	{
		do
		{
			c = fgetc(buffptr->fp);
		}
		while (c != '\n');
	}
	/* */
	
	/* determine how many line can be read into memory (as close to 75 as possible) */
	if ((buffptr->total_lines - buffptr->top_line) > UPDATE_SIZE)
	{
		buffptr->buffer_lines = UPDATE_SIZE;
	}
	else
	{
		buffptr->buffer_lines = buffptr->total_lines - buffptr->top_line + 1;
	}
	/* */
	
	/* read these lines into the memory buffer from the temp file */
	for (count=0; count < buffptr->buffer_lines; count++)
	{
		buffptr->lines[count].length = 0;
		while ((c = fgetc(buffptr->fp)) != '\n')
		{
			buffptr->lines[count].string[buffptr->lines[count].length++] = c;
		}
	}
	/* */
	
	/* determine value for current_line */
	buffptr->current_line = line - buffptr->top_line;
	/* */
	
	/* store the current buffer size now, so we can skip these lines in the temp file later */
	buffptr->prev_lines = buffptr->buffer_lines;
	/* */
	
	return TRUE;
}





/*  BF_initiate_load()  -  Put the buffer into a state where text can be put
 *                         directly to the temp file, bypassing the use of the
 *                         memory buffer.  This function is implemented in
 *                         order to speed up the loading time of a text file,
 *                         as well as to increase the speed of Cuts and Copies
 *                         to the clipboard.
 *
 *  Paramaters:  a pointer to the buffer control structure to be put into
 *                    loading state
 *
 *  Returns:  TRUE if successful, FALSE if unsuccessful
 *
 *  Note:  The results obtained from the use of this function (in combination
 *         with BF_finish_load() and BF_load_string() could be obtained using
 *         the normal Buffer I/O functions.  However, due to the number of
 *         page swaps that would be required to load in a large file and the
 *         resulting delay, these functions have been added.
 *
 *         This function MUST be followed by a call to BF_finish_load().
 */
bool BF_initiate_load(BUFFER *buffptr)
{
	/* close old temp file and delete it */
	fclose(buffptr->fp);
	unlink(buffptr->filename);
	/* */

	/* setup the buffer control structure */
	buffptr->total_lines = 0;
	buffptr->current_line = 0;
	buffptr->top_line = 1;
	buffptr->buffer_lines = 1;
	buffptr->prev_lines = 0;
	strcpy(buffptr->filename, "/usr/tmp/.QWXXXXXX");
	/* */

	/* open the new temp file for writing */
	buffptr->fp = (FILE *)fopen(mktemp(buffptr->filename), "w+");
	if (buffptr->fp == NULL)
	{
		return FALSE;
	}
	/* */

	return TRUE;
}





/*  BF_finish_load()  -  Put the buffer back into regular access mode after a
 *                       call to BF_initiate_load().  This function closes the
 *                       temp file opened for writing by BF_initiate_load()
 *                       and then does a page swap to get part of the buffer
 *                       into the memory page.
 *
 *  Parameters:  a pointer to the corresponding buffer control structure
 *
 *  Returns:  TRUE if successful, FALSE if unsuccessful
 */
bool BF_finish_load(BUFFER *buffptr)
{
	int x, c;

	/* close the temp buffer and reopen it for reading */
	fflush(buffptr->fp);
	fclose(buffptr->fp);
	buffptr->fp = fopen(buffptr->filename, "r+");
	if (buffptr->fp == NULL)
	{
		return FALSE;
	}
	/* */

	/* fill the memory page with the first bit of the file */
	for (x=0; ((x < buffptr->total_lines) && (x < UPDATE_SIZE)); x++)
	{
		buffptr->lines[x].length = 0;
		while ((c = fgetc(buffptr->fp)) != '\n')
		{
			buffptr->lines[x].string[buffptr->lines[x].length++] = c;
		}
	}
	/* */

	/* fix values */
	if (buffptr->total_lines == 0)
	{
		buffptr->total_lines = 1;
		buffptr->buffer_lines = 1;
		buffptr->prev_lines = 0;
	}
	else
	{
		buffptr->buffer_lines = x;
		buffptr->prev_lines = x;
	}
	/* */

	return TRUE;
}





/*  copy_line_to_tempfile()  -  This function is called by BF_load_string().
 *                              It does exactly the same thing in principle
 *                              as copy_line_to_buffer(), but it stores the
 *                              string straight to the temp file.
 *
 *  Parameters:  a pointer to the corresponding buffer control structure
 *               a pointer to the string from which characters are to be taken
 *                    and put into the file
 *               the position within the file of the first character to be
 *                    used, and the position of the last character to be copied
 *                    as well
 *
 *  Returns:  TRUE if successful, FALSE otherwise
 *
 *  Notes:  This function is internal to this module.
 */
bool copy_line_to_tempfile(BUFFER *buffptr, char *string, int first, int last)
{
	int x, length;

	/* Assertions */
	assert(buffptr != NULL);
	assert(string != NULL);
	assert(first >= 0);
	/* */

	length = last - first + 1;

	/* copy chars to temp file */
	for (x=0; ((x < length) && (x < 80)); x++)
	{
		if (fputc(string[first++], buffptr->fp) == EOF)
		{
			fclose(buffptr->fp);
			unlink(buffptr->filename);
			return FALSE;
		}
	}
	if (fputc('\n', buffptr->fp) == EOF)
	{
		fclose(buffptr->fp);
		unlink(buffptr->filename);
		return FALSE;
	}
	/* */

	buffptr->total_lines = buffptr->total_lines + 1;

	return TRUE;
}





/*  BF_load_string()  -  This function is identical in use to BF_put_string(),
 *                       except that this function will copy the given string
 *                       straight to the temp file of the buffer.
 *
 *  Parameters:  a pointer to the corresponding buffer control structure
 *               a pointer to the string to be put into the file
 *               the length of the string passed to the function
 *
 *  Returns:  TRUE if successful, FALSE if unsuccessful
 *
 *  Notes:  This function can only be used after a call to BF_initiate_load()
 *          and a series of calls to this function must be ultimately followed
 *          up by a call to BF_finish_load().
 */
bool BF_load_string(BUFFER *buffptr, char *string, int length)
{
	int startpos, wrappos;

	/* Assertions */
	assert(buffptr != NULL);
	assert(string != NULL);
	assert(length >= 0);
	/* */

	/* maybe this can all fit on one line? */
	if (length < 80)
	{
		if (copy_line_to_tempfile(buffptr, string, 0, length-1) == FALSE)
		{
			return FALSE;
		}

		return TRUE;
	}
	/* */

	/* ok, this string is going to need to be wrapped */
	startpos = 0;

	while (startpos < length)
	{
		/* copy next portion to buffer */
		wrappos = find_wrap_position(string, startpos, length);
		if (copy_line_to_tempfile(buffptr, string, startpos, wrappos-1) == FALSE)
		{
			return FALSE;
		}
		startpos = wrappos;
		/* */
	}
	/* */

	return TRUE;
}





/*  BF_delete_lines()  -  Quickly delete a series of lines from the buffer.
 *                        This function can quickly delete a series of lines
 *                        from the buffer by rewriting the temp file without
 *                        the specified lines and then doing a page swap.  The
 *                        current line in the buffer will be the line that was
 *                        previously after the deleted lines.
 *
 *  Parameters:  a pointer to teh corresponding buffer control structure
 *               the first of the lines to be copied
 *               the last of the lines to be copied
 *
 *  Returns:  TRUE if successful, FALSE if unsuccessful
 *
 *  Notes:  This function is used by the block operation Cut.
 */
bool BF_delete_lines(BUFFER *buffptr, int first, int last)
{
	char *filenameproto = "/usr/tmp/.QWXXXXXX", newfilename[19];
	FILE *newfp;
	int count;
	char c;

	/* Assertions */
	assert(buffptr != NULL);
	assert(first > 1);
	assert(last < buffptr->total_lines);
	/* */

	/* do a page swap if necessary to ensure temp file matches buffer */
	if (buffptr->dirty == TRUE)
	{
		if (update_buffer(buffptr, first, MIDDLE_POS) == FALSE)
		{
			return FALSE;
		}
	}
	/* */

	/* create the new temp file */
	strcpy(newfilename, filenameproto);
	newfp = (FILE *)fopen(mktemp(newfilename), "w+");
	if (newfp == NULL)
	{
		return FALSE;
	}
	/* */

	/* go to start of prev. temp file */
	fseek(buffptr->fp, 0, SEEK_SET);
	/* */

	/* copy the part of the prev temp file before the area to be cut */
	for (count=1; count < first; count++)
	{
		do
		{
			c = fgetc(buffptr->fp);
			if (fputc(c, newfp) == EOF)
			{
				fclose(newfp);
				unlink(newfilename);

				return FALSE;
			}
		}
		while (c != '\n');
	}
	/* */

	/* skip past the lines in the prev. temp file that are to be cut */
	for (count=0; count < (last - first + 1); count++)
	{
		do
		{
			c = fgetc(buffptr->fp);
		}
		while (c != '\n');
	}
	/* */

	/* copy the rest of the old temp file to the new one */
	for (count=last; count < buffptr->total_lines; count++)
	{
		do
		{
			c = fgetc(buffptr->fp);
			if (fputc(c, newfp) == EOF)
			{
				/* error occured while writing new temp file */
				fclose(newfp);
				unlink(newfilename);
				return FALSE;
			}
		}
		while (c != '\n');
	}
	/* */

	/* close and delete old temp file */
	fclose(buffptr->fp);
	unlink(buffptr->filename);
	/* */

	/* link new temp file into control structure */
	buffptr->fp = newfp;
	strcpy(buffptr->filename, newfilename);
	/* */

	/* find new top line */
	if (first-1 > MIDDLE_POS)
	{
		buffptr->top_line = first-1-MIDDLE_POS;
	}
	else
	{
		buffptr->top_line = 1;
	}
	/* */

	/* find line in temp file to start reading into memory buffer at */
	fseek(buffptr->fp, 0, SEEK_SET);
	for (count=1; count < buffptr->top_line; count++)
	{
		do
		{
			c = fgetc(buffptr->fp);
		}
		while (c != '\n');
	}
	/* */

	/* determine how many lines can be read into memory */
	buffptr->total_lines = buffptr->total_lines - (last-first+1);
	if ((buffptr->total_lines - buffptr->top_line) > UPDATE_SIZE)
	{
		buffptr->buffer_lines = UPDATE_SIZE;
	}
	else
	{
		buffptr->buffer_lines = buffptr->total_lines - buffptr->top_line + 1;
	}
	/* */

	/* read those line into the memory buffer from the temp file */
	for (count=0; count < buffptr->buffer_lines; count++)
	{
		buffptr->lines[count].length = 0;
		while ((c = fgetc(buffptr->fp)) != '\n')
		{
			buffptr->lines[count].string[buffptr->lines[count].length++] = c;
		}
	}
	/* */

	/* determine value for current_line */
	buffptr->current_line = first-1 - buffptr->top_line;
	/* */

	/* store the current buffer size now, so we can skip these lines in the temp file later */
	buffptr->prev_lines = buffptr->buffer_lines;
	/* */

	return TRUE;
}

