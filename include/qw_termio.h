/*
 *	QuickWrite
 *	(c)1993,1994 Insync Development
 *
 *	qw_termio.h
 */


#ifndef QW_TERM_H	/* prevent multiple includes */
#define QW_TERM_H

#include <stdio.h>
#include "qw_defs.h"


/* function prototypes */

bool TM_initialize_display(void);
void TM_close_display(void);
void TM_redraw_screen(void);
void TM_write_statusbar(char *string);
void TM_write_menubar(char *string);
bool TM_read_string(int xpos, int maxlength, char *string);
bool TM_read_value(int xpos, int maxlength, int *value);
void TM_wait_bar(int xpos);

/* */

#endif	/* QW_TERM_H */

