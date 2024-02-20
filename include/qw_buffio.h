/*
 *	QuickWrite
 *	(c)1993,1994 Insync Development
 *
 *	qw_buffio.h
 */


#ifndef QW_BUFF_H	/* prevent multiple includes */
#define QW_BUFF_H

#include <stdio.h>
#include "qw_defs.h"

#ifndef MULT
#define MULT 5
#endif
#define MEMSIZE		MULT*100


/* define the structure of a single line of text */

struct linestruct {
	char string[LLEN];	/* store for a line of text */
	int length;		/* count of chars on corresponding line */
};

/* */


/* defint the structure of a buffer control structure */

struct bufferstruct {
	int total_lines;	/* total line stored */
	int top_line;		/* number of the first line in the memory buffer */
	int current_line;	/* offset in memory buffer of the current line */
	int buffer_lines;	/* number of lines currently held in memory buffer */
	int prev_lines;		/* number of lines read into memory buffer during last swap */
	FILE *fp;		/* pointer to file for the disk swap file */
	char filename[19];	/* actual name of the temp file on disk */
	struct linestruct lines[MEMSIZE];	/* internal memory buffer */
	bool dirty;		/* flag used to determine whether a page is memory has been changed or not */
};

typedef struct bufferstruct BUFFER;    /* typedef for convenience */

/* */


/* define return values of BF_delete_line() */

#define DL_OK		1
#define DL_LAST		0
#define DL_ONLY		-1
#define DL_EMPTY	-2
#define DL_ERR	 	-3

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
bool BF_initiate_load(BUFFER *buffptr);
bool BF_finish_load(BUFFER *buffptr);
bool BF_load_string(BUFFER *buffptr, char *string, int length);
bool BF_delete_lines(BUFFER *buffptr, int first, int last);

/* */

#endif	/* QW_BUFF_H */

