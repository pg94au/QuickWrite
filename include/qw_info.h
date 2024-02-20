/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_info.h
 */



#ifndef QW_INFO_H	/* prevent multiple includes */
#define QW_INFO_H

/* function prototypes */

int IF_display_line_number(BUFFER *buffptr);
int IF_about(void);
int IF_buffer_status(BUFFER *buffptr, BUFFER *clipptr);
int IF_help(void);

/* */

#endif	/* QW_INFO_H */

