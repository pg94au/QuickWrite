/* QuickWrite                      */
/* (c)1993,1994 Insync Development */
/*                                 */
/* qw_defs.h                       */

#ifndef QW_DEFS_H
#define QW_DEFS_H

#ifndef CURSES_H	/* curses.h also typedefs char as bool */
typedef char bool;
#endif


/* define some common characters that we'll be reading in */

#define ctrl(c)	((c) & 037)
#define KEY_SWITCH	ctrl('a')
#define KEY_REDRAW	ctrl('l')
#define KEY_FILENAME	ctrl('f')
#define KEY_RETURN	10		/* for some reason KEY_ENTER as */
                                        /* defined in curses.h does not */
                                        /* work with all terminals...   */
#define KEY_DELETE	127
#define KEY_DELETE2	330
#define KEY_BACKSPACE2	8
#define KEY_LEFT2	269	/* keypad cursor keys */
#define KEY_RIGHT2	271
#define KEY_UP2		274
#define KEY_DOWN2	350
#define KEY_PAGE_UP	264
#define KEY_PAGE_UP2	339
#define KEY_PAGE_DOWN	349
#define KEY_PAGE_DOWN2	338
#define KEY_HOME2	273
#define KEY_END2	348
#define KEY_END3	332
#define KEY_BLINE	ctrl('b')
#define KEY_ELINE	ctrl('e')
#define KEY_TAB		9
#define KEY_DELETE_LINE	11	/* CTRL-K */
#define hide_cursor()	move(0,79)
#ifndef TRUE		/* define TRUE and FALSE if necessary */
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* */


/* define the size in of the terminal display in characters */

/* this version of the program has fixed values for the screen size */
#define SCREEN_WIDTH	80
#define SCREEN_HEIGHT	24

/* the size of the editing window */
#define EWH		22
#define EWW		80

/* the length of a line used by this editor */
#define LLEN		80

/* */


/* define the modes available in the program (selectable from the menus, etc.) */

#define QUIT			0
#define EDIT_MODE		1
#define MENU_MODE		2
#define OPEN			3
#define SAVE			4
#define SAVE_AS			5
#define SAVE_CLIPBOARD_AS	6
#define ABOUT			7
#define CUT			8
#define COPY			9
#define PASTE			10
#define INSERT			11
#define MARK_BLOCK		12
#define SEARCH			13
#define SEARCH_NEXT		14
#define SEARCH_AND_REPLACE	15
#define DISPLAY_LINE_NUMBER	16
#define JUMP_TO_LINE		17
#define BUFFER_STATUS		18
#define HELP_ME			19
#define SWITCH_BACK		20

/* */


/* define the following mask values for setting menu state */

#define M_OPEN			1
#define M_SAVE			2
#define M_SAVE_AS		4
#define M_SAVE_CLIPBOARD_AS	8
#define M_ABOUT			16
#define M_QUIT			32
#define M_CUT			64
#define M_COPY			128
#define M_PASTE			256
#define M_INSERT		512
#define M_MARK_BLOCK		1024
#define M_SEARCH		2048
#define M_SEARCH_NEXT		4096
#define M_SEARCH_REPLACE	8192
#define M_DISPLAY_LINE_NUMBER	16384
#define M_JUMP_TO_LINE		32768
#define M_BUFFER_STATUS		65536
#define M_HELP			131072

/* */

#endif	/* QW_DEFS_H */

