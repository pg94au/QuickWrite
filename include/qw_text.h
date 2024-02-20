/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_text.h
 */


#ifndef QW_TEXT_H	/* prevent multiple includes */
#define QW_TEXT_H

#include "qw_defs.h"
#include "qw_buffio.h"

/* function prototypes */

bool TX_insert_character(BUFFER *buffptr, char c);
bool TX_delete_key(BUFFER *buffptr);
bool TX_backspace_key(BUFFER *buffptr);
bool TX_enter_key(BUFFER *buffptr);
bool TX_delete_line(BUFFER *buffptr);
bool TX_tab_key(BUFFER *buffptr);

/* */

#endif	/* QW_TEXT_H */

