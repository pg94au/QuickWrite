/*
 *  QuickWrite V1.00
 *
 *  (c)1993,1994 Insync Development
 *
 *  main.c
 */


#include <curses.h>	/* this won't be here in the end product! */
#include "include/qw_defs.h"
#include "include/qw_error.h"
#include "include/qw_buffio.h"
#include "include/qw_edit.h"
#include "include/qw_menu.h"
#include "include/qw_unix.h"
#include <assert.h>



BUFFER *buffptr, *clipptr;


/* function prototypes */

int main(int argc, char *argv[]);
bool initialize_buffers(void);
void free_buffers(void);
bool load_buffer(int argc, char *argv[]);

/* */



/*  main()  -  The entry point into the program upon execution.
 *             Initializes everything that needs to be set up in order to
 *             run the program.  Handles the switching between the edit and
 *             menu modes in the program.
 *
 *  Parameters:  Command line arguments may include the name of a file to
 *                    be read into the buffer upon entry into the editor
 *
 *  Returns:  TRUE if the program terminated successfully
 *            FALSE if the program terminated due to some error
 */
int main(int argc, char *argv[])
{
	int mode = EDIT_MODE;	/* contains return value for next state */

	/* initialize buffers */
	if (initialize_buffers() == FALSE)
	{
		exit(1);
	}
	/* */

	/* initialize curses & open main windows */
	if (TM_initialize_display() == FALSE)
	{
		free_buffers();
		exit(1);
	}
	/* */

	/* load a file into buffer if specified on command line */
	load_buffer(argc, argv);
	/* */

	/* draw the initial contents of the edit window now */
	EW_initial_draw(buffptr);
	/* */

	/* set up to do the power check */
	UX_power_check();
	/* */

	/* handle switching from edit<>menu mode and quit */
	for(;;)
	{
		switch(mode)
		{
			case EDIT_MODE:
				mode = EH_edit_handler();
				break;
			case MENU_MODE:
				mode = MH_menu_handler();
				break;
			case QUIT:
				break;
		}
		if (mode==QUIT)
			break;
	}
	/* */

	/* close windows and cleanup display */
	TM_close_display();
	putchar('\n');
	putchar('\n');
	/* */

	/* cleanup buffers */
	free_buffers();
	/* */

	exit(0);	/* no problems during execution */
}





/*  initialize_buffers()  -  Initialize the main and clipboard buffers.
 *
 *  Parameters:  none
 *
 *  Returns:  TRUE if successfully allocated
 *            FALSE if unsuccessful (out of memory)
 */
bool initialize_buffers()
{
	buffptr = BF_create_buffer();	/* initialize main buffer */

	if (buffptr != NULL)	/* if first initialized ok */
	{
		clipptr = BF_create_buffer();	/* init. clipboard buffer */

		if (clipptr != NULL)	/* if second initialized ok */
		{
			return TRUE;
		}
		BF_free_buffer(buffptr);	/* if 1st not init'ed ok */
	}

	ER_put_error(ERROR_BUFFER);	/* report error to user */

	return FALSE;
}





/*  free_buffers()  -  Free the main and clipboard buffers (all memory which
 *                     was allocated to them)
 *
 *  Parameters:  none
 *
 *  Returns:  none
 */
void free_buffers()
{
	/* Assertions */
	assert(buffptr != NULL);
	assert(clipptr != NULL);
	/* */

	/* free main and clipboard buffers */
	BF_free_buffer(buffptr);
	BF_free_buffer(clipptr);
	/* */
}





/*  load_buffer()  -  Load a file if specified in the command line arguments.
 *                    Perform a check on the arguments, as they are being
 *                    passed directly from the command line, without any
 *                    check as to their contents.
 *
 *  Parameters:  none
 *
 *  Returns:  TRUE if successful (either the file WAS loaded in successfully,
 *                 or there was no file specified on the command line)
 *            FALSE if unsuccessful (could allocate enough lines to read in
 *                 entire buffer, file not found, arguments incorrect, etc.)
 */
bool load_buffer(int argc, char *argv[])
{
	/* is there no file to load in? */
	if (argc == 1)
	{
		return TRUE;
	}
	/* */

	/* are there too many arguments? */
	if (argc > 2)
	{
		ER_put_error(ERROR_COMMAND_LINE);
		return FALSE;
	}
	/* */

	/* try to load in the file */
	if (FL_open_named_file(buffptr, argv[1]) == FALSE)
	{
		return FALSE;
	}
	/* */

	return TRUE;
}

