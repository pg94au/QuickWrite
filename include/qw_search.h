/*
 *  QuickWrite
 *
 *  (c)1993,1994 Insync Development
 *
 *  qw_search.h
 */



#ifndef QW_SEARCH_H	/* prevent multiple includes */
#define QW_SEARCH_H

int SR_search(BUFFER *buffptr);
int SR_search_next(BUFFER *buffptr);
int SR_search_replace(BUFFER *buffptr);

#endif	/* QW_SEARCH_H */

