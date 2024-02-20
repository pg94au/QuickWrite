/*
 * qw_fileio.h  ---  headerfile
 *
 */

#ifndef QW_FILEIO_H
#define QW_FILEIO_H

bool FL_open_named_file(BUFFER *buffptr, char *name[64]);
int  FL_open_file(BUFFER *buffptr);
int  FL_save_file(BUFFER *buffptr);
int  FL_save_file_as(BUFFER *buffptr);
int  FL_quit(BUFFER *buffptr);
int  FL_save_clipboard_as(BUFFER *buffptr);

#endif

