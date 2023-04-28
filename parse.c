/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/

/***********************************************************************
 *
 * Merge of piewm 1.04 with tvtwm-pl11
 * Portions Copyright 2010 Peter C Tribble <peter.tribble@gmail.com>
 *
 ***********************************************************************/

/***********************************************************************
 *
 * $XConsortium: parse.c,v 1.52 91/07/12 09:59:37 dave Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 * 10-Oct-90 David M. Sternlicht       Storing saved colors on root
 ***********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/SysUtil.h>
#include "twm.h"
#include "screen.h"
#include "menus.h"
#include "util.h"
#include "gram.h"
#include "parse.h"
#include <X11/Xatom.h>
#include "vdt.h"

/* For m4... */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* m4 on 386 4.4-bsd boxes doesn't wanna read from "-"... */
/* XXX - is this a good-enough check?  BSD4_4 should be set on all */
/*       net2 based systems... */
#ifdef BSD4_4
# define M4STDIN "/dev/stdin"
#else
# define M4STDIN "-"
#endif

#ifndef SYSTEM_INIT_DIR
#define SYSTEM_INIT_DIR "/usr/openwin/lib/X11/twm"
#endif
#define BUF_LEN 300

static FILE *twmrc;
static FILE *menufile;
static int ptr = 0;
static int len = 0;
static char buff[BUF_LEN+1];
static char overflowbuff[20];		/* really only need one */
static int overflowlen;
static char **stringListSource, *currentString;
static int ParseUsePPosition();
static int ParseState();
static FILE *start_m4();
int m4_pid;

/*
 * If you're using flex (and lex on 386 bsd boxes, which *is* flex, really)
 * then you need to run flex (/lex) with the -l argument to cause maximum
 * compatibility with AT&T lex, including the definition of yylineno...
 */
extern int yylineno;
extern int mods;
extern int PieMenuWait;
extern char *menu_name;
extern MenuRoot *input_menu;
extern char *InitFile;

int ConstrainedMoveTime = 400;		/* milliseconds, event times */

static int twmFileInput(), twmStringListInput(), twmMenuInput();
void twmUnput();
int (*twmInputFunc)();

extern char *defTwmrc[];        /* default bindings */


/***********************************************************************
 *
 *  Procedure:
 *  ParseTwmrc - parse the .twmrc file
 *
 *  Inputs:
 *  filename  - the filename to parse.  A NULL indicates $HOME/.twmrc
 *
 ***********************************************************************
 */

static int doparse (ifunc, srctypename, srcname)
	int (*ifunc)();
	char *srctypename;
	char *srcname;
{
	extern FILE *yyin;

	yyin = twmrc;

	mods = 0;
	ptr = 0;
	len = 0;
	yylineno = 1;
	ParseError = FALSE;
	twmInputFunc = ifunc;
	overflowlen = 0;

	yyparse();

	if (ParseError) {
		fprintf(stderr, "%s: errors found in twm %s",
			ProgramName, srctypename);
		if (srcname) fprintf (stderr, " \"%s\"", srcname);
		fprintf (stderr, "\n");
	}
	return (ParseError ? 0 : 1);
}

int ParseMenuFile(filename, menuname)
char *filename;
char *menuname;
{
    int status = 1;

    if ((menufile=fopen(filename, "r")) == NULL)
	return 0;

    menu_name = menuname;
    status = doparse(twmMenuInput, "menu file", filename);
    fclose(menufile);
    if (input_menu == NULL) {
	fprintf(stderr, "%s: errors found in menu file %s\n", ProgramName,
		filename);
	status = 0;
    }

    return status;
}

int ParseTwmrc (filename)
	char *filename;
{
	int i;
	char *home = NULL;
	int homelen = 0;
	char *cp = NULL;
	char tmpfilename[257];
	static FILE *raw;
	extern Bool RunM4;		/* Whether or not to run m4 */

	/*
	 * Look for a configuration file to parse.
	 *
	 * If filename given, try it, else try first a screen-specific
	 * and then generic version of the .rc file named after the
	 * executable we're currently running, then piewm, tvwtm, and twm,
	 * then try the system rc file for this program, then the system
	 * rc file for twm.
	 *
	 *   0.  -f filename
	 *   1.  ."progname"rc.#
	 *   2.  ."progname"rc
	 *   3.  .piewmrc.#
	 *   4.  .piewmrc
	 *   5.  .tvtwmrc.#
	 *   6.  .tvtwmrc
	 *   7.  .twmrc.#
	 *   8.  .twmrc
	 *   9.  system."progname"rc
	 *   10.  system.twmrc
	 */
	for (twmrc = NULL, i = 0; !twmrc && i < 11; i++) {
	switch (i) {
	  case 0:           /* -f filename */
		cp = filename;
		break;

	  case 1:           /* ~/."progname"rc.screennum */
		if (!filename) {
			home = getenv ("HOME");
			if (home) {
				homelen = strlen (home);
				cp = tmpfilename;
				(void) sprintf (tmpfilename, "%s/.%src.%d",
					home, PtvtwmName, Scr->screen);
				break;
			}
		}
		continue;

	  case 2:			/* ~/."progname"rc */
		if (home) {
			tmpfilename[homelen + strlen(PtvtwmName) +4] = '\0';
		}
		break;

	  case 3:           /* ~/.piewmrc.screennum */
		if (!filename) {
			home = getenv ("HOME");
			if (home) {
				homelen = strlen (home);
				cp = tmpfilename;
				(void) sprintf (tmpfilename, "%s/.piewmrc.%d",
					home, Scr->screen);
				break;
			}
		}
		continue;

	  case 4:			/* ~/.piewmrc */
		if (home) {
			tmpfilename[homelen + 9] = '\0';
		}
		break;

	  case 5:           /* ~/.tvtwmrc.screennum */
		if (!filename) {
			home = getenv ("HOME");
			if (home) {
				homelen = strlen (home);
				cp = tmpfilename;
				(void) sprintf (tmpfilename, "%s/.tvtwmrc.%d",
					home, Scr->screen);
				break;
			}
		}
		continue;

	  case 6:			/* ~/.tvtwmrc */
		if (home) {
			tmpfilename[homelen + 9] = '\0';
		}
		break;

	  case 7:			/* ~/.twmrc.screennum */
		if (!filename) {
			home = getenv ("HOME");
			if (home) {
				homelen = strlen (home);
				cp = tmpfilename;
				(void) sprintf (tmpfilename, "%s/.twmrc.%d",
					home, Scr->screen);
				break;
			}
		}
		continue;

	  case 8:			/* ~/.twmrc */
		if (home) {
			tmpfilename[homelen + 7] = '\0';
		}
		break;

	  case 9:			/* system."progname"rc */
		cp = tmpfilename;
		(void) sprintf (tmpfilename, "%s/system.%src",
					SYSTEM_INIT_DIR, PtvtwmName);
		break;

	  case 10:			/* system.twmrc */
		cp = tmpfilename;
		(void) sprintf (tmpfilename, "%s/system.twmrc",
					SYSTEM_INIT_DIR);
		break;
	}

	if (cp) {
		raw = fopen (cp, "r");
		twmrc = raw;
	}
	}

    if (raw) {
		int status;

		InitFile = strcpy(malloc(strlen(cp)+1),cp);

		if (filename && cp != filename) {
			fprintf (stderr,
				"%s:  unable to open twmrc file %s, using %s instead\n",
				ProgramName, filename, cp);
		}
		if (RunM4)
		    twmrc = start_m4(raw);
		else
		    twmrc = raw;
		status = doparse (twmFileInput, "file", cp);
		if (RunM4)
		    fclose (twmrc);
		fclose (raw);
		if (input_menu) {
		    fprintf(stderr, "%s: found menu file when expecting twmrc\n",
				ProgramName);
		    status = 1;
		}
		return status;
	} else {
	if (filename) {
	    fprintf (stderr,
	"%s:  unable to open twmrc file %s, using built-in defaults instead\n",
		     ProgramName, filename);
	}
	return ParseStringList (defTwmrc);
    }
}

int ParseStringList (sl)
    char **sl;
{
    stringListSource = sl;
    currentString = *sl;
    return doparse (twmStringListInput, "string list", (char *)NULL);
}


/***********************************************************************
 *
 *  Procedure:
 *	twmFileInput - redefinition of the lex input routine for file input
 *
 *  Returned Value:
 *	the next input character
 *
 ***********************************************************************
 */

#ifdef NO_M4

/* This has Tom's include() funtionality.  This is utterly useless if you
 * can use m4 for the same thing.		Chris P. Ross */

#define MAX_INCLUDES 10

static struct incl {
     FILE *fp;
     char *name;
     int lineno;
} rc_includes[MAX_INCLUDES];
static int include_file = 0;

static int twmFileInput()
{
    if (overflowlen) return (int) overflowbuff[--overflowlen];

    while (ptr == len)
    {
	while (include_file) {
	     if (fgets(buff, BUF_LEN, rc_includes[include_file].fp) == NULL) {
		  free(rc_includes[include_file].name); 
		  fclose(rc_includes[include_file].fp);
		  yylineno = rc_includes[include_file--].lineno;
	     } else 
		  break; 
	}
	if (!include_file)
	     if (fgets(buff, BUF_LEN, twmrc) == NULL)
		  return 0;

	yylineno++;

	if (strncmp(buff, "include", 7) == 0) {
	     /* Whoops, an include file! */
	     char *p = buff + 7, *q;
	     FILE *fp;
	     
	     while (isspace(*p)) p++;
	     for (q = p; *q && !isspace(*q); q++)
		  continue;
	     *q = 0;

	     if ((fp = fopen(p, "r")) == NULL) {
		  fprintf(stderr, "%s: Unable to open included init file %s\n", 
			  ProgramName, p);
		  continue;
	     }
	     if (++include_file >= MAX_INCLUDES) {
		  fprintf(stderr, "%s: init file includes nested too deep\n",
			  ProgramName);
		  continue;
	     }
	     rc_includes[include_file].fp = fp;
	     rc_includes[include_file].lineno = yylineno;
	     yylineno = 0;
	     rc_includes[include_file].name = malloc(strlen(p)+1);
	     strcpy(rc_includes[include_file].name, p);
	     continue;
	}
	ptr = 0;
	len = strlen(buff);
    }
    return ((int)buff[ptr++]);
}
#else

/* If you're going to use m4, use this version instead.  Much simpler.
 * m4 ism's credit to Josh Osborne (stripes) */

static int twmFileInput()
{
	if (overflowlen) return((int) overflowbuff[--overflowlen]);

	while (ptr == len) {
		if (fgets(buff, BUF_LEN, twmrc) == NULL) {
			/* Interrupted?  Probably just the delivery of *
			 * a SIGCHLD.  We don't care, so keep going.   *
			 *             (cross@eng.umd.edu  19-Aug-93)  */
			if (errno == EINTR) {
			    if (fgets(buff, BUF_LEN, twmrc) == NULL)
				return(0);
			} else {
				return(0);
			}
		}

		yylineno++;

		ptr = 0;
		len = strlen(buff);
	}
	return ((int)buff[ptr++]);
}
#endif  /* NO_M4 */

char *current_input_line()
{
    return buff;
}

static int twmStringListInput()
{
    if (overflowlen) return (int) overflowbuff[--overflowlen];

    /*
     * return the character currently pointed to
     */
    if (currentString) {
	unsigned int c = (unsigned int) *currentString++;

	if (c) return c;		/* if non-nul char */
	currentString = *++stringListSource;  /* advance to next bol */
	return '\n';			/* but say that we hit last eol */
    }
    return 0;				/* eof */
}

static int twmMenuInput()
{
    int c;

    if (overflowlen)
	return (int)overflowbuff[--overflowlen];

    if ((c=getc(menufile)) == EOF)
	return 0;
    else if (c == '\n')
	yylineno++;

    return c;
}

/***********************************************************************
 *
 *  Procedure:
 *	twmUnput - redefinition of the lex unput routine
 *
 *  Inputs:
 *	c	- the character to push back onto the input stream
 *
 ***********************************************************************
 */

void twmUnput (c)
    int c;
{
    if (overflowlen < sizeof overflowbuff) {
	overflowbuff[overflowlen++] = (char) c;
    } else {
	twmrc_error_prefix ();
	fprintf (stderr, "unable to unput character (%d)\n",
		 c);
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	TwmOutput - redefinition of the lex output routine
 *
 *  Inputs:
 *	c	- the character to print
 *
 ***********************************************************************
 */

void
TwmOutput(c)
{
    putchar(c);
}


/**********************************************************************
 *
 *  Parsing table and routines
 * 
 ***********************************************************************/

typedef struct _TwmKeyword {
    char *name;
    int value;
    int subnum;
} TwmKeyword;

#define kw0_NoDefaults			1
#define kw0_AutoRelativeResize		2
#define kw0_ForceIcons			3
#define kw0_NoIconManagers		4
#define kw0_OpaqueMove			5
#define kw0_InterpolateMenuColors	6
#define kw0_NoVersion			7
#define kw0_SortIconManager		8
#define kw0_NoGrabServer		9
#define kw0_NoMenuShadows		10
#define kw0_NoRaiseOnMove		11
#define kw0_NoRaiseOnResize		12
#define kw0_NoRaiseOnDeiconify		13
#define kw0_DontMoveOff			14
#define kw0_NoBackingStore		15
#define kw0_NoSaveUnders		16
#define kw0_RestartPreviousState	17
#define kw0_ClientBorderWidth		18
#define kw0_NoTitleFocus		19
#define kw0_RandomPlacement		20
#define kw0_DecorateTransients		21
#define kw0_ShowIconManager		22
#define kw0_NoCaseSensitive		23
#define kw0_NoRaiseOnWarp		24
#define kw0_WarpUnmapped		25
#define kw0_ShowVirtualNames		26
#define kw0_StickyAbove			27
#define kw0_StayUpMenus			28
#define kw0_PannerOpaqueScroll		29
#define kw0_ListRings			30
#define kw0_DontInterpolateTitles	31
#define kw0_WrapVirtual			32
#define kw0_NoOpaqueMoveSaveUnders	33
#define kw0_RememberScreenPosition	34
#ifdef RJC
# define kw0_DoDefaultMenuAction	35
#endif /* RJC */

#define kws_UsePPosition		1
#define kws_IconFont			2
#define kws_ResizeFont			3
#define kws_MenuFont			4
#define kws_TitleFont			5
#define kws_IconManagerFont		6
#define kws_UnknownIcon			7
#define kws_IconDirectory		8
#define kws_MaxWindowSize		9
#define kws_VirtualDesktop		10
#define kws_PannerState			11
#define kws_PannerGeometry		12
#define kws_VirtualFont			15
#define kws_MenuTitleFont               16
#define kws_Identification		17
/* Use this?  Eh, I don't know... */
#define kws_AfterSetupRun		18

#define kwn_ConstrainedMoveTime		1
#define kwn_MoveDelta			2
#define kwn_XorValue			3
#define kwn_FramePadding		4
#define kwn_TitlePadding		5
#define kwn_ButtonIndent		6
#define kwn_BorderWidth			7
#define kwn_IconBorderWidth		8
#define kwn_TitleButtonBorderWidth	9
#define kwn_PannerScale			10
#define kwn_ScrollDistanceX		11
#define kwn_ScrollDistanceY		12
#define kwn_MenuLineWidth               13
#define kwn_TitleFontPadding            14
#define kwn_PopupSensitivity		15
#define kwn_MenuShadowWidth		16
#define kwn_PieMenuWait			17

#define kwcl_BorderColor		1
#define kwcl_IconManagerHighlight	2
#define kwcl_BorderTileForeground	3
#define kwcl_BorderTileBackground	4
#define kwcl_TitleForeground		5
#define kwcl_TitleBackground		6
#define kwcl_IconForeground		7
#define kwcl_IconBackground		8
#define kwcl_IconBorderColor		9
#define kwcl_IconManagerForeground	10
#define kwcl_IconManagerBackground	11
#define kwcl_VirtualForeground		12
#define kwcl_VirtualBackground		13

#define kwc_DefaultForeground		1
#define kwc_DefaultBackground		2
#define kwc_MenuForeground		3
#define kwc_MenuBackground		4
#define kwc_MenuTitleForeground		5
#define kwc_MenuTitleBackground		6
#define kwc_MenuShadowColor		7
#define kwc_VirtualDesktopForeground	8
#define kwc_VirtualDesktopBackground	9
#define kwc_PannerForeground		10
#define kwc_PannerBackground		11

#define kwp_TitleHighlight              1
#define kwp_TitleHighlightLeft          2
#define kwp_TitleHighlightRight         3
#define kwp_PannerBackgroundPixmap      4
#define kwp_VirtualDesktopBackgroundPixmap 5
#define kwp_IconifyPixmap		6
#define kwp_PullRightPixmap		7
#define kwp_ShadowPixmap		8

#define kwm_Name			LTYPE_NAME
#define kwm_ResName			LTYPE_RES_NAME
#define kwm_ResClass			LTYPE_RES_CLASS

/*
 * The following is sorted alphabetically according to name (which must be
 * in lowercase and only contain the letters a-z).  It is fed to a binary
 * search to parse keywords.
 */
static TwmKeyword keytable[] = { 
/* Again, do I really want to implement AfterSetupRun? */
    { "aftersetuprun",		SKEYWORD, kws_AfterSetupRun },
    { "all",			ALL, 0 },
    { "autoraise",		AUTO_RAISE, 0 },
    { "autorelativeresize",	KEYWORD, kw0_AutoRelativeResize },
    { "bordercolor",		CLKEYWORD, kwcl_BorderColor },
    { "bordertilebackground",	CLKEYWORD, kwcl_BorderTileBackground },
    { "bordertileforeground",	CLKEYWORD, kwcl_BorderTileForeground },
    { "borderwidth",		NKEYWORD, kwn_BorderWidth },
    { "button",			BUTTON, 0 },
    { "buttonindent",		NKEYWORD, kwn_ButtonIndent },
    { "c",			CONTROL, 0 },
    { "center",			JKEYWORD, J_CENTER },
    { "clientborderwidth",	KEYWORD, kw0_ClientBorderWidth },
    { "color",			COLOR, 0 },
    { "constrainedmovetime",	NKEYWORD, kwn_ConstrainedMoveTime },
    { "control",		CONTROL, 0 },
    { "cursors",		CURSORS, 0 },
    { "decoratetransients",	KEYWORD, kw0_DecorateTransients },
    { "defaultbackground",	CKEYWORD, kwc_DefaultBackground },
    { "defaultforeground",	CKEYWORD, kwc_DefaultForeground },
    { "defaultfunction",	DEFAULT_FUNCTION, 0 },
    { "destroy",		KILL, 0 },
#ifdef RJC
    { "dodefaultmenuaction",	KEYWORD, kw0_DoDefaultMenuAction},
#endif
    { "donticonifybyunmapping",	DONT_ICONIFY_BY_UNMAPPING, 0 },
    { "dontinterpolatetitles",	KEYWORD, kw0_DontInterpolateTitles },
    { "dontmoveoff",		KEYWORD, kw0_DontMoveOff },
    { "dontsqueezeicon",	DONT_SQUEEZE_ICON, 0 },
    { "dontsqueezetitle",	DONT_SQUEEZE_TITLE, 0 },
    { "east",			DKEYWORD, D_EAST },
    { "f",			FRAME, 0 },
    { "f.autoraise",		FKEYWORD, F_AUTORAISE },
    { "f.backiconmgr",		FKEYWORD, F_BACKICONMGR },
    { "f.beep",			FKEYWORD, F_BEEP },
    { "f.bottomzoom",		FKEYWORD, F_BOTTOMZOOM },
    { "f.circledown",		FKEYWORD, F_CIRCLEDOWN },
    { "f.circleup",		FKEYWORD, F_CIRCLEUP },
    { "f.colormap",		FSKEYWORD, F_COLORMAP },
    { "f.constrainedmove",	FKEYWORD, F_CONSTRAINEDMOVE },
    { "f.cut",			FSKEYWORD, F_CUT },
    { "f.cutfile",		FKEYWORD, F_CUTFILE },
    { "f.deiconify",		FKEYWORD, F_DEICONIFY },
    { "f.delete",		FKEYWORD, F_DELETE },
    { "f.deleteordestroy",	FKEYWORD, F_DELETEORDESTROY },
    { "f.deltastop",		FKEYWORD, F_DELTASTOP },
    { "f.destroy",		FKEYWORD, F_DESTROY },
    { "f.downiconmgr",		FKEYWORD, F_DOWNICONMGR },
    { "f.exec",			FSKEYWORD, F_EXEC },
    { "f.file",			FSKEYWORD, F_FILE },
    { "f.focus",		FKEYWORD, F_FOCUS },
    { "f.forcemove",		FKEYWORD, F_FORCEMOVE },
    { "f.forwiconmgr",		FKEYWORD, F_FORWICONMGR },
    { "f.fullzoom",		FKEYWORD, F_FULLZOOM },
    { "f.function",		FSKEYWORD, F_FUNCTION },
    { "f.hbzoom",		FKEYWORD, F_BOTTOMZOOM },
    { "f.hideiconmgr",		FKEYWORD, F_HIDELIST },
    { "f.horizoom",		FKEYWORD, F_HORIZOOM },
    { "f.htzoom",		FKEYWORD, F_TOPZOOM },
    { "f.hzoom",		FKEYWORD, F_HORIZOOM },
    { "f.iconify",		FKEYWORD, F_ICONIFY },
    { "f.identify",		FKEYWORD, F_IDENTIFY },
    { "f.lefticonmgr",		FKEYWORD, F_LEFTICONMGR },
    { "f.leftzoom",		FKEYWORD, F_LEFTZOOM },
    { "f.lower",		FKEYWORD, F_LOWER },
    { "f.menu",			FSKEYWORD, F_MENU },
    { "f.menufunc",		FSKEYWORD, F_MENUFUNC },
    { "f.move",			FKEYWORD, F_MOVE },
    { "f.nexticonmgr",		FKEYWORD, F_NEXTICONMGR },
    { "f.nop",			FKEYWORD, F_NOP },
    { "f.opaquemove",		FKEYWORD, F_OPAQUEMOVE },
    { "f.pandown",		FKEYWORD, F_SCROLLDOWN },
    { "f.panleft",		FKEYWORD, F_SCROLLLEFT },
    { "f.panner",		FKEYWORD, F_PANNER },
    { "f.panright",		FKEYWORD, F_SCROLLRIGHT },
    { "f.panup",		FKEYWORD, F_SCROLLUP },
    { "f.piemenu",		FSKEYWORD, F_PIEMENU },
    { "f.previconmgr",		FKEYWORD, F_PREVICONMGR },
    { "f.quit",			FKEYWORD, F_QUIT },
    { "f.raise",		FKEYWORD, F_RAISE },
    { "f.raiselower",		FKEYWORD, F_RAISELOWER },
    { "f.refresh",		FKEYWORD, F_REFRESH },
    { "f.relativemove",		FSKEYWORD, F_RELATIVEMOVE },
    { "f.relativeresize",	FKEYWORD, F_RELATIVERESIZE },
    { "f.resize",		FKEYWORD, F_RESIZE },
    { "f.restart",		FKEYWORD, F_RESTART },
    { "f.righticonmgr",		FKEYWORD, F_RIGHTICONMGR },
    { "f.rightzoom",		FKEYWORD, F_RIGHTZOOM },
    { "f.saveyourself",		FKEYWORD, F_SAVEYOURSELF },
    { "f.scroll",		FSKEYWORD, F_SCROLL },
    { "f.scrollback",		FKEYWORD, F_SCROLLBACK },
    { "f.scrolldown",		FKEYWORD, F_SCROLLDOWN },
    { "f.scrollhome",		FKEYWORD, F_SCROLLHOME },
    { "f.scrollleft",		FKEYWORD, F_SCROLLLEFT },
    { "f.scrollright",		FKEYWORD, F_SCROLLRIGHT },
    { "f.scrollup",		FKEYWORD, F_SCROLLUP },
    { "f.showiconmgr",		FKEYWORD, F_SHOWLIST },
    { "f.sorticonmgr",		FKEYWORD, F_SORTICONMGR },
    { "f.source",		FSKEYWORD, F_BEEP },  /* XXX - don't work */
    { "f.stick",		FKEYWORD, F_STICK },
    { "f.test",                 FKEYWORD, F_TESTEXEC },
    { "f.title",		FKEYWORD, F_TITLE },
    { "f.topzoom",		FKEYWORD, F_TOPZOOM },
    { "f.twmrc",		FKEYWORD, F_RESTART },
    { "f.unfocus",		FKEYWORD, F_UNFOCUS },
    { "f.upiconmgr",		FKEYWORD, F_UPICONMGR },
    { "f.version",		FKEYWORD, F_VERSION },
    { "f.vlzoom",		FKEYWORD, F_LEFTZOOM },
    { "f.vrzoom",		FKEYWORD, F_RIGHTZOOM },
    { "f.warpring",		FSKEYWORD, F_WARPRING },
    { "f.warpto",		FSKEYWORD, F_WARPTO },
    { "f.warptoiconmgr",	FSKEYWORD, F_WARPTOICONMGR },
    { "f.warptoscreen",		FSKEYWORD, F_WARPTOSCREEN },
    { "f.winexec",		FSKEYWORD, F_WINEXEC },
    { "f.winrefresh",		FKEYWORD, F_WINREFRESH },
    { "f.wintest",		FSKEYWORD, F_TESTWINEXEC },
    { "f.zoom",			FKEYWORD, F_ZOOM },
    { "forceicons",		KEYWORD, kw0_ForceIcons },
    { "frame",			FRAME, 0 },
    { "framepadding",		NKEYWORD, kwn_FramePadding },
    { "function",		FUNCTION, 0 },
    { "i",			ICON, 0 },
    { "icon",			ICON, 0 },
    { "iconbackground",		CLKEYWORD, kwcl_IconBackground },
    { "iconbordercolor",	CLKEYWORD, kwcl_IconBorderColor },
    { "iconborderwidth",	NKEYWORD, kwn_IconBorderWidth },
    { "icondirectory",		SKEYWORD, kws_IconDirectory },
    { "iconfont",		SKEYWORD, kws_IconFont },
    { "iconforeground",		CLKEYWORD, kwcl_IconForeground },
    { "iconifybyunmapping",	ICONIFY_BY_UNMAPPING, 0 },
    { "iconifypixmap",		PKEYWORD, kwp_IconifyPixmap },
    { "iconmanagerbackground",	CLKEYWORD, kwcl_IconManagerBackground },
    { "iconmanagerdontshow",	ICONMGR_NOSHOW, 0 },
    { "iconmanagerfont",	SKEYWORD, kws_IconManagerFont },
    { "iconmanagerforeground",	CLKEYWORD, kwcl_IconManagerForeground },
    { "iconmanagergeometry",	ICONMGR_GEOMETRY, 0 },
    { "iconmanagerhighlight",	CLKEYWORD, kwcl_IconManagerHighlight },
    { "iconmanagers",		ICONMGRS, 0 },
    { "iconmanagershow",	ICONMGR_SHOW, 0 },
    { "iconmgr",		ICONMGR, 0 },
    { "iconregion",		ICON_REGION, 0 },
    { "icons",			ICONS, 0 },
    { "icontitle",		ICON_TITLE, 0 },
    { "identification",		SKEYWORD, kws_Identification },
    { "interpolatemenucolors",	KEYWORD, kw0_InterpolateMenuColors },
    { "l",			LOCK, 0 },
    { "left",			JKEYWORD, J_LEFT },
    { "lefttitlebutton",	LEFT_TITLEBUTTON, 0 },
    { "listrings",	        KEYWORD, kw0_ListRings },
    { "lock",			LOCK, 0 },
    { "m",			META, 0 },
    { "maketitle",		MAKE_TITLE, 0 },
    { "maxwindowsize",		SKEYWORD, kws_MaxWindowSize },
    { "menu",			MENU, 0 },
    { "menubackground",		CKEYWORD, kwc_MenuBackground },
    { "menufont",		SKEYWORD, kws_MenuFont },
    { "menuforeground",		CKEYWORD, kwc_MenuForeground },
    { "menulinewidth",		NKEYWORD, kwn_MenuLineWidth },
    { "menushadowcolor",	CKEYWORD, kwc_MenuShadowColor },
    { "menushadowwidth",	NKEYWORD, kwn_MenuShadowWidth },
    { "menutitlebackground",	CKEYWORD, kwc_MenuTitleBackground },
    { "menutitlefont",		SKEYWORD, kws_MenuTitleFont },
    { "menutitleforeground",	CKEYWORD, kwc_MenuTitleForeground },
    { "meta",			META, 0 },
    { "mod",			META, 0 },  /* fake it */
    { "monochrome",		MONOCHROME, 0 },
    { "move",			MOVE, 0 },
    { "movedelta",		NKEYWORD, kwn_MoveDelta },
    { "name",			MKEYWORD, kwm_Name },
    { "nobackingstore",		KEYWORD, kw0_NoBackingStore },
    { "nocasesensitive",	KEYWORD, kw0_NoCaseSensitive },
    { "nodefaults",		KEYWORD, kw0_NoDefaults },
    { "nograbserver",		KEYWORD, kw0_NoGrabServer },
    { "nohighlight",		NO_HILITE, 0 },
    { "noiconmanagers",		KEYWORD, kw0_NoIconManagers },
    { "noicontitle",		NO_ICON_TITLE, 0 },
    { "nomenushadows",		KEYWORD, kw0_NoMenuShadows },
    { "noopaquemovesaveunders",	KEYWORD, kw0_NoOpaqueMoveSaveUnders },
    { "noraiseondeiconify",	KEYWORD, kw0_NoRaiseOnDeiconify },
    { "noraiseonmove",		KEYWORD, kw0_NoRaiseOnMove },
    { "noraiseonresize",	KEYWORD, kw0_NoRaiseOnResize },
    { "noraiseonwarp",		KEYWORD, kw0_NoRaiseOnWarp },
    { "north",			DKEYWORD, D_NORTH },
    { "nosaveunders",		KEYWORD, kw0_NoSaveUnders },
    { "nostackmode",		NO_STACKMODE, 0 },
    { "notitle",		NO_TITLE, 0 },
    { "notitlefocus",		KEYWORD, kw0_NoTitleFocus },
    { "notitlehighlight",	NO_TITLE_HILITE, 0 },
    { "noversion",		KEYWORD, kw0_NoVersion },
    { "opaquemove",		KEYWORD, kw0_OpaqueMove },
    { "pannerbackground",	CKEYWORD, kwc_PannerBackground },
    { "pannerbackgroundpixmap",	PKEYWORD, kwp_PannerBackgroundPixmap },
    { "pannerforeground",	CKEYWORD, kwc_PannerForeground },
    { "pannergeometry",		SKEYWORD, kws_PannerGeometry },
    { "panneropaquescroll",       KEYWORD, kw0_PannerOpaqueScroll },
    { "pannerscale",		NKEYWORD, kwn_PannerScale },
    { "pannerstate",		SKEYWORD, kws_PannerState },
    { "piemenu",		PIEMENU, 0 },
    { "piemenuwait",		NKEYWORD, kwn_PieMenuWait },
    { "pixmaps",		PIXMAPS, 0 },
    { "popupsensitivity",	NKEYWORD, kwn_PopupSensitivity },
    { "pullrightpixmap",	PKEYWORD, kwp_PullRightPixmap },
    { "r",			ROOT, 0 },
    { "randomplacement",	KEYWORD, kw0_RandomPlacement },
    { "rememberscreenposition",	KEYWORD, kw0_RememberScreenPosition },
    { "resclass",		MKEYWORD, kwm_ResClass },
    { "resize",			RESIZE, 0 },
    { "resizefont",		SKEYWORD, kws_ResizeFont },
    { "resname",		MKEYWORD, kwm_ResName },
    { "restartpreviousstate",	KEYWORD, kw0_RestartPreviousState },
    { "right",			JKEYWORD, J_RIGHT },
    { "righttitlebutton",	RIGHT_TITLEBUTTON, 0 },
    { "root",			ROOT, 0 },
    { "s",			SHIFT, 0 },
    { "savecolor",		SAVECOLOR, 0 },
    { "scrolldistancex",	NKEYWORD, kwn_ScrollDistanceX },
    { "scrolldistancey",	NKEYWORD, kwn_ScrollDistanceY },
    { "select",			SELECT, 0 },
    { "shadowpixmap",		PKEYWORD, kwp_ShadowPixmap },
    { "shift",			SHIFT, 0 },
    { "showiconmanager",	KEYWORD, kw0_ShowIconManager },
    { "showvirtualnames",	KEYWORD, kw0_ShowVirtualNames },
    { "sorticonmanager",	KEYWORD, kw0_SortIconManager },
    { "south",			DKEYWORD, D_SOUTH },
    { "squeezeicon",		SQUEEZE_ICON, 0 },
    { "squeezetitle",		SQUEEZE_TITLE, 0 },
    { "starticonified",		START_ICONIFIED, 0 },
    { "stayupmenus",		KEYWORD, kw0_StayUpMenus },
    { "sticky",			STICKY, 0 },
    { "stickyabove",		KEYWORD, kw0_StickyAbove },
    { "t",			TITLE, 0 },
    { "title",			TITLE, 0 },
    { "titlebackground",	CLKEYWORD, kwcl_TitleBackground },
    { "titlebuttonborderwidth",	NKEYWORD, kwn_TitleButtonBorderWidth },
    { "titlefont",		SKEYWORD, kws_TitleFont },
    { "titlefontpadding",       NKEYWORD, kwn_TitleFontPadding },
    { "titleforeground",	CLKEYWORD, kwcl_TitleForeground },
    { "titlehighlight",		PKEYWORD, kwp_TitleHighlight },
    { "titlehighlightleft",	PKEYWORD, kwp_TitleHighlightLeft },
    { "titlehighlightright",	PKEYWORD, kwp_TitleHighlightRight },
    { "titlepadding",		NKEYWORD, kwn_TitlePadding },
    { "unknownicon",		SKEYWORD, kws_UnknownIcon },
    { "usepposition",		SKEYWORD, kws_UsePPosition },
    { "virtualbackground",	CLKEYWORD, kwcl_VirtualBackground },
    { "virtualdesktop",		SKEYWORD, kws_VirtualDesktop },
    { "virtualdesktopbackground",CKEYWORD, kwc_VirtualDesktopBackground },
    { "virtualdesktopbackgroundpixmap",PKEYWORD,
				    kwp_VirtualDesktopBackgroundPixmap },
    { "virtualdesktopforeground",CKEYWORD, kwc_VirtualDesktopForeground },
    { "virtualfont",		SKEYWORD, kws_VirtualFont },
    { "virtualforeground",	CLKEYWORD, kwcl_VirtualForeground },
    { "w",			WINDOW, 0 },
    { "wait",			WAIT_CURS, 0 },
    { "warpcursor",		WARP_CURSOR, 0 },
    { "warpunmapped",		KEYWORD, kw0_WarpUnmapped },
    { "west",			DKEYWORD, D_WEST },
    { "window",			WINDOW, 0 },
    { "windowfunction",		WINDOW_FUNCTION, 0 },
    { "windowring",		WINDOW_RING, 0 },
    { "wrapvirtual",		KEYWORD, kw0_WrapVirtual },
    { "xorvalue",		NKEYWORD, kwn_XorValue },
    { "zoom",			ZOOM, 0 },
};

static int numkeywords = (sizeof(keytable)/sizeof(keytable[0]));

int parse_keyword (s, nump)
    char *s;
    int *nump;
{
    register int lower = 0, upper = numkeywords - 1;

    XmuCopyISOLatin1Lowered (s, s);
    while (lower <= upper) {
        int middle = (lower + upper) / 2;
	TwmKeyword *p = &keytable[middle];
        int res = strcmp (p->name, s);

        if (res < 0) {
            lower = middle + 1;
        } else if (res == 0) {
	    *nump = p->subnum;
            return p->value;
        } else {
            upper = middle - 1;
        }
    }
    return ERRORTOKEN;
}



/*
 * action routines called by grammar
 */

int do_single_keyword (keyword)
    int keyword;
{
    switch (keyword) {
      case kw0_NoDefaults:
	Scr->NoDefaults = TRUE;
	return 1;

      case kw0_StickyAbove:
	Scr->StickyAbove = TRUE;
	return 1;

      case kw0_PannerOpaqueScroll:
	Scr->PannerOpaqueScroll = TRUE;
	return 1;

      case kw0_AutoRelativeResize:
	Scr->AutoRelativeResize = TRUE;
	return 1;

      case kw0_ForceIcons:
	if (Scr->FirstTime) Scr->ForceIcon = TRUE;
	return 1;

      case kw0_NoIconManagers:
	Scr->NoIconManagers = TRUE;
	return 1;

      case kw0_OpaqueMove:
	Scr->OpaqueMove = TRUE;
	return 1;

      case kw0_InterpolateMenuColors:
	if (Scr->FirstTime) Scr->InterpolateMenuColors = TRUE;
	return 1;

      case kw0_NoVersion:
	/* obsolete */
	return 1;

      case kw0_SortIconManager:
	if (Scr->FirstTime) Scr->SortIconMgr = TRUE;
	return 1;

      case kw0_NoGrabServer:
	Scr->NoGrabServer = TRUE;
	return 1;

      case kw0_NoMenuShadows:
	if (Scr->FirstTime) Scr->Shadow = FALSE;
	return 1;

      case kw0_NoRaiseOnMove:
	if (Scr->FirstTime) Scr->NoRaiseMove = TRUE;
	return 1;

      case kw0_NoRaiseOnResize:
	if (Scr->FirstTime) Scr->NoRaiseResize = TRUE;
	return 1;

      case kw0_NoRaiseOnDeiconify:
	if (Scr->FirstTime) Scr->NoRaiseDeicon = TRUE;
	return 1;

      case kw0_DontMoveOff:
	Scr->DontMoveOff = TRUE;
	return 1;

      case kw0_NoBackingStore:
	Scr->BackingStore = FALSE;
	return 1;

      case kw0_NoSaveUnders:
	Scr->SaveUnder = FALSE;
	return 1;

      case kw0_RestartPreviousState:
	RestartPreviousState = True;
	return 1;

      case kw0_ClientBorderWidth:
	if (Scr->FirstTime) Scr->ClientBorderWidth = TRUE;
	return 1;

      case kw0_NoTitleFocus:
	Scr->TitleFocus = FALSE;
	return 1;

      case kw0_RandomPlacement:
	Scr->RandomPlacement = TRUE;
	return 1;

      case kw0_DecorateTransients:
	Scr->DecorateTransients = TRUE;
	return 1;

      case kw0_ShowIconManager:
	Scr->ShowIconManager = TRUE;
	return 1;

      case kw0_ShowVirtualNames:
	Scr->ShowVirtualNames = TRUE;
	return 1;

      case kw0_StayUpMenus:
	if (Scr->FirstTime) Scr->StayUpMenus = TRUE;
	return 1;

      case kw0_NoCaseSensitive:
	Scr->CaseSensitive = FALSE;
	return 1;

      case kw0_NoRaiseOnWarp:
	Scr->NoRaiseWarp = TRUE;
	return 1;

      case kw0_WarpUnmapped:
	Scr->WarpUnmapped = TRUE;
	return 1;

      case kw0_ListRings:
	if (Scr->FirstTime) Scr->ListRings = TRUE;
	return 1;

      case kw0_DontInterpolateTitles:
	Scr->DontInterpolateTitles = TRUE;
	return 1;

      case kw0_WrapVirtual:
	Scr->WrapVirtual = TRUE;
	return 1;

      case kw0_RememberScreenPosition:
	Scr->RememberScreenPosition = TRUE;
	return 1;

      case kw0_NoOpaqueMoveSaveUnders:
	if (Scr->FirstTime) Scr->OpaqueMoveSaveUnders = FALSE;
	return 1;

#ifdef RJC
      case kw0_DoDefaultMenuAction:
	fprintf(stderr, "%s: DoDefaultMenuAction ignored\n", ProgramName);
	return 1;
#endif
    }

    return 0;
}


int do_string_keyword (keyword, s)
    int keyword;
    char *s;
{
    switch (keyword) {

      case kws_VirtualDesktop:
	{
	    int status, x, y;
	    unsigned int width, height;

	    status = XParseGeometry(s, &x, &y, &width, &height);
	    if ((status & (WidthValue & HeightValue)) != (WidthValue & HeightValue)) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid VirtualDesktop geometry \"%s\"\n", s);
	    } else {
	   	Scr->VirtualDesktop = True; 
		Scr->vdtWidth = width;
		Scr->vdtHeight = height;
	    }
	    return 1;
	}

      case kws_PannerState:
	{
	    int state = ParseState(s);
	    if (state < 0) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid PannerState argument \"%s\"\n", s);
	    } else {
		Scr->PannerState = state;
	    }
	    return 1;
	}

      case kws_PannerGeometry:
	{
	    int status, x, y;
	    unsigned int width, height;

	    status = XParseGeometry(s, &x, &y, &width, &height);
	    if ((status & (XValue | YValue)) != (XValue | YValue)) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid PannerGeometry \"%s\"\n", s);
	    } else {
		Scr->PannerGeometry = s;
	    }
	    return 1;
	}

      case kws_UsePPosition:
	{ 
	    int ppos = ParseUsePPosition (s);
	    if (ppos < 0) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid UsePPosition argument \"%s\"\n", s);
	    } else {
		Scr->UsePPosition = ppos;
	    }
	    return 1;
	}

      case kws_VirtualFont:
	if (!Scr->HaveFonts) Scr->VirtualFont.name = s;
	return 1;

      case kws_IconFont:
	if (!Scr->HaveFonts) Scr->IconFont.name = s;
	return 1;

      case kws_ResizeFont:
	if (!Scr->HaveFonts) Scr->SizeFont.name = s;
	return 1;

      case kws_MenuFont:
	if (!Scr->HaveFonts) Scr->MenuFont.name = s;
	return 1;

      case kws_MenuTitleFont:
	if (!Scr->HaveFonts) Scr->MenuTitleFont.name = s; 
	return 1;

      case kws_TitleFont:
	if (!Scr->HaveFonts) Scr->TitleBarFont.name = s;
	return 1;

      case kws_IconManagerFont:
	if (!Scr->HaveFonts) Scr->IconManagerFont.name = s;
	return 1;

      case kws_UnknownIcon:
	if (Scr->FirstTime) Scr->Unknown.name = s;
	return 1;

      case kws_IconDirectory:
	if (Scr->FirstTime) Scr->IconDirectory = ExpandFilename (s);
	return 1;

      case kws_Identification:
	if (Scr->FirstTime) Scr->Identification = s;
	return 1;

      case kws_AfterSetupRun:
	if (Scr->FirstTime) Scr->AfterSetupRun = s;
	return 1;

      case kws_MaxWindowSize:
	JunkMask = XParseGeometry (s, &JunkX, &JunkY, &JunkWidth, &JunkHeight);
	if ((JunkMask & (WidthValue | HeightValue)) != 
	    (WidthValue | HeightValue)) {
	    twmrc_error_prefix();
	    fprintf (stderr, "bad MaxWindowSize \"%s\"\n", s);
	    return 0;
	}
	if (JunkWidth <= 0 || JunkHeight <= 0) {
	    twmrc_error_prefix();
	    fprintf (stderr, "MaxWindowSize \"%s\" must be positive\n", s);
	    return 0;
	}
	Scr->MaxWindowWidth = JunkWidth;
	Scr->MaxWindowHeight = JunkHeight;
	return 1;
    }

    return 0;
}


int do_number_keyword (keyword, num)
    int keyword;
    int num;
{
    switch (keyword) {
      case kwn_PannerScale:
	if (num > 0)
	    Scr->PannerScale = num;
	return 1;

      case kwn_ScrollDistanceX:
	if (Scr->FirstTime)
		Scr->vdtScrollDistanceX = (num * Scr->MyDisplayWidth) / 100;
	return 1;

      case kwn_ScrollDistanceY:
	if (Scr->FirstTime)
		Scr->vdtScrollDistanceY = (num * Scr->MyDisplayHeight) / 100;
	return 1;

      case kwn_ConstrainedMoveTime:
	ConstrainedMoveTime = num;
	return 1;

      case kwn_MenuLineWidth:
	Scr->MenuLineWidth = num;
	return 1;

      case kwn_TitleFontPadding:
	Scr->TitleFontPadding = num;
	return 1;

      case kwn_MoveDelta:
	Scr->MoveDelta = num;
	return 1;

      case kwn_PieMenuWait:
	PieMenuWait = num;
	return 1;

      case kwn_XorValue:
	if (Scr->FirstTime) Scr->XORvalue = num;
	return 1;

      case kwn_FramePadding:
	if (Scr->FirstTime) Scr->FramePadding = num;
	return 1;

      case kwn_TitlePadding:
	if (Scr->FirstTime) Scr->TitlePadding = num;
	return 1;

      case kwn_ButtonIndent:
	if (Scr->FirstTime) Scr->ButtonIndent = num;
	return 1;

      case kwn_BorderWidth:
	if (Scr->FirstTime) Scr->BorderWidth = num;
	return 1;

      case kwn_IconBorderWidth:
	if (Scr->FirstTime) Scr->IconBorderWidth = num;
	return 1;

      case kwn_TitleButtonBorderWidth:
	if (Scr->FirstTime) Scr->TBInfo.border = num;
	return 1;

      case kwn_PopupSensitivity:
	if (num > 100 || num < 5) {
	    twmrc_error_prefix();
	    fprintf(stderr, "PopupSensitivity %d out of range, ignoring.\n",
			num);
	} else if (Scr->FirstTime) Scr->PopupSensitivity = num;
	return 1;

      case kwn_MenuShadowWidth:
	Scr->ShadowWidth = num;
	return 1;

    }

    return 0;
}

name_list **do_colorlist_keyword (keyword, colormode, s)
    int keyword;
    int colormode;
    char *s;
{
    switch (keyword) {
      case kwcl_BorderColor:
	GetColor (colormode, &Scr->BorderColor, s);
	return &Scr->BorderColorL;

      case kwcl_IconManagerHighlight:
	GetColor (colormode, &Scr->IconManagerHighlight, s);
	return &Scr->IconManagerHighlightL;

      case kwcl_BorderTileForeground:
	GetColor (colormode, &Scr->BorderTileC.fore, s);
	return &Scr->BorderTileForegroundL;

      case kwcl_BorderTileBackground:
	GetColor (colormode, &Scr->BorderTileC.back, s);
	return &Scr->BorderTileBackgroundL;

      case kwcl_TitleForeground:
	GetColor (colormode, &Scr->TitleC.fore, s);
	return &Scr->TitleForegroundL;

      case kwcl_TitleBackground:
	GetColor (colormode, &Scr->TitleC.back, s);
	return &Scr->TitleBackgroundL;

      case kwcl_VirtualForeground:
	GetColor (colormode, &Scr->VirtualC.fore, s);
	return &Scr->VirtualForegroundL;

      case kwcl_VirtualBackground:
	GetColor (colormode, &Scr->VirtualC.back, s);
	return &Scr->VirtualBackgroundL;

      case kwcl_IconForeground:
	GetColor (colormode, &Scr->IconC.fore, s);
	return &Scr->IconForegroundL;

      case kwcl_IconBackground:
	GetColor (colormode, &Scr->IconC.back, s);
	return &Scr->IconBackgroundL;

      case kwcl_IconBorderColor:
	GetColor (colormode, &Scr->IconBorderColor, s);
	return &Scr->IconBorderColorL;

      case kwcl_IconManagerForeground:
	GetColor (colormode, &Scr->IconManagerC.fore, s);
	return &Scr->IconManagerFL;

      case kwcl_IconManagerBackground:
	GetColor (colormode, &Scr->IconManagerC.back, s);
	return &Scr->IconManagerBL;
    }
    return NULL;
}

int do_color_keyword (keyword, colormode, s)
    int keyword;
    int colormode;
    char *s;
{
    switch (keyword) {
      case kwc_PannerBackground:
	GetColor (colormode, &Scr->PannerC.back, s);
	Scr->PannerBackgroundSet = True;
	return 1;

      case kwc_PannerForeground:
	GetColor (colormode, &Scr->PannerC.fore, s);
	return 1;

      case kwc_VirtualDesktopBackground:
	if (GetColor (colormode, &Scr->vdtC.back, s))
	    Scr->vdtBackgroundSet = True;
	return 1;

      case kwc_VirtualDesktopForeground:
	GetColor (colormode, &Scr->vdtC.fore, s);
	return 1;

      case kwc_DefaultForeground:
	GetColor (colormode, &Scr->DefaultC.fore, s);
	return 1;

      case kwc_DefaultBackground:
	GetColor (colormode, &Scr->DefaultC.back, s);
	return 1;

      case kwc_MenuForeground:
	GetColor (colormode, &Scr->MenuC.fore, s);
	return 1;

      case kwc_MenuBackground:
	GetColor (colormode, &Scr->MenuC.back, s);
	return 1;

      case kwc_MenuTitleForeground:
	GetColor (colormode, &Scr->MenuTitleC.fore, s);
	return 1;

      case kwc_MenuTitleBackground:
	GetColor (colormode, &Scr->MenuTitleC.back, s);
	return 1;

      case kwc_MenuShadowColor:
	GetColor (colormode, &Scr->MenuShadowColor, s);
	return 1;

    }

    return 0;
}

int do_pixmap_keyword(keyword, filename)
    int keyword;
    char *filename;
{

switch(keyword)
    {

 case kwp_TitleHighlight:
    Scr->hilite.name = filename;
    return 1;

 case kwp_TitleHighlightLeft:
    Scr->hiliteLeft.name = filename;
    return 1;

 case kwp_TitleHighlightRight:
    Scr->hiliteRight.name = filename;
    return 1;

 case kwp_IconifyPixmap:
    Scr->iconifyPm.name = filename;
    return 1;

 case kwp_PullRightPixmap:
    Scr->pullrightPm.name = filename;
    return 1;

 case kwp_PannerBackgroundPixmap:
    Scr->PannerPixmap = filename;
    return 1;

 case kwp_VirtualDesktopBackgroundPixmap:
    Scr->vdtPixmap = filename;
    return 1;

 case kwp_ShadowPixmap:
    Scr->shadowPm.name = filename;
    return 1;

    }
    return 0;
}

/*
 * put_pixel_on_root() Save a pixel value in twm root window color property.
 */
void
put_pixel_on_root(pixel)                                 
    Pixel pixel;                                         
{                                                        
  int           i, addPixel = 1;
  Atom          pixelAtom, retAtom;	                 
  int           retFormat;
  unsigned long nPixels, retAfter;                     
  Pixel        *retProp;
  pixelAtom = XInternAtom(dpy, "_MIT_PRIORITY_COLORS", True);        
  XGetWindowProperty(dpy, Scr->Root, pixelAtom, 0, 8192, 
		     False, XA_CARDINAL, &retAtom,       
		     &retFormat, &nPixels, &retAfter,    
		     (unsigned char **)&retProp);

  for (i=0; i< nPixels; i++)                             
      if (pixel == retProp[i]) addPixel = 0;             
                                                         
  if (addPixel)                                          
      XChangeProperty (dpy, Scr->Root, _XA_MIT_PRIORITY_COLORS,
		       XA_CARDINAL, 32, PropModeAppend,  
		       (unsigned char *)&pixel, 1);                       
}                                                        

/*
 * do_string_savecolor() save a color from a string in the twmrc file.
 */
void do_string_savecolor(colormode, s)
     int colormode;
     char *s;
{
  Pixel p;
  GetColor(colormode, &p, s);
  put_pixel_on_root(p);
}

/*
 * do_var_savecolor() save a color from a var in the twmrc file.
 */
typedef struct _cnode {int i; struct _cnode *next;} Cnode, *Cptr;
Cptr chead = NULL;

void
do_var_savecolor(key)
int key;
{
  Cptr cptrav, cpnew;
  if (!chead) {
    chead = (Cptr)malloc(sizeof(Cnode));
    chead->i = key; chead->next = NULL;
  }
  else {
    cptrav = chead;
    while (cptrav->next != NULL) { cptrav = cptrav->next; }
    cpnew = (Cptr)malloc(sizeof(Cnode));
    cpnew->i = key; cpnew->next = NULL; cptrav->next = cpnew;
  }
}

/*
 * assign_var_savecolor() traverse the var save color list placeing the pixels
 *                        in the root window property.
 */
void assign_var_savecolor()
{
  Cptr cp = chead;
  while (cp != NULL) {
    switch (cp->i) {
    case kwcl_BorderColor:
      put_pixel_on_root(Scr->BorderColor);
      break;
    case kwcl_IconManagerHighlight:
      put_pixel_on_root(Scr->IconManagerHighlight);
      break;
    case kwcl_BorderTileForeground:
      put_pixel_on_root(Scr->BorderTileC.fore);
      break;
    case kwcl_BorderTileBackground:
      put_pixel_on_root(Scr->BorderTileC.back);
      break;
    case kwcl_TitleForeground:
      put_pixel_on_root(Scr->TitleC.fore);
      break;
    case kwcl_TitleBackground:
      put_pixel_on_root(Scr->TitleC.back);
      break;
    case kwcl_IconForeground:
      put_pixel_on_root(Scr->IconC.fore);
      break;
    case kwcl_IconBackground:
      put_pixel_on_root(Scr->IconC.back);
      break;
    case kwcl_IconBorderColor:
      put_pixel_on_root(Scr->IconBorderColor);
      break;
    case kwcl_IconManagerForeground:
      put_pixel_on_root(Scr->IconManagerC.fore);
      break;
    case kwcl_IconManagerBackground:
      put_pixel_on_root(Scr->IconManagerC.back);
      break;
    }
    cp = cp->next;
  }
  if (chead) {
    free(chead);
    chead = NULL;
  }
}

static int ParseUsePPosition (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s, "off") == 0) {
	return PPOS_OFF;
    } else if (strcmp (s, "on") == 0) {
	return PPOS_ON;
    } else if (strcmp (s, "non-zero") == 0 ||
	       strcmp (s, "nonzero") == 0) {
	return PPOS_NON_ZERO;
    }

    return -1;
}

static int ParseState (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s, "withdrawn") == 0) {
	return WithdrawnState;
    } else if (strcmp (s, "normal") == 0) {
	return NormalState;
    } else if (strcmp (s, "iconic") == 0) {
	return IconicState;
    }

    return -1;
}

void
do_squeeze_entry (list, name, type, justify, num, denom)
    name_list **list;			/* squeeze or dont-squeeze list */
    char *name;				/* window name */
    short type;				/* match type */
    int justify;			/* left, center, right */
    int num;				/* signed num */
    int denom;				/* 0 or indicates fraction denom */
{
    int absnum = (num < 0 ? -num : num);

    if (denom < 0) {
	twmrc_error_prefix();
	fprintf (stderr, "negative SqueezeTitle denominator %d\n", denom);
	return;
    }
    if (absnum > denom && denom != 0) {
	twmrc_error_prefix();
	fprintf (stderr, "SqueezeTitle fraction %d/%d outside window\n",
		 num, denom);
	return;
    }
    if (denom == 1) {
	twmrc_error_prefix();
	fprintf (stderr, "useless SqueezeTitle faction %d/%d, assuming 0/0\n",
		 num, denom);
	num = 0;
	denom = 0;
    }

    if (HasShape) {
	SqueezeInfo *sinfo;
	sinfo = (SqueezeInfo *) malloc (sizeof(SqueezeInfo));

	if (!sinfo) {
	    twmrc_error_prefix();
	    fprintf (stderr, "unable to allocate %d bytes for squeeze info\n",
		     sizeof(SqueezeInfo));
	    return;
	}
	sinfo->justify = justify;
	sinfo->num = num;
	sinfo->denom = denom;
	AddToList (list, name, type, (char *)sinfo);
    }
}

static char *m4_defs();
static FILE *start_m4(fraw)
FILE *fraw;
{
	int fno;
	int fids[2];
	int fres;		/* Fork result */

	fno = fileno(fraw);
	/* if (-1 == fcntl(fno, F_SETFD, 0)) perror("fcntl()"); */
	pipe(fids);
	fres = fork();
	if (fres < 0) {
		perror("Fork for m4 failed");
		exit(23);
	}
	if (fres == 0) {
		extern Display *dpy;
		extern char *display_name;
		char *tmp_file;

		/* Child */
		close(0);			/* stdin */
		close(1);			/* stdout */
		dup2(fno, 0);		/* stdin = fraw */
		dup2(fids[1], 1);	/* stdout = pipe to parent */
		/* get_defs("m4", dpy, display_name) */
		tmp_file = m4_defs(dpy, display_name);
		execlp("m4", "m4", tmp_file, M4STDIN, NULL); 
		/* If we get here we are screwed... */
		perror("Can't execlp() m4");
		exit(124);
	}
	/* Parent */
	close(fids[1]);
	m4_pid = fres;
	return(fdopen(fids[0], "r"));
}

/* Code taken and munged from xrdb.c */
#define MAXHOSTNAME 255
#define Resolution(pixels, mm)	((((pixels) * 100000 / (mm)) + 50) / 100)
#define EXTRA	(15)

static char *MkDef(name, def)
char *name, *def;
{
	static char *cp = NULL;
	static int maxsize = 0;
	int n, nl;

	/* The char * storage only lasts for 1 call... */
	if ((n = EXTRA + ((nl = strlen(name)) +  strlen(def))) > maxsize) {
		if (cp) free(cp);
		cp = malloc(n);
		maxsize = n;
	}
	/* Otherwise cp is aready big 'nuf */
	if (cp == NULL) { 
		fprintf(stderr, "%s: Can't get %d bytes for arg parm\n",
			ProgramName, n);
		exit(33);
	}
	strcpy(cp, "define(");
	strcpy(cp+7, name);
	*(cp + nl + 7) = ',';
	*(cp + nl + 8) = ' ';
	strcpy(cp + nl + 9, def);
	strcat(cp + nl + 9, ")dnl\n");
	return(cp);
}

/* Make a definition, but quote the defined string... */

static char *MkQte(name, def)
char *name, *def;
{
	char *cp, *cp2;

	cp = malloc(3 + strlen(def));
	if (cp == NULL) {
		fprintf(stderr, "Can't get %d bytes for arg parm\n", 3 + strlen(def));
		exit(27);
	}

	/* Note:  We're assuming that ` and ' are still the m4 quote */
	/* characters.  Nothing should be ahead of this file in m4's */
	/* stdin, so it should be safe...                            */
	*cp = '`';
	strcpy(cp + 1, def);
	strcat(cp, "\'");
	cp2 = MkDef(name, cp);
	free(cp);		/* Not really needed, but good habits die hard... */
	return(cp2);
}

static char *MkNum(name, def)
char *name;
int def;
{
	char num[20];

	sprintf(num, "%d", def);
	return(MkDef(name, num));
}

#ifdef NOSTEMP
int mkstemp(str)
char *str;
{
	int fd;

	mktemp(str);
	fd = creat(str, 0744);
	if (fd == -1) perror("mkstemp's creat");
	return(fd);
}
#endif

static char *m4_defs(display, host)
Display *display;
char *host;
{
	extern int KeepTmpFile;
	Screen *screen;
	Visual *visual;
	char client[MAXHOSTNAME], server[MAXHOSTNAME], *colon;
	struct hostent *hostname;
	char *vc;		/* Visual Class */
	static char tmp_name[] = "/tmp/twmrcXXXXXX";
	int fd;
	FILE *tmpf;
	char *getenv_res;	/* Result of a getenv() call */

	fd = mkstemp(tmp_name);		/* I *hope* mkstemp exists, because */
					/* I tried to find the "portable" */
					/* mktmp... */
	if (fd < 0) {
		perror("mkstemp failed in m4_defs");
		exit(377);
	}
	tmpf = fdopen(fd, "w+");
	XmuGetHostname(client, MAXHOSTNAME);
	hostname = gethostbyname(client);
	strcpy(server, XDisplayName(host));
	colon = index(server, ':');
	if (colon != NULL) *colon = '\0';
	if ((server[0] == '\0') || (!strcmp(server, "unix")))
		strcpy(server, client); /* must be connected to :0 or unix:0 */
	/* The machine running the X server */
	fputs(MkDef("SERVERHOST", server), tmpf);
	/* The machine running the window manager process */
	fputs(MkDef("CLIENTHOST", client), tmpf);
	if (hostname)
		fputs(MkDef("HOSTNAME", hostname->h_name), tmpf);
	else
		fputs(MkDef("HOSTNAME", client), tmpf);
	if ((getenv_res = getenv("USER")) == NULL)
	    if ((getenv_res = getenv("LOGNAME")) == NULL)
		getenv_res = "nobody";
	fputs(MkDef("USER", getenv_res), tmpf);
	if ((getenv_res = getenv("HOME")) == NULL) 
	    getenv_res = ".";
	fputs(MkQte("HOME", getenv_res), tmpf);
	fputs(MkNum("VERSION", ProtocolVersion(display)), tmpf);
	fputs(MkNum("REVISION", ProtocolRevision(display)), tmpf);
	fputs(MkQte("VENDOR", ServerVendor(display)), tmpf);
	fputs(MkNum("RELEASE", VendorRelease(display)), tmpf);
	screen = ScreenOfDisplay(display, Scr->screen);
	visual = DefaultVisualOfScreen(screen);
	fputs(MkNum("WIDTH", screen->width), tmpf);
	fputs(MkNum("HEIGHT", screen->height), tmpf);
	fputs(MkNum("X_RESOLUTION",Resolution(screen->width,screen->mwidth)), tmpf);
	fputs(MkNum("Y_RESOLUTION",Resolution(screen->height,screen->mheight)),tmpf);
	fputs(MkNum("PLANES",DisplayPlanes(display, DefaultScreen(display))), tmpf);
	fputs(MkNum("BITS_PER_RGB", visual->bits_per_rgb), tmpf);
	fputs(MkDef("TWM_TYPE", PtvtwmName), tmpf);
	switch(visual->class) {
		case(StaticGray):	vc = "StaticGray";	break;
		case(GrayScale):	vc = "GrayScale";	break;
		case(StaticColor):	vc = "StaticColor";	break;
		case(PseudoColor):	vc = "PseudoColor";	break;
		case(TrueColor):	vc = "TrueColor";	break;
		case(DirectColor):	vc = "DirectColor";	break;
		default:			vc = "NonStandard";	break;
	}
	fputs(MkDef("CLASS", vc), tmpf);
	if (visual->class != StaticGray && visual->class != GrayScale) {
		fputs(MkDef("COLOR", "Yes"), tmpf);
	} else {
		fputs(MkDef("COLOR", "No"), tmpf);
	}
	fputs(MkQte("INITFILE", InitFile), tmpf);
#ifdef RJC
	fputs(MkDef("RJC", "Yes"), tmpf);
#else
	fputs(MkDef("RJC", "No"), tmpf);
#endif
#ifdef XPM
	fputs(MkDef("XPM", "Yes"), tmpf);
#else
	fputs(MkDef("XPM", "No"), tmpf);
#endif
	if (KeepTmpFile) {
		fprintf(stderr, "Left file: %s\n", tmp_name);
	} else {
		fprintf(tmpf, "syscmd(/bin/rm %s)\n", tmp_name);
	}
	fclose(tmpf);
	return(tmp_name);
}
