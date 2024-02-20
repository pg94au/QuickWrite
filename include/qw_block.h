/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_block.h
 */


#ifndef QW_BLOCK_H	/* prevent multiple includes */
#define QW_BLOCK_H

#include "qw_buffio.h"


struct mark {
	int line, pos;
};
 

/* function prototypes */

int BL_mark_block(BUFFER *buffptr, BUFFER *clipptr);
int BL_continue_marking(BUFFER *buffptr, BUFFER *clipptr);
int BL_cut_block(BUFFER *buffptr, BUFFER *clipptr);
int BL_paste_block(BUFFER *buffptr, BUFFER *clipptr);
int BL_insert_block(BUFFER *buffptr, BUFFER *clipptr);
void draw_block(BUFFER *buffptr);
void unmark_block(BUFFER *buffptr);
void redraw_chars(BUFFER *buffptr, struct mark *mark1, struct mark *mark2, int mode);

/* */

#endif	/* QW_BLOCK_H */

