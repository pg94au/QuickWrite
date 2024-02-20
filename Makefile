# Makefile for QuickWrite
#
# (c)1993,1994 Insync Development


PRODUCT = qw
CFLAGS = -O2
OBJECTS = main.o qw_buffio.o qw_termio.o qw_error.o qw_ewhand.o qw_cursor.o qw_text.o qw_edit.o qw_menu.o qw_info.o qw_block.o qw_search.o qw_fileio.o qw_unix.o
LIBS = -lm -lc_s -lcurses

all: $(PRODUCT)

$(PRODUCT): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(PRODUCT)

clean:
	rm *.o

