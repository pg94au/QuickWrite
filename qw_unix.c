/*UNIX I/O MODULE:*/

/* This module is responsible for low level disk access involving
 * opening, closing and creating files.  Procedures that check for
 * power failure and IO errors will also be provided in this module.
 */
#include <stdio.h>
#include <assert.h>
#include "include/qw_defs.h"
#include "include/qw_error.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>

FILE *UX_open_file(char *filename, char mode,bool *exist);
void UX_close_file(FILE *fptr);
void UX_power_check(void);
void UX_find_IO_error(FILE *fptr, char mode);
void send_message(void);


/*Global Constants:*/
#define	maxfnam	256 	/* Maximum number of characters for a filename.	*/
	
	/*R_OK	04	 Used to check for read access.	      		*/
	/*W_OK	02	 Used to check for write access.	        */
	/*F_OK	00	 Used to check for file existence.	        */


/*Global Variables:*/
char	oldfilename[maxfnam];	/* Stores the filename requested for opening.	                */
char	tempfilename[maxfnam];	/* Stores the temporary unique filename to be chosen randomly.	*/
bool	openforw;		/* Holds TRUE if an existing file is open for writing.		*/


/* UX_open_file(filename, mode)	-	Opens the specified file for either   reading or writing
 *					depending on the mode parameter passed.
 *
 * Parameters:	filename	-	A pointer to the a filename string.
 *  
 * Returns:	fptr		-	A pointer to a structure that has been defined in   
 *					stdio.h, or NULL if the file can't be opened.
 */

FILE *UX_open_file(char *filename, char mode, bool *exist)

{
	FILE *fptr;

	/* Assertions */
	assert(filename != NULL);


	/* Does the file exist? */
	fptr = NULL;
	if (access(filename, F_OK) == 0) 
	{
		/** Is the read mode requested, and does user have the 
		    read access to the specified file? **/
		*exist = TRUE;
		if((mode == 'r') && (access(filename,R_OK)==0))
		{
			openforw = FALSE;
			fptr = fopen(filename, &mode);
		}
			
		else if ((mode == 'w') && (access(filename, W_OK)==0)) 
		{
			/*** Is the write mode requested, and does the user have
			     the write access to the specified file? ***/
		
			openforw = TRUE;
			strcpy(tempfilename, mktemp(".qwfXXXXXX"));	
			strcpy(oldfilename, filename);
			fptr = fopen(tempfilename, &mode);
			/*** ***/
		}
		
		else
		{
		}
		/** **/
					
	
	}
	/* Unix I/O Module - UX_open_file procedure cont'd. */
	
	else if (mode == 'w')
		/** If the file doesn't exist and write mode is requested, create it and return the file pointer.**/
	{	
		*exist = FALSE;
		openforw = FALSE;
		fptr = fopen(filename, &mode);
		/** **/
	}
	else
		/** File doesn't exist, thus can't open file for reading.**/
		/* no error because qw_fileio will just empty the buffer
			and treat this like an empty file*/
	{
		*exist = FALSE;
	}
	/* */
	return fptr;
		
}



/* UX_close_file(fptr)	-	Closes the specified file and will handle 
 *				any errors if they occur.
 *
 * Parameters:	fptr	-	A pointer to a file.
 *
 * Returns:	none
 */
void UX_close_file(FILE *fptr)

{
	/* Assertions */
	assert(fptr != NULL);

	if (openforw == TRUE)
	{
		fclose(fptr);
		rename(tempfilename,oldfilename);
	}
	/* */

	/*If EOF is returned after closing the file, send error code to
	  Error Handler Module (using ER_put_error procedure).*/
	else if (fclose(fptr) != 0)
	{
		ER_put_error(ERCLOSE);
	}
	/* */
}




/*  send_message(void) - 	Procedure that calls the ER_put_error procedure
*				in the Error Handler Module.  This procedure
*				passes an integer(NOPOW) that represents the
*				appropriate error to be displayed to the user.
*   Parameters:	none
*
*   Returns   :	none
*/
void send_message(void)
{
	ER_put_error(NOPOW);
	UX_power_check();
}


/*  UX_power_check(void)	-	Called from Main Module as part of the 
 *				initialization of the program upon execution.
 *				It's purpose is to detect a power failure
 *				and to inform the user if this occurs.
 *
 *  Parameters:	none
 *
 *  Returns   :	none
 */
void UX_power_check(void)
{
	signal(SIGPWR, send_message);
}


/* UX_find_IO_error(fptr, mode)	-	This procedure determines the error that occurred during
 *					any file I/O operation(reading or writing).  The appropriate
 *					error code is sent to the Error Handler Module and an 
 *					attempt is made to recover(or/and clean partially written
 *					files).
 *
 *  Parameters: 	fptr	-	A pointer to the file being accessed.
 *			mode	-	The mode that was in use when the error occurred.
 *
 *  Returns   :	none
 */
void UX_find_IO_error(FILE *fptr, char mode)

{
	/* Assertions */
	assert(fptr != NULL);
	/*assert((mode == 'r') OR (mode == 'w'));*/

	/* If the error occurred while reading the file, close it and send proper error code to the 
	   Error Handler Module. */
	if(mode == 'r') 
	{
		UX_close_file(fptr);
		ER_put_error(ERREAD);
	}
	/* */

	/* If the error occurred while writing to the tempfile, close the file, delete it, and send the 
	   proper error code to the Error Handler. */
	if(mode == 'w')
	{ 
		UX_close_file(fptr);
		unlink(tempfilename);	/* Delete the partially written tempfile. */
		ER_put_error(ERWRITE);
	}
	/* */
}

