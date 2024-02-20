/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_cursor.h
 */


#ifndef QW_CURSOR_H	/* prevent multiple includes */
#define QW_CURSOR_H


#include "qw_buffio.h"


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


#endif	/* QW_CURSOR_H */

