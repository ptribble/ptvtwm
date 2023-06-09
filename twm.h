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
 * $XConsortium: twm.h,v 1.74 91/05/31 17:38:30 dave Exp $
 *
 * twm include file
 *
 * 28-Oct-87 Thomas E. LaStrange	File created
 * 10-Oct-90 David M. Sternlicht        Storeing saved colors on root
 ***********************************************************************/

#ifndef _TWM_
#define _TWM_

#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>
#ifndef PRE_R5_ENV
# include <X11/Xfuncs.h>
#endif
#ifdef XPM
# include <X11/xpm.h>
#endif

#include "icons.h"

#ifndef WithdrawnState
#define WithdrawnState 0
#endif

#ifdef SIGNALRETURNSINT
# define SIGNAL_T int
# define SIGNAL_RETURN return 0
#else
# define SIGNAL_T void
# define SIGNAL_RETURN return
#endif

typedef SIGNAL_T (*SigProc)();	/* type of function returned by signal() */

#define BW 2			/* border width */
#define BW2 4			/* border width  * 2 */

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define NULLSTR ((char *) NULL)

#define MAX_BUTTONS	5	/* max mouse buttons supported */

/* info stings defines */
#define INFO_LINES 30
#define INFO_SIZE 200

/* contexts for button presses */
#define C_NO_CONTEXT	-1
#define C_WINDOW	0
#define C_TITLE		1
#define C_ICON		2
#define C_ROOT		3
#define C_FRAME		4
#define C_ICONMGR	5
#define C_NAME		6
#define C_IDENTIFY      7
#define NUM_CONTEXTS	8

#define C_WINDOW_BIT	(1 << C_WINDOW)
#define C_TITLE_BIT	(1 << C_TITLE)
#define C_ICON_BIT	(1 << C_ICON)
#define C_ROOT_BIT	(1 << C_ROOT)
#define C_FRAME_BIT	(1 << C_FRAME)
#define C_ICONMGR_BIT	(1 << C_ICONMGR)
#define C_NAME_BIT	(1 << C_NAME)

#define C_ALL_BITS	(C_WINDOW_BIT | C_TITLE_BIT | C_ICON_BIT |\
			 C_ROOT_BIT | C_FRAME_BIT | C_ICONMGR_BIT)

/* modifiers for button presses */
#define MOD_SIZE	((ShiftMask | ControlMask | Mod1Mask \
			  | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask) + 1)

#define TITLE_BAR_SPACE         1	/* 2 pixel space bordering chars */
#define TITLE_BAR_FONT_HEIGHT   15	/* max of 15 pixel high chars */
#define TITLE_BAR_HEIGHT        (TITLE_BAR_FONT_HEIGHT+(2*TITLE_BAR_SPACE))

/* defines for zooming/unzooming */
#define ZOOM_NONE 0

#define UNKNOWN_PIXEL ((Pixel)-1)

#define FBF(fix_fore, fix_back, fix_font)\
    Gcv.foreground = fix_fore;\
    Gcv.background = fix_back;\
    Gcv.font = fix_font;\
    XChangeGC(dpy, Scr->NormalGC, GCFont|GCForeground|GCBackground,&Gcv)

#define FB(fix_fore, fix_back)\
    Gcv.foreground = fix_fore;\
    Gcv.background = fix_back;\
    XChangeGC(dpy, Scr->NormalGC, GCForeground|GCBackground,&Gcv)

typedef struct MyFont
{
    char *name;			/* name of the font */
    XFontStruct *font;		/* font structure */
    int height;			/* height of the font */
    int y;			/* Y coordinate to draw characters */
} MyFont;

typedef struct ColorPair
{
    Pixel fore, back;
} ColorPair;

typedef struct _TitleButton {
    struct _TitleButton *next;		/* next link in chain */
    char *name;				/* bitmap name in case of deferal */
    Pixmap bitmap;			/* image to display in button */
    Bool isXpm;				/* is it a Xpm pixmap or a bitmap? */
    int srcx, srcy;			/* from where to start copying */
    unsigned int width, height;		/* size of pixmap */
    int dstx, dsty;			/* to where to start copying */
    int func;				/* function to execute */
    char *action;			/* optional action arg */
    struct MenuRoot *menuroot;		/* menu to pop on F_MENU */
    Bool rightside;			/* t: on right, f: on left */
} TitleButton;

typedef struct _TBWindow {
    Window window;			/* which window in this frame */
    TitleButton *info;			/* description of this window */
} TBWindow;

typedef struct _SqueezeInfo {
    int justify;			/* left, center, right */
    int num;				/* signed pixel count or numerator */
    int denom;				/* 0 for pix count or denominator */
} SqueezeInfo;

#define J_LEFT			1
#define J_CENTER		2
#define J_RIGHT			3

/* Colormap window entry for each window in WM_COLORMAP_WINDOWS
 * ICCCM property.
 */
typedef struct TwmColormap
{	
    Colormap c;			/* Colormap id */
    int state;			/* install(ability) state */
    unsigned long install_req;	/* request number which installed it */
    Window w;			/* window causing load of color table */
    int refcnt;
} TwmColormap;

#define CM_INSTALLABLE		1
#define CM_INSTALLED		2
#define CM_INSTALL		4

typedef struct ColormapWindow
{
    Window w;			/* Window id */
    TwmColormap *colormap;	/* Colormap for this window */
    int visibility;		/* Visibility of this window */
    int refcnt;
} ColormapWindow;

typedef struct Colormaps
{
    ColormapWindow **cwins;	/* current list of colormap windows */
    int number_cwins;		/* number of elements in current list */
    char *scoreboard;		/* conflicts between installable colortables */
} Colormaps;

#define ColormapsScoreboardLength(cm) ((cm)->number_cwins * \
				       ((cm)->number_cwins - 1) / 2)

/* Bit definitions for the _TWM_FLAGS property.  This property
 * will be used to store state information to be used if 
 * RestartPreviousState is set
 */
#define TWM_FLAGS_VALID		(1 << 0)	/* the flags property was present */
#define TWM_FLAGS_STICKY	(1 << 1)	/* the sticky state */

/* for each window that is on the display, one of these structures
 * is allocated and linked into a list 
 */
typedef struct TwmWindow
{
    struct TwmWindow *next;	/* next twm window */
    struct TwmWindow *prev;	/* previous twm window */
    Window w;			/* the child window */
    Window root;		/* its idea of the root window */
    Window virtualWindow;	/* small panner prepresentation */
    Window virtualIcon;		/* small panner prepresentation */
    unsigned long flags;	/* _TWM_FLAGS property value */
    int old_bw;			/* border width before reparenting */
    Window frame;		/* the frame window */
    Window title_w;		/* the title bar window */
    Window hilite_w;		/* the hilite window */
    Pixmap gray;
    int frame_x;		/* x position of frame */
    int frame_y;		/* y position of frame */
    int screen_x;		/* x position relative to screen */
    int screen_y;		/* y position relative to screen */
    int frame_width;		/* width of frame */
    int frame_height;		/* height of frame */
    int frame_bw;		/* borderwidth of frame */
    int title_x;
    int title_y;
    TwmIcon icon;		/* the icon for this window */
    int icon_loc_x;		/* icon x coordinate */
    int icon_loc_y;		/* icon y coordiante */
    int title_height;		/* height of the title bar */
    int title_width;		/* width of the title bar */
    char *full_name;		/* full name of the window */
    char *name;			/* name of the window */
    char *icon_name;		/* name of the icon */
    int name_width;		/* width of name text */
    int highlightx;		/* start of highlight window */
    int rightx;			/* start of right buttons */
    XWindowAttributes attr;	/* the child window attributes */
    XSizeHints hints;		/* normal hints */
    XWMHints *wmhints;		/* WM hints */
    Window group;		/* group ID */
    XClassHint class;
    struct WList *list;
    /***********************************************************************
     * color definitions per window
     **********************************************************************/
    Pixel border;		/* border color */
    Pixel icon_border;		/* border color */
    ColorPair border_tile;
    ColorPair title;
    ColorPair iconc;
    ColorPair virtual;
    short iconified;		/* has the window ever been iconified? */
    short isicon;		/* is the window an icon now ? */
    short icon_on;		/* is the icon visible */
    short mapped;		/* is the window mapped ? */
    short auto_raise;		/* should we auto-raise this window ? */
    short forced;		/* has had an icon forced upon it */
    short icon_moved;		/* user explicitly moved the icon */
    short highlight;		/* should highlight this window */
    short stackmode;		/* honor stackmode requests */
    short iconify_by_unmapping;	/* unmap window to iconify it */
    short iconmgr;		/* this is an icon manager window */
    short transient;		/* this is a transient window */
    short icon_title;		/* should we give the icon a title? */
    Window transientfor;	/* window contained in XA_XM_TRANSIENT_FOR */
    short titlehighlight;	/* should I highlight the title bar */
    short sticky;		/* is the window sticky */
    struct IconMgr *iconmgrp;	/* pointer to it if this is an icon manager */
    int save_frame_x;		/* x position of frame */
    int save_frame_y;		/* y position of frame */
    int save_frame_width;	/* width of frame */
    int save_frame_height;	/* height of frame */
    short zoomed;		/* is the window zoomed? */
    short wShaped;		/* this window has a bounding shape */
    short squeeze_icon;		/* shape the icon with its label? */
    unsigned long protocols;	/* which protocols this window handles */
    Colormaps cmaps;		/* colormaps for this application */
    TBWindow *titlebuttons;
    SqueezeInfo *squeeze_info;	/* should the title be squeezed? */
    struct {
	struct TwmWindow *next, *prev;
	Bool cursor_valid;
	int curs_x, curs_y;
    } ring;
} TwmWindow;

#define DoesWmTakeFocus		(1L << 0)
#define DoesWmSaveYourself	(1L << 1)
#define DoesWmDeleteWindow	(1L << 2)

#define TBPM_DOT ":dot"		/* name of titlebar pixmap for dot */
#define TBPM_ICONIFY ":iconify"	/* same image as dot */
#define TBPM_RESIZE ":resize"	/* name of titlebar pixmap for resize button */
#define TBPM_XLOGO ":xlogo"	/* name of titlebar pixmap for xlogo */
#define TBPM_DELETE ":delete"	/* same image as xlogo */
#define TBPM_MENU ":menu"	/* name of titlebar pixmap for menus */
#define TBPM_QUESTION ":question"	/* name of unknown titlebar pixmap */

#ifndef PRE_R5_ENV
# include <X11/Xosdefs.h>
#endif
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern char *malloc(), *calloc(), *realloc(), *getenv();
extern int free();
#endif
extern void Reborder();
extern SIGNAL_T Done();
void ComputeCommonTitleOffsets();
void ComputeWindowTitleOffsets(), ComputeTitleLocation();
extern char *ProgramName;
extern char *PtvtwmName;
extern Display *dpy;
extern Window ResizeWindow;	/* the window we are resizing */
extern int HasShape;		/* this server supports Shape extension */

extern int PreviousScreen;

extern Cursor UpperLeftCursor;
extern Cursor RightButt;
extern Cursor MiddleButt;
extern Cursor LeftButt;

extern XClassHint NoClass;

extern XContext VirtualContext;
extern XContext TwmContext;
extern XContext MenuContext;
extern XContext IconManagerContext;
extern XContext ScreenContext;
extern XContext ColormapContext;

extern char *Home;
extern int HomeLen;
extern int ParseError;

extern int HandlingEvents;

extern Window JunkRoot;
extern Window JunkChild;
extern int JunkX;
extern int JunkY;
extern unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth, JunkMask;
extern Bool JunkIsXpm[1];
extern XGCValues Gcv;
extern int InfoLines;
extern char Info[][INFO_SIZE];
extern int Argc;
extern char **Argv;
extern char **Environ;
extern void NewFontCursor();
extern Pixmap CreateMenuIcon();

#ifdef XLOADIMAGE
extern int xloadim_pid;
#endif

extern int m4_pid;

extern Bool ErrorOccurred;
extern XErrorEvent LastErrorEvent;

#define ResetError() (ErrorOccurred = False)

extern Bool RestartPreviousState;
extern Bool GetWMState();

extern Atom _XA_MIT_PRIORITY_COLORS;
extern Atom _XA_WM_CHANGE_STATE;
extern Atom _XA_WM_STATE;
extern Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_WM_PROTOCOLS;
extern Atom _XA_WM_TAKE_FOCUS;
extern Atom _XA_WM_SAVE_YOURSELF;
extern Atom _XA_WM_DELETE_WINDOW;
extern Atom _XA_TWM_FLAGS;
extern Atom _XA_TWM_RESTART;
extern Atom _XA_TWM_PROCESS;
extern Atom _XA_TWM_MACHINE;

extern void CreateFonts();
extern void FetchWmProtocols();
extern void FetchWmColormapWindows();

#endif /* _TWM_ */
