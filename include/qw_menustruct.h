/*  QuickWrite                      */
/*  (c)1993,1994 Insync Development */
/*                                  */
/*  qw_menustruct.h                 */


struct menustruct menus[5] =
{
	{
		4,1,TRUE,3,1,17,6," FILE ",
		{	"Open             ",
			"Save             ",
			"Save As          ",
			"Save Clipboard As",
			"About            ",
			"Quit             ",
			NULL, NULL, NULL, NULL	},
		{	TRUE, FALSE, FALSE, FALSE, TRUE, TRUE,
			NULL, NULL, NULL, NULL	},
		{	OPEN, SAVE, SAVE_AS, SAVE_CLIPBOARD_AS, ABOUT, QUIT,
			NULL, NULL, NULL, NULL	}
	},
	{
		0,2,FALSE,0,8,10,5," EDIT ",
		{	"Cut       ",
			"Copy      ",
			"Paste     ",
			"Insert    ",
			"Mark Block",
			NULL, NULL, NULL, NULL, NULL	},
		{	FALSE, FALSE, FALSE, FALSE, FALSE,
			NULL, NULL, NULL, NULL, NULL	},
		{	CUT, COPY, PASTE, INSERT, MARK_BLOCK,
			NULL, NULL, NULL, NULL, NULL	}
	},
	{
		1,3,FALSE,0,15,18,3," SEARCH ",
		{	"Search            ",
			"Search Next       ",
			"Search And Replace",
			NULL, NULL, NULL, NULL, NULL, NULL, NULL	},
		{	FALSE, FALSE, FALSE,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL	},
		{	SEARCH, SEARCH_NEXT, SEARCH_AND_REPLACE,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL	}
	},
	{
		2,4,TRUE,3,24,19,3," TOOLS ",
		{	"Display Line Number",
			"Jump To Line       ",
			"Buffer Status      ",
			NULL, NULL, NULL, NULL, NULL, NULL, NULL	},
		{	TRUE, TRUE, TRUE,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL	},
		{	DISPLAY_LINE_NUMBER, JUMP_TO_LINE, BUFFER_STATUS,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL	}
	},
	{
		3,0,TRUE,1,32,7,1," HELP ",
		{	"Help Me",
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			NULL	},
		{	TRUE,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			NULL	},
		{	HELP_ME,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			NULL	}
	}
};

