/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_ewhand.h
 */



#ifndef QW_EWHAND_H	/* prevent multiple inlcudes */
#define QW_EWHAND_H

#include "qw_buffio.h"


/* function prototypes */

void EW_initial_draw(BUFFER *buffptr);
void EW_refresh(void);
void EW_touch(void);
void EW_update_position(BUFFER *buffptr);
void EW_get_cursor_pos(int *cur_x, int *des_x, int *cur_y);
void EW_set_cursor_pos(int new_cursor_x, int new_desired_x);
void EW_insert_char(int c);
void EW_delete_char(void);
void EW_redraw_char(BUFFER *buffptr, int mode);
void EW_insert_line(BUFFER *buffptr, int x);
void EW_delete_line(BUFFER *buffptr, int code);
void EW_update_lines(BUFFER *buffptr, int new_cursor_x, int cursor_line, int lines_put, int wrap_pos, bool last_line);
bool EW_reposition_cursor(BUFFER *buffptr, int new_cursor_y);
void EW_redraw_window(BUFFER *buffptr);

/* */

#endif	/* QW_EWHAND_H */

