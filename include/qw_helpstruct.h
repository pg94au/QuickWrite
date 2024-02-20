/* QuickWrite                      */
/* (c)1993,1994 Insync Development */
/*                                 */
/* qw_helpstruct.h                 */

#define MAX 18

char *index[MAX]={"Special Keys in the Edit Window                         ",
		  "Open                                                    ",
	   	  "Save                                                    ",
		  "Save As                                                 ",
		  "Save Clipboard As                                       ",
		  "About                                                   ",
		  "Quit                                                    ",
		  "Cut                                                     ",
		  "Copy                                                    ",
		  "Paste                                                   ",
		  "Insert                                                  ",
		  "Mark Block                                              ",
		  "Search                                                  ",
		  "Search Next                                             ",
		  "Search and Replace                                      ",
		  "Display Line Number                                     ",
		  "Jump to Line                                            ",
		  "Buffer Status                                           "};

/* Switches Used in text are:
 *
 * \v : Signifies the beginning of the document.
 * \r : Signifies that BOLD is to be turned on.
 * \f : Signifies that NORMAL is to be turned on.
 * \n : Signifies a new line.
 * \t : Signifies the end of the document.
 * \b : Signifies a page break.
 */
char *text[MAX]={"\v\n     => \rCTRL-L\f - This will redraw the screen which is"
		 " being currently\n                 displayed.  This is"
		 " usefull over the modem, when line\n                 noise" 
		 " is encountered.  This is a way to restore the screen\n"
		 "                 to its original format.\n\n"
		 "     => \rCTRL-K\f - This will delete the current line of the"
		 " document.  As a\n                 result this will put the"
		 " cursor to the beginning of the\n                 line that"
		 " was brought up.\n\n"
		 "     While in the edit window there are many different ways"
		 " in which you\ncan move the cursor.\n\n"
		 "     => \rthe 4 cursor keys\f - These will move the position"
		 " of the cursor by\n                            one either in"
		 " the left, right, up, and down\n                            "
                 "direction.\b"
		 "\n     => \rPgUp\f - This will put the cursor to the top line"
		 " of the current\n               screen.  If the cursor is"
		 " already at the top line, it will\n               display"
		 " the previous screenful of text.\n\n"
		 "     => \rPgDn\f - This will put the cursor to the bottom"
		 " line of the current\n               screen.  If the cursor"
		 " is already at the bottom line, it\n               "
		 "will display the next screenful of text.\n\n"
		 "     => \rHome\f - This will put the cursor to the first"
		 " line of the document,\n               while placing the"
		 " cursor on the first character.\n\n"
		 "     => \rEnd\f - This will put the cursor to the last line"
		 " of the document,\n              while placing the cursor"
		 " on the last character.\n\n"
		 "     => \rCTRL-B\f - This will put the cursor to the"
		 " beginning of the current\n"
		 "                 line.\b"
		 "\n     => \rCTRL-E\f - This will put the cursor to the end of"
		 " the current line. \n\n"
		 "\rNOTE:\f  All these different ways of moving in the edit"
		 " window will cause a\n       beep if it couldn't successfully"
		 " be done.  For example, if you are\n       already on the"
		 " first line and first characer and you press Home.\n"
		 "       This will cause a beep"
		 " because you are already in home position.\t",
		 "\v\n     => select \rOPEN\f from the \rFILE\f menu.\n\n"
		 "     The open command gives you the choice of either opening"
		 " a new file or\nopeining a file that already exists.\n\n"
		 "     You can also open a file from the command line, the"
		 " format should look\nlike:\n\n     => \rqw <filename>\f\t",
		 "\v\n     => select \rSAVE\f from the \rFILE\f menu.\n\n"
  		 "     This will save the contents of the main buffer to a file"
		 " that has\nalready been specified.  If no file has been"
		 " specified a prompt will occur\non the bottom of the screen"
		 " asking for a filename.\t",
		 "\v\n     => select \rSAVE AS\f from the \rFILE\f menu.\n\n"
		 "     This will save the contents of the main buffer to a file"
		 " whose name\nyou must specify.\t",
		 "\v\n     => select \rSAVE CLIPBOARD AS\f from the \rFILE\f"
		 " menu.\n\n     When any cutting or copying is done to the "
		 "file (\rCUT\f and \rCOPY\f is\nexplained in another topic)"
		 " that text will be stored in the Clipboard until\nyou exit"
		 " QuickWrite.  The \rSAVE CLIPBOARD AS\f option will allow"
		 " you to save\nthe contents of the clipboard to a file which"
		 " you must specify.\t",
		 "\v\n     => select \rABOUT\f from the \rFILE\f menu.\n\n"
		 "     This will display the general information about the"
		 " product release\nand about the developers.\t",
		 "\v\n     => select \rQUIT\f from the \rFILE\f menu.\n\n"
		 "     The \rQUIT\f command will automatically exit you from"
		 " the editor and back\ninto the operating system.  While you"
		 " are quiting, QuickWrite will ask you\nif you really want to"
		 " quit.  If the file you have loading has been modified\nand"
		 " you select quit, it will ask you if you would like to save"
		 " the changes\nthat have been make to the file before you"
		 " quit.\t",
		 "\v\n     => select \rCUT\f from the \rEDIT\f menu.\n\n"
		 "     The \rCUT\f command will cut out a section of text"
		 " which has been selected\n(\rMARK BLOCK\f is explained in"
		 " another topic).  The selected text that will\nbe cut will"
		 " be stored in the Clipboard buffer.\n\n\rNOTE:\f  "
		 " Before you can execute a \rCUT\f you must have executed"
		 " \rMARK BLOCK\f.\t",
		 "\v\n     => select \rCOPY\f from the \rEDIT\f menu.\n\n"
		 "     The \rCOPY\f command will copy the selected section of"
		 " text(\rMARK BLOCK\f is\nexplained in another topic).  The"
		 " selected text that will be copied will\nbe stored in the"
		 " Clipboard buffer.\n\n\rNOTE:\f  Before you can execute a"
		 " \rCOPY\f you must have executed \rMARK BLOCK\f.\n"
		 "       The \rCOPY\f command will not delete the text from the"
		 " main buffer.\t", 
		 "\v\n     => select \rPASTE\f from the \rEDIT\f menu.\n\n"
		 "     The \rPASTE\f command will copy whatever is in the"
		 " Clipboard buffer onto\nthe main buffer starting where your"
		 " cursor is.  The \rPASTE\f command will also\ninclude all"
		 " carrriage returns.  An implication of this is that no word"
		 "\nwrapping will be done on the text that was pasted.\n\n"
		 "\rNOTE:\f  Before you can execute a \rPASTE\f you must have"
		 " executed \rCUT\f or \rCOPY\f.\t",
		 "\v\n     => select \rINSERT\f from the \rEDIT\f menu.\n\n"
		 "     The \rINSERT\f command will copy whatever is in the"
		 " Clipboard buffer onto\nthe main buffer startind where your"
		 " cursor is.  The \rINSERT\f command will not\ninclude any"
		 " carriage returns.  An implication of this is that word "
		 "wrapping\nwill be done on the text that was inserted.\n\n"
		 "\rNOTE:\f  Before you can execute a \rINSERT\f you must have"
		 " executed \rCUT\f or \rCOPY\f.\t",
		 "\v\n     => select \rMARK BLOCK\f from the \rEDIT\f menu.\n\n"
		 "     The \rMARK BLOCK\f command will allow you to mark text"
		 "(select or\nhighlight).  To use the \rMARK BLOCK\f command"
		 " follow these simple step:\n\n     => \rPlace your cursor"
		 " where you want to start marking.\f\n     => \rSelect the "
		 "MARK BLOCK from the EDIT menu.\f\n     => \rYou will now"
		 " be placed in edit mode.\f\n     => \rYou can now use all the"
		 " cursor movement keys to select the\f\n        \rhighlighted"
		 " text.\f\n"
		 "     => \rWhen you have selected all the text you wanted,"
		 " switch into menu\n        \rmode.\f\n     => \rYou can now"
		 " select CUT or COPY or switch back into edit mode.\f\n"
		 "     => \rIf you switch back into edit mode you can continue"
		 " to update the\f\n        \rmarked block.\f\b\n\rNOTE:\f  "
		 "While you are marking text, if you press any other key other"
		 " than\n       cursor movement keys the highlighted text will"
		 " go away.\n       While marking a block you are only allowed"
		 " to execute these\n       following commands:  \rCUT\f,"
		 " \rCOPY\f, \rMARK BLOCK\f, \rDISPLAY LINE NUMBER\f,"
		 "            \rHELP ME\f.\t",
		 "\v\n     => select \rSEARCH\f from the \rSEARCH\f menu.\n\n"
		 "     The \rSEARCH\f command allows you to input a word,"
		 " character, or phrase,\nand it will find the first occurence"
		 " of the specifec string you entered.\n\n"
		 "     If the \rSEARCH\f string is not found, QuickWrite will"
		 " ask you if you\nwould like to start from the beginning of"
		 " the document.\n\n"
		 "     The maximum length of the search string can be 50"
		 " characters.\n\n"
		 "     The search string can contain any legitimate character"
		 " that you can\ntype in edit mode except for tabs and carriage"
		 " returns.\n\n"
		 "     Sometimes the search can take some time, therefore a"
		 " spinning bar is\nincluded showing you that the editor is"
		 " working.\n\n     If the string is found it will be high"
		 "lighted.\t",
		 "\v\n     => select \rSEARCH NEXT\f from the \rSEARCH\f menu."
		 "\n\n     The \rSEARCH NEXT\f command allows you to enter a"
		 " string to be searched for if you haven't already.\n\n"
		 "     \rSEARCH NEXT\f will search for the first occurence"
		 " of the string.  Once\nfound it will ask you if you would"
		 " like to search for the next occurence.\n\n"
		 "     If the \rSEARCH\f string is not found, QuickWrite will"
                 " ask you if you\nwould like to start from the beginning of"
                 " the document.\n\n"
                 "     The maximum length of the search string can be 50"
                 " characters.\n\n"
                 "     The search string can contain any legitimate character"
                 " that you can\ntype in edit mode except for tabs and carriage"
                 " returns.\b"
                 "\n     Sometimes the search can take some time, therefore a"
                 " spinning bar is\nincluded showing you that the editor is"
                 " working.\n\n     If the string is found it will be high"
                 "lighted.\t",          
		 "\v\n     => select \rSEARCH AND REPLACE\f from the \r"
		 "SEARCH\f menu.\n\n"
		 "     The \rSEARCH AND REPLACE\f command will ask you for a"
		 " search string and a\nreplace string.\n\n"
		 "     If the search string is found it will ask you if you"
		 " would like to\nreplace it with the replace string.\n\n"
		 "     The maximum length of the search and replace string"
		 " can be 50\ncharacters.\n\n"
                 "     The search and replace string can contain any"
		 " legitimate character\nthat you can"
                 " type in edit mode except for tabs and carriage"
                 " returns.\n\n"
                 "     Sometimes the search can take some time, therefore a"
                 " spinning bar is\nincluded showing you that the editor is"
                 " working.\b\n     If the string is found it will be high"
                 "lighted.\n\n     If the search and replace string are the"
		 " same nothing will happen.\t",
		 "\v\n     => select \rDISPLAY LINE NUMBER\f from the \r"
		 "TOOLS\f menu.\n\n"
		 "     The \rDISPLAY LINE NUMBER\f command will display the"
		 " line number of the\nline where the cursor is currently"
		 " located on.\t", 
		 "\v\n     => select \rJUMP TO LINE NUMBER\f from the \r"
		 "TOOLS\f menu.\n\n"
		 "     The \rJUMP TO LINE NUMBER\f command will ask the user"
		 " for which line they\nwish to jump to.  If the user enters"
		 " a line number which is not valid a\nbeep will be sent.\t",
		 "\v\n     => select \rBUFFER STATUS\f from the \rTOOLS\f.\n\n"
		 "     The \rBUFFER STATUS\f command will open up a window"
		 " containing information about the buffer and clipboard.\n\n" 
		 "     The information that it will present sill be Word"
		 " Count, Line Count,\nByte Count, and if the buffer has been"
		 " modified.\t"};    
