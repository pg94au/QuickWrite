/*
 * QW_unix.h  ---  headerfile
 *
 */

#ifndef QW_UNIX_H
#define QW_UNIX_H

/* function prototypes */

FILE *UX_open_file(char *filename, char mode,bool *exist);
void UX_close_file(FILE *fptr);
void UX_power_check(void);
void UX_find_IO_error(FILE *fptr, char mode);

/* */

#endif

