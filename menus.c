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
 * Pie menu tracking (optimized for speed) and layout (optimized for
 * screen space) designed and implemented by Don Hopkins.
 * These algorithms are not patented or restricted.
 * You may use them as you like and improve upon them.
 * Take a look and feel free!
 *
 * Copyright (C) 1989 by Don Hopkins. All rights reserved.
 * This program is provided for unrestricted use, provided that this 
 * copyright message is preserved. There is no warranty, and no author 
 * or distributer accepts responsibility for any damage caused by this 
 * program. 
 *
 ***********************************************************************/

/***********************************************************************
 *
 * Merge of piewm 1.04 with tvtwm-pl11
 * Portions Copyright 2010 Peter C Tribble <peter.tribble@gmail.com>
 *
 ***********************************************************************/

/***********************************************************************
 *
 * $XConsortium: menus.c,v 1.186 91/07/17 13:58:00 dave Exp $
 *
 * twm menu code
 *
 * $Log: menus.c,v $
 * Revision 1.25  1995/02/09  20:08:59  cross
 * Document a lack of a feature...
 *
 * Revision 1.24  1995/02/03  00:13:17  cross
 * Use XmuCompareISOLatin1(), not FIXED_XmuCompareISOLatin1(), since it
 * seems to be free of the original bug in R5 and greater (and,
 * FIXED_XmuCompareISOLatin1() had a bug in it that XmuCompareISOLatin1()
 * doesn't...)
 *
 * Revision 1.23  1994/10/17  20:06:20  cross
 * Fixed relative resize buglet, and changed around to the X11R6 code
 * for warping...
 *
 * Revision 1.22  1994/03/14  17:33:08  cross
 * Doesn't SEGV (doesn't do anything at all, now) if you call relativemove
 * when there's not a window for it to act upon.
 *
 * Revision 1.21  1993/12/01  22:58:42  cross
 * Added f.relativemove...
 *
 * Revision 1.20  1993/05/20  15:40:06  cross
 * pl9b5.  Mostly double-headed machine stuff
 * XPM fixes to such.
 *
 * Revision 1.18  1993/01/20  23:14:34  cross
 * RJC changes
 *
 * Revision 1.17  1992/11/04  01:17:22  cross
 * Added code from rjc, with filtering.
 * See text in CHANGES for details.
 *
 * Revision 1.16  1992/08/17  15:04:24  cross
 * Added a needed include of sys/types.h
 *
 * Revision 1.15  1992/05/09  02:48:56  cross
 * Minor modification that should be more correct, but doesn't seem
 * to change much...
 *
 * Revision 1.14  1992/05/02  19:46:35  cross
 * Added changes by RJC.  A few bug fixes, reorganization of lots of
 * code, and many new features
 *
 * Revision 1.13  1992/02/09  21:50:03  cross
 * First attempt to make StayUpMenus work again...  There should be
 * more work done on this...
 *
 * Revision 1.12  1992/01/20  20:27:39  cross
 * Fixed a bug that caused incorect colors on titlebar pixmaps
 *
 * Revision 1.11  1992/01/08  18:55:49  cross
 * rewrote all the color/pixmap code
 * fixes from Bob Mende (mende@piecomputer.rutgers.edu)
 *   - f.menufunc works
 *   - SIGUSR1 implementation
 *   - ListRings works
 * don't use m4 by default; added -m/-M flag
 * DontInterpolateTitles
 * icon stuff.  IconWindow doesn't get a bad mask.
 * colors passed to xloadimage
 *
 * Revision 1.10  1991/10/29  22:28:11  cross
 * Updated to be a strict superset of X11R5's twm, not X11R4's.
 *
 * Revision 1.9  1991/10/05  08:39:16  cross
 * Changed to the new version of Xpm (v3.0, released a whole day before
 * the contrib deadline.  Ugh.)
 *
 * Revision 1.8  1991/10/04  22:44:03  cross
 * Changed the pixmap definition for the pullright pixmap.
 * Now R5'ized.
 *
 * Revision 1.7  1991/10/03  00:09:23  cross
 * Fixed some bogus uses of the NULL symbol.  Also, had to cast menu12_data
 * to type char*.  It's unsigned char * in the R5 dist, but XCrPFBData()
 * still wants a char*.  gcc caught it.
 *
 * Revision 1.6  1991/10/02  23:20:05  cross
 * Spacing changes
 *
 * Revision 1.5  1991/10/02  22:36:23  cross
 * Made some minor adjustements to text in the info window
 *
 * Revision 1.4  1991/10/01  20:12:10  cross
 * Fixed a typo.  Had a "<" where I meant ">="
 *
 * Revision 1.3  1991/09/26  00:41:59  cross
 * Changed old-style call to XCreatePixmapFromData to new-style (Xpm
 * v3.0 +)
 * Also, included X11/xpm.h
 *
 * Revision 1.2  1991/09/26  00:19:00  cross
 * Changed all calls to FindBitmap() to FindPixmap().
 * also, changed ispixmap stuff to isxpm
 *
 * Revision 1.1  1991/09/26  00:14:58  cross
 * Initial revision
 *
 * Revision 10.0  91/06/12  09:05:38  toml
 * Revision bump
 * 
 * Revision 9.1  91/05/14  11:27:44  toml
 * Fixed crash problem and removed #include "malloc.h"
 * 
 * Revision 9.0  91/04/23  07:40:40  toml
 * Revision bump
 * 
 * Revision 8.3  91/04/15  09:13:24  toml
 * Remove version comment
 * 
 * Revision 8.2  90/12/29  16:39:35  toml
 * RJC patches
 * 
 * Revision 8.1  90/12/29  10:13:12  toml
 * StayUpMenus
 * 
 * Revision 8.0  90/11/15  20:02:43  toml
 * Revision bump
 * 
 * Revision 7.4  90/11/15  19:59:42  toml
 * removed a printf from a patch
 * 
 * Revision 7.3  90/11/12  22:05:00  toml
 * Applied opaque move patches
 * 
 * Revision 7.2  90/11/12  21:34:54  toml
 * Implemented Scr->StickyAbove
 * 
 * Revision 7.1  90/11/12  19:57:25  toml
 * Patches to allow sticky windows to lower
 * 
 * Revision 1.2  90/11/04  18:38:22  brossard
 * Sticky windows are now child of the virtual root.
 * This has the advantage that they can now be raised and lowered like
 * any other window.  They no longuer are above everything else.
 * It has the disadvantage that when you move the desktop, the
 * sticky windows have to be moved back after scrolling the desktop.
 * 
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include "twm.h"
#include "gc.h"
#include "menus.h"
#include "resize.h"
#include "events.h"
#include "util.h"
#include "parse.h"
#include "gram.h"
#include "screen.h"
#include <X11/Xmu/CharSet.h>		/* for XmuCompareISOLatin1() */
#include <X11/Xmu/Drawing.h>
#include "version.h"
#include "vdt.h"
#include "add_window.h"
#include "patchlevel.h"
#include <math.h>

#define TWO_PI 6.2831853071795865
#define DEG_TO_RAD(d) (((d) * TWO_PI) / 360.0)
#define PIE_BORDER 4
#define PIE_LABEL_RADIUS_MIN 12
#define PIE_LABEL_RADIUS_STEP 1
#define PIE_LABEL_RADIUS_EXTRA 6
#define PIE_INACTIVE_RADIUS 6

extern XEvent Event;

int RootFunction = 0;
char *RootAction = NULL;

MenuRoot *ActiveMenu = NULL;		/* the active menu */
MenuItem *ActiveItem = NULL;		/* the active menu item */
int MoveFunction;			/* either F_MOVE or F_FORCEMOVE */
int WindowMoved = FALSE;
int menuFromFrameOrWindowOrTitlebar = FALSE;

int ConstMove = FALSE;		/* constrained move variables */
int ConstMoveDir;
int ConstMoveX;
int ConstMoveY;
int ConstMoveXL;
int ConstMoveXR;
int ConstMoveYT;
int ConstMoveYB;

/* Globals used to keep track of whether the mouse has moved during
   a resize function. */
int ResizeOrigX;
int ResizeOrigY;

int MenuDepth = 0;		/* number of menus up */
static struct {
    int x;
    int y;
} MenuOrigins[MAXMENUDEPTH];
static Cursor LastCursor;

void WarpAlongRing(), WarpToWindow(), WarpToScreen();
void BumpWindowColormap();
static void LayoutPieMenu(MenuRoot *menu);
static MenuItem *CalcPieMenuItem(MenuRoot *menu, int x, int y);
static void PaintPieMenu(MenuRoot *mr);
static void PaintPieMenuEntry(MenuRoot *mr, MenuItem *mi, int exposure);
static void ShapePieMenu(MenuRoot *mr);

extern char *Action;
extern int Context;
extern TwmWindow *ButtonWindow, *Tmp_win;
extern XEvent Event, ButtonEvent;
extern char *InitFile;
static void Identify();

int PieMenuWait = 100;

#define MAX(x,y) ((x)>(y)?(x):(y))
#define ABS(x) ((x)<0?-(x):(x))
#define SHADOWWIDTH 5			/* in pixels */



/***********************************************************************
 *
 *  Procedure:
 *	InitMenus - initialize menu roots
 *
 ***********************************************************************
 */

void
InitMenus()
{
    int i, j, k;
    FuncKey *key, *tmp;

    for (i = 0; i < MAX_BUTTONS+1; i++)
	for (j = 0; j < NUM_CONTEXTS; j++)
	    for (k = 0; k < MOD_SIZE; k++)
	    {
		Scr->Mouse[i][j][k].func = 0;
		Scr->Mouse[i][j][k].item = NULL;
	    }

    Scr->DefaultFunction.func = 0;
    Scr->WindowFunction.func = 0;

    if (FirstScreen)
    {
	for (key = Scr->FuncKeyRoot.next; key != NULL;)
	{
	    free(key->name);
	    tmp = key;
	    key = key->next;
	    free((char *) tmp);
	}
	Scr->FuncKeyRoot.next = NULL;
    }

}



/***********************************************************************
 *
 *  Procedure:
 *	AddFuncKey - add a function key to the list
 *
 *  Inputs:
 *	name	- the name of the key
 *	cont	- the context to look for the key press in
 *	mods	- modifier keys that need to be pressed
 *	func	- the function to perform
 *	win_name- the window name (if any)
 *	action	- the action string associated with the function (if any)
 *
 ***********************************************************************
 */

Bool AddFuncKey (name, cont, mods, func, win_name, action)
    char *name;
    int cont, mods, func;
    char *win_name;
    char *action;
{
    FuncKey *tmp;
    KeySym keysym;
    KeyCode keycode;

    /*
     * Don't let a 0 keycode go through, since that means AnyKey to the
     * XGrabKey call in GrabKeys().
     */
    if ((keysym = XStringToKeysym(name)) == NoSymbol ||
	(keycode = XKeysymToKeycode(dpy, keysym)) == 0)
    {
	return False;
    }

    /* see if there already is a key defined for this context */
    for (tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->keysym == keysym &&
	    tmp->cont == cont &&
	    tmp->mods == mods)
	    break;
    }

    if (tmp == NULL)
    {
	tmp = (FuncKey *) malloc(sizeof(FuncKey));
	tmp->next = Scr->FuncKeyRoot.next;
	Scr->FuncKeyRoot.next = tmp;
    }

    tmp->name = name;
    tmp->keysym = keysym;
    tmp->keycode = keycode;
    tmp->cont = cont;
    tmp->mods = mods;
    tmp->func = func;
    tmp->win_name = win_name;
    tmp->action = action;

    return True;
}



int CreateTitleButton (name, func, action, menuroot, rightside, append)
    char *name;
    int func;
    char *action;
    MenuRoot *menuroot;
    Bool rightside;
    Bool append;
{
    TitleButton *tb = (TitleButton *) malloc (sizeof(TitleButton));

    if (!tb) {
	fprintf (stderr,
		 "%s:  unable to allocate %d bytes for title button\n",
		 ProgramName, sizeof(TitleButton));
	return 0;
    }


    tb->next = NULL;
    tb->name = name;			/* note that we are not copying */
    tb->bitmap = None;			/* WARNING, values not set yet */
#ifdef XPM
    tb->isXpm = False;		 /* assume a bitmap */
#endif  /* XPM */
    tb->width = 0;			/* see InitTitlebarButtons */
    tb->height = 0;			/* ditto */
    tb->func = func;
    tb->action = action;
    tb->menuroot = menuroot;
    tb->rightside = rightside;
    if (rightside) {
	Scr->TBInfo.nright++;
    } else {
	Scr->TBInfo.nleft++;
    }

    /*
     * Cases for list:
     * 
     *     1.  empty list, prepend left       put at head of list
     *     2.  append left, prepend right     put in between left and right
     *     3.  append right                   put at tail of list
     *
     * Do not refer to widths and heights yet since buttons not created
     * (since fonts not loaded and heights not known).
     */
    if ((!Scr->TBInfo.head) || ((!append) && (!rightside))) {	/* 1 */
	tb->next = Scr->TBInfo.head;
	Scr->TBInfo.head = tb;
    } else if (append && rightside) {	/* 3 */
	register TitleButton *t;
	for /* SUPPRESS 530 */
	  (t = Scr->TBInfo.head; t->next; t = t->next) ;
	t->next = tb;
	tb->next = NULL;
    } else {				/* 2 */
	register TitleButton *t, *prev = NULL;
	for (t = Scr->TBInfo.head; t && !t->rightside; t = t->next) {
	    prev = t;
	}
	if (prev) {
	    tb->next = prev->next;
	    prev->next = tb;
	} else {
	    tb->next = Scr->TBInfo.head;
	    Scr->TBInfo.head = tb;
	}
    }

    return 1;
}



/*
 * InitTitlebarButtons - Do all the necessary stuff to load in a titlebar
 * button.  If we can't find the button, then put in a question; if we can't
 * find the question mark, something is wrong and we are probably going to be
 * in trouble later on.
 */
void InitTitlebarButtons ()
{
    TitleButton *tb;
    int h;
    Pixel bg_color;

    /*
     * initialize dimensions
     */
    Scr->TBInfo.width = (Scr->TitleHeight -
			 2 * (Scr->FramePadding + Scr->ButtonIndent));
    Scr->TBInfo.pad = ((Scr->TitlePadding > 1)
		       ? ((Scr->TitlePadding + 1) / 2) : 1);
    h = Scr->TBInfo.width - 2 * Scr->TBInfo.border;

    /*
     * add in some useful buttons and bindings so that novices can still
     * use the system.
     */
    if (!Scr->NoDefaults) {
	/* insert extra buttons */
	if (!CreateTitleButton (TBPM_ICONIFY, F_ICONIFY, "", (MenuRoot *) NULL,
				False, False)) {
	    fprintf (stderr, "%s:  unable to add iconify button\n",
		     ProgramName);
	}
	if (!CreateTitleButton (TBPM_RESIZE, F_RESIZE, "", (MenuRoot *) NULL,
				True, True)) {
	    fprintf (stderr, "%s:  unable to add resize button\n",
		     ProgramName);
	}
	AddDefaultBindings ();
    }
    ComputeCommonTitleOffsets ();

    /*
     * load in images and do appropriate centering
     */

    for (tb = Scr->TBInfo.head; tb; tb = tb->next) {
	tb->bitmap = FindPixmap (tb->name, &tb->width, &tb->height,
		&tb->isXpm, &bg_color, NULL, 0, NULL);
	if (!tb->bitmap) {
	    tb->bitmap = FindPixmap (TBPM_QUESTION, &tb->width, &tb->height,
					&tb->isXpm, &bg_color, NULL, 0, NULL);
	    if (!tb->bitmap) {		/* cannot happen (see util.c) */
		fprintf (stderr,
			 "%s:  unable to add titlebar button \"%s\"\n",
			 ProgramName, tb->name);
	    }
	} else if ((tb->name[0] != ':') && (tb->name[0] != '#') &&
			(tb->isXpm == False))
	    tb->bitmap = XmuLocateBitmapFile(ScreenOfDisplay(dpy, Scr->screen),
				tb->name, NULL, 0, &tb->width, &tb->height,
				&JunkX, &JunkY);

	tb->dstx = (h - tb->width + 1) / 2;
	if (tb->dstx < 0) {		/* clip to minimize copying */
	    tb->srcx = -(tb->dstx);
	    tb->width = h;
	    tb->dstx = 0;
	} else {
	    tb->srcx = 0;
	}
	tb->dsty = (h - tb->height + 1) / 2;
	if (tb->dsty < 0) {
	    tb->srcy = -(tb->dsty);
	    tb->height = h;
	    tb->dsty = 0;
	} else {
	    tb->srcy = 0;
	}

	if (tb->isXpm == True) {
	    Pixmap new_btn;
	    GC gc;
	    XGCValues vals;

	    new_btn = XCreatePixmap(dpy, Scr->Root, h, h,
			DefaultDepth(dpy, Scr->screen));
	    vals.foreground = bg_color;
	    gc = XCreateGC(dpy, new_btn, GCForeground, &vals);
	    XFillRectangle(dpy, new_btn, gc, 0, 0, h, h);
	    XCopyArea(dpy, tb->bitmap, new_btn, gc, tb->srcx, tb->srcy,
			tb->width, tb->height, tb->dstx, tb->dsty);
	    XFreeGC(dpy, gc);
	    XFreePixmap(dpy, tb->bitmap);
	    tb->bitmap = new_btn;
/* A completely new pixmap has been generated, so tvtwm must be
 * told that it is the same size as the window, and therefore the
 * coords for src and dst should all be 0. 				*/
	    tb->width = tb->height = h;
	    tb->dstx = tb->dsty = tb->srcx = tb->srcy = 0;
	}
    }
}



PaintEntry(mr, mi, exposure)
MenuRoot *mr;
MenuItem *mi;
int exposure;
{
    int y_offset;
    int text_y;
    GC gc;
    MyFont *font;

    XSynchronize(dpy, True);

#ifdef DEBUG_MENUS
    fprintf(stderr, "Paint entry\n");
#endif

    if (mr->pie_menu) {
        PaintPieMenuEntry(mr, mi, exposure);
	return;
    }

    y_offset = mi->item_num * Scr->EntryHeight;

    if (mi->func != F_TITLE)
    {
	int x, y;

	font = &(Scr->MenuFont);

	text_y = y_offset + font->y;

	if (mi->state)
	{
	    XSetForeground(dpy, Scr->NormalGC, mi->hi_back);

	    XFillRectangle(dpy, mr->w, Scr->NormalGC, 0, y_offset,
		mr->width, Scr->EntryHeight);

	    FBF(mi->hi_fore, mi->hi_back, font->font->fid);

	    XDrawString(dpy, mr->w, Scr->NormalGC, mi->x,
		text_y, mi->item, mi->strlen);

	    gc = Scr->NormalGC;
	}
	else
	{
	    if (mi->user_colors || !exposure)
	    {
		XSetForeground(dpy, Scr->NormalGC, mi->back);

		XFillRectangle(dpy, mr->w, Scr->NormalGC, 0, y_offset,
		    mr->width, Scr->EntryHeight);

		FBF(mi->fore, mi->back, font->font->fid);
		gc = Scr->NormalGC;
	    }
	    else
		gc = Scr->MenuGC;

	    XDrawString(dpy, mr->w, gc, mi->x,
		text_y, mi->item, mi->strlen);
	}

	if (mi->func == F_MENU || mi->func == F_PIEMENU || mi->func == F_MENUFUNC)
	{
	    /* Get or create the pull right menu icon */
	    if (Scr->pullrightPm.pm == None) {
		if (Scr->pullrightPm.name) {
#ifdef XPM
		    XpmColorSymbol ctrans[2];

		    ctrans[0].name = "foreground";
		    ctrans[0].value = NULL;
		    ctrans[0].pixel = Scr->MenuC.fore;
		    ctrans[1].name = "background";
		    ctrans[1].value = NULL;
		    ctrans[1].pixel = Scr->MenuC.back;
#endif

		    Scr->pullrightPm.pm = FindPixmap(Scr->pullrightPm.name,
						&Scr->pullrightPm.width,
						&Scr->pullrightPm.height,
						&Scr->pullrightPm.isXpm,
						NULL,
#ifdef XPM
						ctrans, 2,
#else
						NULL, 0,
#endif
						&Scr->pullrightPm.mask);
		} else {
		    Scr->pullrightPm.pm = CreateMenuIcon(Scr->MenuFont.height,
						&Scr->pullrightPm.width,
						&Scr->pullrightPm.height);
		    Scr->pullrightPm.isXpm = FALSE;
		    Scr->pullrightPm.mask = None;
		}
	    }
	    x = mr->width - Scr->pullrightPm.width - 5;
	    y = y_offset + ((font->height - Scr->pullrightPm.height) / 2);
	    if (Scr->pullrightPm.name && (Scr->pullrightPm.name[0] != '#'))
		XCopyArea(dpy, Scr->pullrightPm.pm, mr->w, gc, 0, 0,
			Scr->pullrightPm.width, Scr->pullrightPm.height, x, y);
	    else
		XCopyPlane(dpy, Scr->pullrightPm.pm, mr->w, gc, 0, 0,
			Scr->pullrightPm.width, Scr->pullrightPm.height,
			x, y, 1);
	}
	if (abs(mr->def) == (mi->item_num+1))
	    XDrawRectangle(dpy, mr->w, gc, 0, text_y-2, 2, 2);
    }
    else
    {
	int y;

	if (Scr->MenuTitleFont.name != NULL )
	    font = &(Scr->MenuTitleFont);
	else
	    font = &(Scr->MenuFont);

	text_y = y_offset + font->y;

	XSetForeground(dpy, Scr->NormalGC, mi->back);

	/* fill the rectangle with the title background color */
	XFillRectangle(dpy, mr->w, Scr->NormalGC, 0, y_offset,
	    mr->width, Scr->EntryHeight);

	if ( Scr->MenuLineWidth > 0 ) /* note we loose the high efficiancy `0' line width */
	{
	    XSetForeground(dpy, Scr->NormalGC, mi->fore);
	    XSetLineAttributes(dpy,Scr->NormalGC,
			       Scr->MenuLineWidth,
			       LineSolid,
			       CapButt,
			       JoinMiter);

	    /* now draw the dividing lines */
	    if (y_offset)
	      XDrawLine (dpy, mr->w, Scr->NormalGC, 0, y_offset,
			 mr->width, y_offset);
	    y = ((mi->item_num+1) * Scr->EntryHeight)-1;
	    XDrawLine(dpy, mr->w, Scr->NormalGC, 0, y, mr->width, y);
	    XSetLineAttributes(dpy,Scr->NormalGC,
			       0,
			       LineSolid,
			       CapButt,
			       JoinMiter);
	}

	FBF(mi->fore, mi->back, font->font->fid);
	/* finally render the title */
	XDrawString(dpy, mr->w, Scr->NormalGC, mi->x,
	    text_y, mi->item, mi->strlen);
	
	if (abs(mr->def) == (mi->item_num+1))
	    XDrawRectangle(dpy, mr->w, Scr->NormalGC, 0, text_y-2, 2, 2);
    }
}



PaintMenu(mr, e)
MenuRoot *mr;
XEvent *e;
{
    MenuItem *mi;

    if (mr->pie_menu) {
        PaintPieMenu(mr);
    } else {
        for (mi = mr->first; mi != NULL; mi = mi->next)
        {
	  int y_offset = mi->item_num * Scr->EntryHeight;

	  /* be smart about handling the expose, redraw only the entries
	   * that we need to
	   */
	  if (e->xexpose.y < (y_offset + Scr->EntryHeight) &&
	      (e->xexpose.y + e->xexpose.height) > y_offset)
	    {
	      PaintEntry(mr, mi, True);
	    }
	}
    }
    XSync(dpy, 0);
}



static Bool fromMenu;

UpdateMenu()
{
    MenuItem *mi;
    int i, x, y, x_root, y_root, entry;
    int awx, awy;
    int done;
    MenuItem *badItem = NULL;
    Window aJunkRoot,aJunkChild;
    unsigned int aJunkMask;
    static int firstTime = True;

    fromMenu = TRUE;

    while (TRUE)
    {
      /* block until there is an event */
	if (!menuFromFrameOrWindowOrTitlebar) {
	    XMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask |
		       EnterWindowMask | ExposureMask |
		       VisibilityChangeMask | LeaveWindowMask |
		       PointerMotionMask, &Event);
	}

	if (Event.type == MotionNotify) {
	    /* discard any extra motion events before a release */
	    while(XCheckMaskEvent(dpy,
				ButtonMotionMask | ButtonReleaseMask, &Event))
		if (Event.type == ButtonRelease)
		    break;
	}

	if (!DispatchEvent ())
	    continue;

	if (Event.type == ButtonRelease)
	{
	    menuFromFrameOrWindowOrTitlebar = FALSE;
	    fromMenu = FALSE;
	    if (Scr->StayUpMenus)
	    {
		if (firstTime == True)
		{
		    /* it was the first release of the button */
		    firstTime = False;
		} /* end if  */
		else
		{
		    /* thats the second we need to return now */
		    firstTime = True;
		    return;
		} /* end else  */
	    } /* end if  */
	    else
	    {
		return;
	    } /* end else  */
	}

	if (Event.type ==  Cancel || !ActiveMenu) {
	    menuFromFrameOrWindowOrTitlebar = FALSE;
	    fromMenu = FALSE;
	    return;
	}

	if (Event.type == LeaveNotify
	    && ((XCrossingEvent *)&Event)->window == ActiveMenu->w) {
	    if (ActiveItem && ActiveItem->func != F_TITLE) {
		ActiveItem->state = 0;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	    ActiveItem = NULL;
	    continue;
	}

	if (Event.type != MotionNotify)
	    continue;

	XQueryPointer( dpy, ActiveMenu->w, &aJunkRoot, &aJunkChild,
	    &x_root, &y_root, &x, &y, &aJunkMask);

	/* if we are in the parent, pop back down, this should prevent
	 * menus that are slow to pop up from hanging around too long.
	 */
	if (ActiveMenu->prev &&
	    ActiveMenu->prev->w == ((XMotionEvent *)&Event)->window) {
	    if (Scr->Shadow)
		XUnmapWindow(dpy, ActiveMenu->shadow);
	    XUnmapWindow(dpy, ActiveMenu->w);
	    ActiveMenu->mapped = UNMAPPED;
	    UninstallRootColormap();
	    if (ActiveItem) {
		ActiveItem->state = 0;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	    ActiveItem = NULL;
	    ActiveMenu = ActiveMenu->prev;
	    MenuDepth--;
	    continue;
	}

	if (((XMotionEvent *)&Event)->window != ActiveMenu->w)
	    continue;

	done = FALSE;

	/* if we haven't received the enter notify yet, wait */
	if (ActiveMenu && !ActiveMenu->entered)
	    continue;

	XFindContext(dpy, ActiveMenu->w, ScreenContext, (caddr_t *)&Scr);

    if (ActiveMenu->pie_menu) {

	/* if they released the button, move the menu around, since we
           still have the pointer grabbed. */

        if (aJunkMask == 0) {
	    awx = x_root - ActiveMenu->center_x;
	    awy = y_root - ActiveMenu->center_y;
	    XMoveWindow(dpy, ActiveMenu->w, awx, awy);
	    /* this is stupid -- shadows look ugly with pie menus. */
            if (Scr->Shadow) {
		XMoveWindow(dpy, ActiveMenu->shadow,
			    awx + SHADOWWIDTH,
			    awy + SHADOWWIDTH);
	    }
	    return;
	}

	mi = CalcPieMenuItem(ActiveMenu, x, y);
	if (mi == NULL) {
	    if (ActiveItem && ActiveItem->func != F_TITLE) {
		ActiveItem->state = 0;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	    ActiveItem = NULL;
	    continue;
	}
	entry = mi->item_num;
    } else {
	if (x < 0 || y < 0 ||
	    x >= ActiveMenu->width || y >= ActiveMenu->height)
	{
	    if (ActiveItem && ActiveItem->func != F_TITLE)
	    {
		ActiveItem->state = 0;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	    ActiveItem = NULL;
	    continue;
	}

	/* look for the entry that the mouse is in */
	entry = y / Scr->EntryHeight;
	for (i = 0, mi = ActiveMenu->first; mi != NULL; i++, mi=mi->next)
	{
	    if (i == entry)
		break;
	}
    }

	/* if there is an active item, we might have to turn it off */
	if (ActiveItem)
	{
	    /* is the active item the one we are on ? */
	    if (ActiveItem->item_num == entry && ActiveItem->state)
		done = TRUE;

	    /* if we weren't on the active entry, let's turn the old
	     * active one off 
	     */
	    if (!done && ActiveItem->func != F_TITLE)
	    {
		ActiveItem->state = 0;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	}

	/* if we weren't on the active item, change the active item and turn
	 * it on 
	 */
	if (!done)
	{
	    ActiveItem = mi;
	    if (ActiveItem->func != F_TITLE && !ActiveItem->state)
	    {
		ActiveItem->state = 1;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	}

	/*
	 * Are we in the right place to popup a submenu?
	 * If we're a regular menu, then this is determined by
	 * PopupSensitivity, which is the rightmost percentage of the
	 * menu item that's sensitive. If a pie menu, then anywhere in
	 * quadrant will do - we skipped the dead zone at the centre of
	 * the menu earlier.
	 */
	if ((ActiveItem->func == F_MENU || ActiveItem->func == F_PIEMENU || ActiveItem->func == F_MENUFUNC) && (ActiveMenu->pie_menu ||
	    (((ActiveMenu->width - x) * 100) <
	    (ActiveMenu->width * Scr->PopupSensitivity))))
	{
	    MenuRoot *save = ActiveMenu;
	    int savex = MenuOrigins[MenuDepth - 1].x;
	    int savey = MenuOrigins[MenuDepth - 1].y;

	    if (MenuDepth < MAXMENUDEPTH) {
	      if (ActiveMenu->pie_menu) {
		PopUpMenu (ActiveItem->sub,
			   savex + (int)ActiveItem->x,
			   savey + (int)ActiveItem->y, False);
	      } else {
		PopUpMenu (ActiveItem->sub,
			   (savex + (int)(ActiveMenu->width *
					(100-Scr->PopupSensitivity) / 100.0)),
			   (savey + ActiveItem->item_num * Scr->EntryHeight)
			   /*(savey + ActiveItem->item_num * Scr->EntryHeight +
			    (Scr->EntryHeight >> 1))*/, False);
	      }
	    } else if (!badItem) {
		XBell (dpy, 0);
		badItem = ActiveItem;
	    }

	    /* if the menu did get popped up, unhighlight the active item */
	    if (save != ActiveMenu && ActiveItem->state)
	    {
		ActiveItem->state = 0;
		PaintEntry(save, ActiveItem, False);
		ActiveItem = NULL;
	    }
	}
	if (badItem != ActiveItem) badItem = NULL;
	XFlush(dpy);
    }

}



/***********************************************************************
 *
 *  Procedure:
 *	NewMenuRoot - create a new menu root
 *
 *  Returned Value:
 *	(MenuRoot *)
 *
 *  Inputs:
 *	name	- the name of the menu root
 *
 ***********************************************************************
 */

MenuRoot *
NewMenuRoot(name)
    char *name;
{
    MenuRoot *tmp;

#define UNUSED_PIXEL ((unsigned long) (~0))   /* more than 24 bits */

    tmp = (MenuRoot *) malloc(sizeof(MenuRoot));
    tmp->hi_fore = UNUSED_PIXEL;
    tmp->hi_back = UNUSED_PIXEL;
    tmp->name = name;
    tmp->file = NULL;
    tmp->mtime = 0;
    tmp->prev = NULL;
    tmp->first = NULL;
    tmp->last = NULL;
    tmp->items = 0;
    tmp->width = 0;
    tmp->mapped = NEVER_MAPPED;
    tmp->pull = FALSE;
    tmp->w = None;
    tmp->shadow = None;
    tmp->real_menu = FALSE;
    tmp->pie_menu = FALSE;
    tmp->initial_angle = 0.0;
    tmp->current = -1;
    tmp->segments = NULL;

    if (Scr->MenuList == NULL)
    {
	Scr->MenuList = tmp;
	Scr->MenuList->next = NULL;
    }

    if (Scr->LastMenu == NULL)
    {
	Scr->LastMenu = tmp;
	Scr->LastMenu->next = NULL;
    }
    else
    {
	Scr->LastMenu->next = tmp;
	Scr->LastMenu = tmp;
	Scr->LastMenu->next = NULL;
    }

    if (strcmp(name, TWM_WINDOWS) == 0)
	Scr->Windows = tmp;

    return (tmp);
}



/***********************************************************************
 *
 *  Procedure:
 *	AddToMenu - add an item to a root menu
 *
 *  Returned Value:
 *	(MenuItem *)
 *
 *  Inputs:
 *	menu	- pointer to the root menu to add the item
 *	item	- the text to appear in the menu
 *	action	- the string to possibly execute
 *	sub	- the menu root if it is a pull-right entry
 *	func	- the numeric function
 *	fore	- foreground color string
 *	back	- background color string
 *
 ***********************************************************************
 */

MenuItem *
AddToMenu(menu, item, action, sub, func, fore, back)
    MenuRoot *menu;
    char *item, *action;
    MenuRoot *sub;
    int func;
    char *fore, *back;
{
    MenuItem *tmp;
    int width;
    MyFont *font;

#ifdef DEBUG_MENUS
    fprintf(stderr, "adding menu item=\"%s\", action=%s, sub=%d, f=%d\n",
	item, action, sub, func);
#endif

    tmp = (MenuItem *) malloc(sizeof(MenuItem));
    tmp->root = menu;

    if (menu->first == NULL)
    {
	menu->first = tmp;
	tmp->prev = NULL;
    }
    else
    {
	menu->last->next = tmp;
	tmp->prev = menu->last;
    }
    menu->last = tmp;

    tmp->item = item;
    tmp->strlen = strlen(item);
    tmp->action = action;
    tmp->next = NULL;
    tmp->sub = NULL;
    tmp->state = 0;
    tmp->func = func;
    tmp->icon = None;
    tmp->mask = None;

    if ( func == F_TITLE && Scr->MenuTitleFont.name != NULL)
	font= &(Scr->MenuTitleFont);
    else
	font= &(Scr->MenuFont);

    if (!Scr->HaveFonts) CreateFonts();
    width = XTextWidth(font->font, item, tmp->strlen);
    if (width <= 0)
	width = 1;
    if ((!menu->pie_menu) && (width > menu->width))
	menu->width = width;

    tmp->user_colors = FALSE;
    if (Scr->Monochrome == COLOR && fore != NULL)
    {
	int save;

	save = Scr->FirstTime;
	Scr->FirstTime = TRUE;
	GetColor(COLOR, &tmp->fore, fore);
	GetColor(COLOR, &tmp->back, back);
	Scr->FirstTime = save;
	tmp->user_colors = TRUE;
    }
    if (sub != NULL)
    {
	tmp->sub = sub;
	menu->pull = TRUE;
    }
    tmp->item_num = menu->items++;

    return (tmp);
}



MakeMenus()
{
    MenuRoot *mr;

    for (mr = Scr->MenuList; mr != NULL; mr = mr->next)
    {
	if (mr->real_menu == FALSE)
	    continue;

	MakeMenu(mr);
    }
}



MakeMenu(mr)
MenuRoot *mr;
{
    MenuItem *start, *end, *cur, *tmp;
    XColor f1, f2, f3;
    XColor b1, b2, b3;
    XColor save_fore, save_back;
    int num, i;
    int fred, fgreen, fblue;
    int bred, bgreen, bblue;
    int width;
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    Colormap cmap = Scr->TwmRoot.cmaps.cwins[0]->colormap->c;
    MyFont *titleFont; 
  
    if ( Scr->MenuTitleFont.name != NULL )
    {
 	Scr->EntryHeight = MAX(Scr->MenuFont.height,
 			       Scr->MenuTitleFont.height) + 4;
 	titleFont= &(Scr->MenuTitleFont);
    }
    else
    {
  	Scr->EntryHeight = Scr->MenuFont.height + 4;
	titleFont= &(Scr->MenuFont);
    }

    /* lets first size the window accordingly */
    if (mr->mapped == NEVER_MAPPED)
    {
        if (mr->pie_menu) {
	    LayoutPieMenu(mr);
	} else {
	  if (mr->items == 0)
	    AddToMenu(mr, "Empty", NULLSTR, NULL, F_TITLE, NULLSTR, NULLSTR);

	  if (mr->pull == TRUE)
	    mr->width += 16 + 10;

	  width = mr->width + 10;

	  for (cur = mr->first; cur != NULL; cur = cur->next)
	    {
	      if (cur->func != F_TITLE)
		cur->x = 5;
	      else
		{
		  cur->x = width - XTextWidth(titleFont->font, cur->item,
		    cur->strlen);
		  cur->x /= 2;
		}
	    }
	  mr->height = mr->items * Scr->EntryHeight;
	  mr->width += 10;
	}
	if (Scr->Shadow)
	{
	    /*
	     * Make sure that you don't draw into the shadow window or else
	     * the background bits there will get saved
	     */
	    valuemask = CWBorderPixel;
	    attributes.border_pixel = Scr->MenuShadowColor;
	    if (Scr->SaveUnder) {
		valuemask |= CWSaveUnder;
		attributes.save_under = True;
	    }
	    if (Scr->shadowPm.name && ! Scr->shadowPm.pm) {
#ifdef XPM
		XpmColorSymbol ctrans[2];

		ctrans[0].name = "foreground";
		ctrans[0].value = NULL;
		ctrans[0].pixel = Scr->MenuShadowColor;
		ctrans[1].name = "background";
		ctrans[1].value = NULL;
		ctrans[1].pixel = Scr->MenuC.back;
#endif
		
		Scr->shadowPm.pm = FindPixmap(Scr->shadowPm.name,
						&Scr->shadowPm.width, 
						&Scr->shadowPm.height,
						&(Scr->shadowPm.isXpm),
						NULL,
#ifdef XPM
						ctrans, 2,
#else
						NULL, 0,
#endif
						&Scr->shadowPm.mask);
		if (! Scr->shadowPm.pm)
		    Scr->shadowPm.name=None;
	    }

	    if (Scr->shadowPm.pm) {
		valuemask |= CWBackPixmap;
		attributes.background_pixmap = Scr->shadowPm.pm;
	    } else {
		valuemask |= CWBackPixel;
		attributes.background_pixel = Scr->MenuShadowColor;
	    }
	    
	    mr->shadow = XCreateWindow (dpy, Scr->Root, 0, 0,
					(unsigned int) mr->width, 
					(unsigned int) mr->height,
					Scr->shadowPm.pm ? (unsigned int)1 :
							(unsigned int)0,
					CopyFromParent, 
					(unsigned int) CopyFromParent,
					(Visual *) CopyFromParent,
					valuemask, &attributes);
	}

	valuemask = (CWBackPixel | CWBorderPixel | CWEventMask);
	attributes.background_pixel = Scr->MenuC.back;
	attributes.border_pixel = Scr->MenuC.fore;
	attributes.event_mask = (ExposureMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | PointerMotionHintMask);
	if (Scr->SaveUnder) {
	    valuemask |= CWSaveUnder;
	    attributes.save_under = True;
	}
	if (Scr->BackingStore) {
	    valuemask |= CWBackingStore;
	    attributes.backing_store = Always;
	}
	mr->w = XCreateWindow (dpy, Scr->Root, 0, 0, (unsigned int) mr->width,
			       (unsigned int) mr->height, (unsigned int) 1,
			       CopyFromParent, (unsigned int) CopyFromParent,
			       (Visual *) CopyFromParent,
			       valuemask, &attributes);


	XSaveContext(dpy, mr->w, MenuContext, (caddr_t)mr);
	XSaveContext(dpy, mr->w, ScreenContext, (caddr_t)Scr);

	mr->mapped = UNMAPPED;
    }

    /* get the default colors into the menus */
    for (tmp = mr->first; tmp != NULL; tmp = tmp->next)
    {
	if (!tmp->user_colors) {
	    if (tmp->func != F_TITLE) {
		tmp->fore = Scr->MenuC.fore;
		tmp->back = Scr->MenuC.back;
	    } else {
		tmp->fore = Scr->MenuTitleC.fore;
		tmp->back = Scr->MenuTitleC.back;
	    }
	}

	if (mr->hi_fore != UNUSED_PIXEL)
	{
	    tmp->hi_fore = mr->hi_fore;
	    tmp->hi_back = mr->hi_back;
	}
	else
	{
	    tmp->hi_fore = tmp->back;
	    tmp->hi_back = tmp->fore;
	}
    }

    if (Scr->Monochrome == MONOCHROME || !Scr->InterpolateMenuColors)
	return;

    start = mr->first;
    while (TRUE)
    {
	for (; start != NULL; start = start->next)
	{
	    if (start->user_colors)
		break;
	}
	if (start == NULL)
	    break;

	for (end = start->next; end != NULL; end = end->next)
	{
	    if (end->user_colors)
		break;
	}
	if (end == NULL)
	    break;

	/* we have a start and end to interpolate between */
	num = end->item_num - start->item_num;

	f1.pixel = start->fore;
	XQueryColor(dpy, cmap, &f1);
	f2.pixel = end->fore;
	XQueryColor(dpy, cmap, &f2);

	b1.pixel = start->back;
	XQueryColor(dpy, cmap, &b1);
	b2.pixel = end->back;
	XQueryColor(dpy, cmap, &b2);

	fred = ((int)f2.red - (int)f1.red) / num;
	fgreen = ((int)f2.green - (int)f1.green) / num;
	fblue = ((int)f2.blue - (int)f1.blue) / num;

	bred = ((int)b2.red - (int)b1.red) / num;
	bgreen = ((int)b2.green - (int)b1.green) / num;
	bblue = ((int)b2.blue - (int)b1.blue) / num;

	f3 = f1;
	f3.flags = DoRed | DoGreen | DoBlue;

	b3 = b1;
	b3.flags = DoRed | DoGreen | DoBlue;

	num -= 1;
	for (i = 0, cur = start->next; i < num; i++, cur = cur->next)
	{
	    f3.red += fred;
	    f3.green += fgreen;
	    f3.blue += fblue;

	    b3.red += bred;
	    b3.green += bgreen;
	    b3.blue += bblue;

	    if (Scr->DontInterpolateTitles && (cur->func == F_TITLE))
		continue;

	    save_fore = f3;
	    save_back = b3;

	    XAllocColor(dpy, cmap, &f3);
	    XAllocColor(dpy, cmap, &b3);
	    cur->hi_back = cur->fore = f3.pixel;
	    cur->hi_fore = cur->back = b3.pixel;
	    cur->user_colors = True;

	    f3 = save_fore;
	    b3 = save_back;
	}
	start = end;
    }
}



 /********************************************************************\
 *                                                                    *
 * HandleChangingMenus -- check if the contents of a menu should be   *
 *                         changed and if so, do so.                  *
 *                                                                    *
 * Used to be part of PopUpMenu, but it is needed by the code to do   *
 * default menu actions too.                                          *
 *                                                                    *
 \********************************************************************/

HandleChangingMenus(menu)
MenuRoot *menu;
{
    int WindowNameOffset, WindowNameCount;
    TwmWindow **WindowNames;
    TwmWindow *tmp_win2, *tmp_win3;
    TwmWindow **sortlist, *bakwin;
    int i, loop, curpos, count;

    int (*compar)() = (Scr->CaseSensitive ? strcmp : XmuCompareISOLatin1);

    if (menu == Scr->Windows) {
	TwmWindow *tmp_win;

	/* this is the twm windows menu,  let's go ahead and build it */

	DestroyMenu(menu);

	menu->first = NULL;
	menu->last = NULL;
	menu->items = 0;
	menu->width = 0;
	menu->mapped = NEVER_MAPPED;
	AddToMenu(menu, "TWM Windows", NULLSTR, NULL, F_TITLE,NULLSTR,NULLSTR);

	/* CODE to SORT THE MENU @@@@@@@@@@@@@@ */
	if (Scr->SortIconMgr) {
	    for(count=0,tmp_win=Scr->TwmRoot.next; tmp_win != NULL;
		tmp_win = tmp_win->next,count++) ;
	    sortlist = (TwmWindow **)malloc(sizeof(TwmWindow *) * count);
	    for(loop=0,tmp_win=Scr->TwmRoot.next; tmp_win != NULL;
		tmp_win = tmp_win->next,loop++)
	    	sortlist[loop]=tmp_win;
	  
	    /* Now that we have them in the list, just do an insertion sort. */
	    for (curpos=1;curpos<count;curpos++) {
		/*Everything from 0 to curpos-1 is sorted.*/
		bakwin = sortlist[curpos];
		for (loop=curpos;(loop>0);loop--)
		    /*Everything from loop to curpos is sorted*/
		    /* Sort order is [0] = min, [n]=max */
		    if (((*compar)(sortlist[loop-1]->name,bakwin->name))>0)
			sortlist[loop]=sortlist[loop-1];
		    else
			break;
		sortlist[loop]=bakwin;
	    }
	  
	    for(loop=0;loop<count;loop++) 
		AddToMenu(menu,sortlist[loop]->name, (char *)sortlist[loop],
			NULL, F_POPUP, NULLSTR, NULLSTR);
	} else {
	    WindowNameOffset=(char *)Scr->TwmRoot.next->name -
				(char *)Scr->TwmRoot.next;
	    for(tmp_win = Scr->TwmRoot.next , WindowNameCount=0;
		tmp_win != NULL;
		tmp_win = tmp_win->next)
		WindowNameCount++;
	    WindowNames =
		    (TwmWindow **)malloc(sizeof(TwmWindow *)*WindowNameCount);
	    WindowNames[0] = Scr->TwmRoot.next;
	    for(tmp_win = Scr->TwmRoot.next->next , WindowNameCount=1;
		tmp_win != NULL; tmp_win = tmp_win->next,WindowNameCount++) {
		tmp_win2 = tmp_win;
		for (i=0;i<WindowNameCount;i++) {
		    if ((*compar)(tmp_win2->name,WindowNames[i]->name) < 0)
		    {
			tmp_win3 = tmp_win2;
			tmp_win2 = WindowNames[i];
			WindowNames[i] = tmp_win3;
		    }
		}
		WindowNames[WindowNameCount] = tmp_win2;
	    }
	    for (i=0; i<WindowNameCount; i++) {
		AddToMenu(menu, WindowNames[i]->name, (char *)WindowNames[i],
			  NULL, F_POPUP,NULL,NULL);
	    }
	    free(WindowNames);
	  }
      MakeMenu(menu);
    }
    else if (menu->file != NULL) {
	struct stat buf;

	if ( stat(menu->file, &buf) == 0 && buf.st_mtime > menu->mtime ) {
	    menu->mtime=buf.st_mtime;

	    menu->real_menu = FALSE;
	    ParseMenuFile(menu->file, menu->name);
	    if (menu->real_menu == FALSE)
		fprintf(stderr,"%s: Menu %s not defined in %s\n",
			ProgramName, menu->name, menu->file);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	PopUpMenu - pop up a pull down menu
 *
 *  Inputs:
 *	menu	- the root pointer of the menu to pop up
 *	x, y	- location of upper left of menu
 *      center	- whether or not to center horizontally over position
 *
 ***********************************************************************
 */

Bool PopUpMenu (menu, x, y, center)
    MenuRoot *menu;
    int x, y;
    Bool center;
{
  if (!menu) return False;

  InstallRootColormap();

  /* Prevent recursively bringing up menus. */
  if (menu->mapped == MAPPED) return False;

  HandleChangingMenus(menu);

  if (menu->w == None || menu->items == 0) return False;

  /*
   * Dynamically set the parent;  this allows pull-ups to also be main
   * menus, or to be brought up from more than one place.
   */
  menu->prev = ActiveMenu;

  XGrabPointer(dpy, Scr->Root, True,
	       ButtonPressMask | ButtonReleaseMask |
	       ButtonMotionMask | PointerMotionHintMask,
	       GrabModeAsync, GrabModeAsync, Scr->Root,
	       menu->pie_menu ? Scr->SelectCursor : Scr->MenuCursor,
	       CurrentTime);

  ActiveMenu = menu;
  menu->mapped = MAPPED;
  menu->entered = FALSE;

  if (menu->pie_menu) {
    x -= menu->center_x;
    y -= menu->center_y;
  } else {
    if (center) {
      x -= (menu->width / 2);
      y -= (Scr->EntryHeight / 2); /* sticky menus would be nice here */
    }

    /* if the menu has a default entry, put it under the cursor */
    if (menu->def)
      y -= (abs(menu->def) - 1) * Scr->EntryHeight;
  }

  /*
   * clip to screen
   */
  if (x + menu->width > Scr->MyDisplayWidth) {
    x = Scr->MyDisplayWidth - menu->width;
  }
  if (x < 0) x = 0;
  if (y + menu->height > Scr->MyDisplayHeight) {
    y = Scr->MyDisplayHeight - menu->height;
  }
  if (y < 0) y = 0;

  MenuOrigins[MenuDepth].x = x;
  MenuOrigins[MenuDepth].y = y;
  MenuDepth++;

  XMoveWindow(dpy, menu->w, x, y);
  if (Scr->Shadow) {
    XMoveWindow (dpy, menu->shadow, x + Scr->ShadowWidth, y + Scr->ShadowWidth);
    XRaiseWindow (dpy, menu->shadow);
  }
  XMapRaised(dpy, menu->w);
  if (!menu->pie_menu) {
    if (Scr->Shadow) {
      XMapWindow (dpy, menu->shadow);
    }
  }
  XSync(dpy, 0);
  return True;
}



/***********************************************************************
 *
 *  Procedure:
 *	PopDownMenu - unhighlight the current menu selection and
 *		take down the menus
 *
 ***********************************************************************
 */

PopDownMenu()
{
    MenuRoot *tmp;

    if (ActiveMenu == NULL)
	return;

    if (ActiveItem)
    {
	ActiveItem->state = 0;
	PaintEntry(ActiveMenu, ActiveItem, False);
    }

    for (tmp = ActiveMenu; tmp != NULL; tmp = tmp->prev)
    {
	if (Scr->Shadow) {
	    XUnmapWindow (dpy, tmp->shadow);
	}
	XUnmapWindow(dpy, tmp->w);
	tmp->mapped = UNMAPPED;
	UninstallRootColormap();
    }

    XFlush(dpy);
    ActiveMenu = NULL;
    ActiveItem = NULL;
    MenuDepth = 0;
    if (Context == C_WINDOW || Context == C_FRAME || Context == C_TITLE)
	menuFromFrameOrWindowOrTitlebar = TRUE;
}



/***********************************************************************
 *
 *  Procedure:
 *	FindMenuRoot - look for a menu root
 *
 *  Returned Value:
 *	(MenuRoot *)  - a pointer to the menu root structure 
 *
 *  Inputs:
 *	name	- the name of the menu root 
 *
 ***********************************************************************
 */

MenuRoot *
FindMenuRoot(name)
    char *name;
{
    MenuRoot *tmp;

    for (tmp = Scr->MenuList; tmp != NULL; tmp = tmp->next)
    {
	if (strcmp(name, tmp->name) == 0)
	    return (tmp);
    }
    return NULL;
}



static Bool belongs_to_twm_window (t, w)
    register TwmWindow *t;
    register Window w;
{
    if (!t) return False;

    if (w == t->frame || w == t->title_w || w == t->hilite_w ||
	w == t->icon.w
	|| w == t->icon.image_w
	|| w == t->icon.text_w) return True;

    if (t && t->titlebuttons) {
	register TBWindow *tbw;
	register int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;
	for (tbw = t->titlebuttons; nb > 0; tbw++, nb--) {
	    if (tbw->window == w) return True;
	}
    }
    return False;
}




/***********************************************************************
 *
 *  Procedure:
 *	resizeFromCenter -
 *
 ***********************************************************************
 */


extern int AddingX;
extern int AddingY;
extern int AddingW;
extern int AddingH;

void resizeFromCenter(w, tmp_win)
    Window w;
    TwmWindow *tmp_win;
{
  int lastx, lasty, width, height, bw2;
  int namelen;
  XEvent event;

  namelen = strlen (tmp_win->name);
  bw2 = tmp_win->frame_bw * 2;
  AddingW = tmp_win->attr.width + bw2;
  AddingH = tmp_win->attr.height + tmp_win->title_height + bw2;
  width = (SIZE_HINDENT + XTextWidth (Scr->SizeFont.font,
				      tmp_win->name, namelen));
  height = Scr->SizeFont.height + SIZE_VINDENT * 2;
  XGetGeometry(dpy, w, &JunkRoot, &origDragX, &origDragY,
	       (unsigned int *)&DragWidth, (unsigned int *)&DragHeight,
	       &JunkBW, &JunkDepth);
  XWarpPointer(dpy, None, w,
	       0, 0, 0, 0, DragWidth/2, DragHeight/2);
  XQueryPointer(dpy, Scr->Root, &JunkRoot,
		&JunkChild, &JunkX, &JunkY,
		&AddingX, &AddingY, &JunkMask);
/*****
  Scr->SizeStringOffset = width +
    XTextWidth(Scr->SizeFont.font, ": ", 2);
  XResizeWindow (dpy, Scr->SizeWindow, Scr->SizeStringOffset +
		 Scr->SizeStringWidth, height);
  XDrawImageString (dpy, Scr->SizeWindow, Scr->NormalGC, width,
		    SIZE_VINDENT + Scr->SizeFont.font->ascent,
		    ": ", 2);
*****/
  lastx = -10000;
  lasty = -10000;
/*****
  MoveOutline(Scr->Root,
	      origDragX - JunkBW, origDragY - JunkBW,
	      DragWidth * JunkBW, DragHeight * JunkBW,
	      tmp_win->frame_bw,
	      tmp_win->title_height);
*****/
  MenuStartResize(tmp_win, origDragX, origDragY, DragWidth, DragHeight);
  while (TRUE)
    {
      XMaskEvent(dpy,
		 ButtonPressMask | PointerMotionMask, &event);

      if (event.type == MotionNotify) {
	/* discard any extra motion events before a release */
	while(XCheckMaskEvent(dpy,
			      ButtonMotionMask | ButtonPressMask, &event))
	  if (event.type == ButtonPress)
	    break;
      }

      if (event.type == ButtonPress)
      {
	MenuEndResize(tmp_win);
	XMoveResizeWindow(dpy, w, AddingX, AddingY, AddingW, AddingH);
	break;
      }

/*    if (!DispatchEvent ()) continue; */

      if (event.type != MotionNotify) {
	continue;
      }

      /*
       * XXX - if we are going to do a loop, we ought to consider
       * using multiple GXxor lines so that we don't need to
       * grab the server.
       */
      XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkChild,
		    &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

      if (lastx != AddingX || lasty != AddingY)
      {
	MenuDoResize(AddingX, AddingY, tmp_win);

	lastx = AddingX;
	lasty = AddingY;
      }

    }
}



/***********************************************************************
 *
 *  Procedure:
 *	RestartTwm - Restart Twm
 ***********************************************************************
 */

void
RestartTwm(time)
    Time time;
{
    XSync (dpy, 0);
    Reborder (time);
    XSync (dpy, 0);
    execvp(*Argv, Argv);
    fprintf (stderr, "%s:  unable to restart:  %s\n", ProgramName, *Argv);
}


/***********************************************************************
 *
 *  Procedure:
 *	ExecuteFunction - execute a twm root function
 *
 *  Inputs:
 *	func	- the function to execute
 *	action	- the menu action to execute 
 *	sub	- pullright menu, if any
 *	w	- the window to execute this function on
 *	tmp_win	- the twm window structure
 *	event	- the event that caused the function
 *	context - the context in which the button was pressed
 *	pulldown- flag indicating execution from pull down menu
 *
 *  Returns:
 *	TRUE if should continue with remaining actions else FALSE to abort
 *
 ***********************************************************************
 */

/* for F_WARPTO */
int
#ifdef __STDC__
WarpThere(TwmWindow *t)
#else
WarpThere(t)
TwmWindow	*t;
#endif
{
    if (Scr->WarpUnmapped || t->mapped) {
	if (!t->mapped)
	    DeIconify(t);
	if (!Scr->NoRaiseWarp)
	    RaiseFrame(t);
	WarpToWindow(t);
	return TRUE;
    }
    return FALSE;
}

static struct MenuItem *
GetDefaultAction(menu)
struct MenuRoot *menu;
{
    struct MenuItem *item;
    int i;

    if (!menu)
	return NULL;

    HandleChangingMenus(menu);

    if (!menu->def)
	return NULL;

    for(item = menu->first, i=abs(menu->def) ; (--i > 0) && (item) ;
			item=item->next)
	;

    if (item->func == F_MENU)
	return GetDefaultAction(item->sub);
    else
	return item;
}

int
ExecuteFunction(func, action, sub, w, tmp_win, eventp, context, pulldown)
    int func;
    char *action;
    struct MenuRoot *sub;
    Window w;
    TwmWindow *tmp_win;
    XEvent *eventp;
    int context;
    int pulldown;
{
    short Real_OpaqueMove;                  /* holder for F_OPAQUEMOVE */
    char tmp[200];
    char *ptr;
    char buff[MAX_FILE_SIZE];
    int count, fd;
    Window rootw;
    int do_next_action = TRUE;
    int moving_icon = FALSE;
    Bool fromtitlebar = False;
    struct MenuItem *item;

    RootFunction = 0;
    if (Cancel)
	return TRUE;			/* XXX should this be FALSE? */

    switch (func)
    {
    case F_UPICONMGR:
    case F_LEFTICONMGR:
    case F_RIGHTICONMGR:
    case F_DOWNICONMGR:
    case F_FORWICONMGR:
    case F_BACKICONMGR:
    case F_NEXTICONMGR:
    case F_PREVICONMGR:
    case F_NOP:
    case F_TITLE:
    case F_DELTASTOP:
    case F_RAISELOWER:
    case F_WARPTOSCREEN:
    case F_WARPTO:
    case F_WARPRING:
    case F_WARPTOICONMGR:
    case F_COLORMAP:
	break;
    default:
        XGrabPointer(dpy, Scr->Root, True,
            ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync,
            Scr->Root, Scr->WaitCursor, CurrentTime);
	break;
    }

    switch (func)
    {
    case F_NOP:
    case F_TITLE:
	break;

    case F_MENU:
    case F_PIEMENU:
	item = GetDefaultAction(sub);

	if (item == NULL) {
	    XBell(dpy, 0);
	    return FALSE;
	}

	if (Deferrable(item->func) && DeferExecution(context, item->func,
					item->action, Scr->SelectCursor))
	    return TRUE;

	return ExecuteFunction(item->func, 
				item->action, 
				item->sub, 
				w, tmp_win, 
				eventp, context, pulldown);
	break;

    case F_PANNER:
	if (Scr->Panner) {
	    if (XFindContext (dpy, Scr->Panner, TwmContext, (caddr_t *)&tmp_win) != XCSUCCESS)
		tmp_win = AddWindow(Scr->Panner, FALSE, NULL);
	    if (tmp_win->mapped) {
		UnmapFrame(tmp_win);
		tmp_win->mapped = FALSE;
	    }
	    else {
		if (tmp_win->isicon)
		    DeIconify(tmp_win);
		else
		    MapFrame(tmp_win);
		tmp_win->mapped = TRUE;
	    }
	}
	break;

    case F_SCROLL:
    case F_SCROLLHOME:
    case F_SCROLLLEFT:
    case F_SCROLLRIGHT:
    case F_SCROLLUP:
    case F_SCROLLDOWN:
    case F_SCROLLBACK:
	ScrollDesktop(func, action);
	break;

    case F_DELTASTOP:
	if (WindowMoved) do_next_action = FALSE;
	break;

    case F_RESTART:
	RestartTwm(eventp->xbutton.time);
	break;

    case F_UPICONMGR:
    case F_DOWNICONMGR:
    case F_LEFTICONMGR:
    case F_RIGHTICONMGR:
    case F_FORWICONMGR:
    case F_BACKICONMGR:
	MoveIconManager(func);
        break;

    case F_NEXTICONMGR:
    case F_PREVICONMGR:
	JumpIconManager(func);
        break;

    case F_SHOWLIST:
	if (Scr->NoIconManagers)
	    break;
	DeIconify(Scr->iconmgr.twm_win);
	RaiseFrame(Scr->iconmgr.twm_win);
	break;

    case F_HIDELIST:
	if (Scr->NoIconManagers)
	    break;
	HideIconManager ();
	break;

    case F_STICK:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	if (Scr->VirtualDesktop) {
	    if (tmp_win->sticky) {
		if (Scr->StickyAbove) {
		    XReparentWindow(dpy, tmp_win->frame, Scr->VirtualDesktop,
			tmp_win->frame_x + Scr->vdtPositionX,
			tmp_win->frame_y + Scr->vdtPositionY);
		    tmp_win->frame_x += Scr->vdtPositionX;
		    tmp_win->frame_y += Scr->vdtPositionY;
		    XMoveWindow(dpy, tmp_win->virtualWindow,
			tmp_win->frame_x / Scr->PannerScale,
			tmp_win->frame_y / Scr->PannerScale);
		    if (tmp_win->icon.w) {
			XReparentWindow(dpy, tmp_win->icon.w, Scr->VirtualDesktop,
			    tmp_win->icon_loc_x + Scr->vdtPositionX,
			    tmp_win->icon_loc_y + Scr->vdtPositionY);
			tmp_win->icon_loc_x += Scr->vdtPositionX;
			tmp_win->icon_loc_y += Scr->vdtPositionY;
			XMoveWindow(dpy, tmp_win->virtualIcon,
			    tmp_win->icon_loc_x / Scr->PannerScale,
			    tmp_win->icon_loc_y / Scr->PannerScale);
		    }
		    tmp_win->root = Scr->VirtualDesktop;
		}
		if (!tmp_win->isicon)
		    XMapRaised(dpy, tmp_win->virtualWindow);
		else if (tmp_win->virtualIcon)
		    XMapRaised(dpy, tmp_win->virtualIcon);
		tmp_win->sticky = False;
		SetSWM_ROOT(tmp_win);
		SetTWM_FLAGS(tmp_win);
	    }
	    else {
		if (Scr->StickyAbove) {
		    XReparentWindow(dpy, tmp_win->frame, Scr->Root,
			tmp_win->frame_x - Scr->vdtPositionX,
			tmp_win->frame_y - Scr->vdtPositionY);
		    tmp_win->frame_x -= Scr->vdtPositionX;
		    tmp_win->frame_y -= Scr->vdtPositionY;
		    if (tmp_win->icon.w) {
			XReparentWindow(dpy, tmp_win->icon.w, Scr->Root,
			    tmp_win->icon_loc_x - Scr->vdtPositionX,
			    tmp_win->icon_loc_y - Scr->vdtPositionY);
			tmp_win->icon_loc_x -= Scr->vdtPositionX;
			tmp_win->icon_loc_y -= Scr->vdtPositionY;
		    }
		    tmp_win->root = Scr->Root;
		}
		XUnmapWindow(dpy, tmp_win->virtualWindow);
		if (tmp_win->virtualIcon)
		    XUnmapWindow(dpy, tmp_win->virtualIcon);
		tmp_win->sticky = True;
		SetSWM_ROOT(tmp_win);
		SetTWM_FLAGS(tmp_win);
	    }
	}
	break;

    case F_SORTICONMGR:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	{
	    int save_sort;

	    save_sort = Scr->SortIconMgr;
	    Scr->SortIconMgr = TRUE;

	    if (context == C_ICONMGR)
		SortIconManager((IconMgr *) NULL);
	    else if (tmp_win->iconmgr)
		SortIconManager(tmp_win->iconmgrp);
	    else
		XBell(dpy, 0);

	    Scr->SortIconMgr = save_sort;
	}
	break;

    case F_IDENTIFY:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	Identify(tmp_win);
	break;

    case F_VERSION:
	Identify ((TwmWindow *) NULL);
	break;

    case F_AUTORAISE:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	tmp_win->auto_raise = !tmp_win->auto_raise;
	if (tmp_win->auto_raise) ++(Scr->NumAutoRaises);
	else --(Scr->NumAutoRaises);
	break;

    case F_BEEP:
	XBell(dpy, 0);
	break;

    case F_POPUP:
	tmp_win = (TwmWindow *)action;
	if (Scr->WindowFunction.func != 0)
	{
	   ExecuteFunction(Scr->WindowFunction.func,
			   Scr->WindowFunction.item->action,
			   NULL,
			   w, tmp_win, eventp, C_FRAME, FALSE);
	}
	else
	{
	    DeIconify(tmp_win);
	    if (Scr->VirtualDesktop)
		ScrollToQuadrant(tmp_win);
	    RaiseFrame (tmp_win);
	}
	break;

    case F_RELATIVERESIZE:
    case F_RESIZE:
	EventHandler[EnterNotify] = HandleUnknown;
	EventHandler[LeaveNotify] = HandleUnknown;
	if (DeferExecution(context, func, action, Scr->MoveCursor))
	    return TRUE;

	PopDownMenu();
	if (pulldown)
	    XWarpPointer(dpy, None, Scr->Root, 
		0, 0, 0, 0, eventp->xbutton.x_root, eventp->xbutton.y_root);

	if (w != tmp_win->icon.w) {	/* can't resize icons */

	  if ((Context == C_FRAME || Context == C_WINDOW || Context == C_TITLE)
	       && fromMenu)
	    resizeFromCenter(w, tmp_win);
	  else {
	    /*
	     * see if this is being done from the titlebar
	     */
	    fromtitlebar =
	      belongs_to_twm_window (tmp_win, eventp->xbutton.window);

	    /* Save pointer position so we can tell if it was moved or
	       not during the resize. */
	    ResizeOrigX = eventp->xbutton.x_root;
	    ResizeOrigY = eventp->xbutton.y_root;

	    if (func == F_RELATIVERESIZE) {
		short Real_AutoRelativeResize = Scr->AutoRelativeResize;
		Scr->AutoRelativeResize = TRUE;
		StartResize (eventp, tmp_win, fromtitlebar);
		Scr->AutoRelativeResize = Real_AutoRelativeResize;
	    } else {
		StartResize (eventp, tmp_win, fromtitlebar);
	    }

	    do {
	      XMaskEvent(dpy,
			 ButtonPressMask | ButtonReleaseMask |
			 EnterWindowMask | LeaveWindowMask |
			 ButtonMotionMask, &Event);

		if (fromtitlebar && Event.type == ButtonPress) {
		  fromtitlebar = False;
		  continue;
		}

		if (Event.type == MotionNotify) {
		  /* discard any extra motion events before a release */
		  while
		    (XCheckMaskEvent
		     (dpy, ButtonMotionMask | ButtonReleaseMask, &Event))
		      if (Event.type == ButtonRelease)
			break;
		}

	      if (!DispatchEvent ()) continue;

	    } while (!(Event.type == ButtonRelease || Cancel));
	    return TRUE;
	}
	}
	break;


    case F_ZOOM:
    case F_HORIZOOM:
    case F_FULLZOOM:
    case F_LEFTZOOM:
    case F_RIGHTZOOM:
    case F_TOPZOOM:
    case F_BOTTOMZOOM:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;
	fullzoom(tmp_win, func);
	break;

    case F_MOVE:
    case F_FORCEMOVE:
    case F_CONSTRAINEDMOVE:
    case F_OPAQUEMOVE:
	if (DeferExecution(context, func, action, Scr->MoveCursor))
	    return TRUE;

	PopDownMenu();
	rootw = eventp->xbutton.root;
	MoveFunction = func;
	if (MoveFunction == F_OPAQUEMOVE) {
	    Real_OpaqueMove = Scr->OpaqueMove;
	    Scr->OpaqueMove = TRUE;
	}
	
	if (pulldown)
	    XWarpPointer(dpy, None, Scr->Root, 
		0, 0, 0, 0, eventp->xbutton.x_root, eventp->xbutton.y_root);

	if (context == C_ICON && tmp_win->icon.w)
	{
	    DragIcon(tmp_win, eventp, pulldown);
	}
	else if (w != tmp_win->icon.w)
	{
	    DragFrame(tmp_win, eventp, pulldown);
	}

	if (MoveFunction == F_OPAQUEMOVE)
	  Scr->OpaqueMove = Real_OpaqueMove ;

        break;

    case F_FUNCTION:
    case F_MENUFUNC:
	{
	    MenuRoot *mroot;
	    MenuItem *mitem;

	    if ((mroot = FindMenuRoot(action)) == NULL)
	    {
		fprintf (stderr, "%s: couldn't find function \"%s\"\n", 
			 ProgramName, action);
		return TRUE;
	    }

	    if (NeedToDefer(mroot) && DeferExecution(context, func, action, Scr->SelectCursor))
		return TRUE;
	    else
	    {
		for (mitem = mroot->first; mitem != NULL; mitem = mitem->next)
		{
		    if (!ExecuteFunction (mitem->func, mitem->action, NULL, w,
					  tmp_win, eventp, context, pulldown))
		      break;
		}
	    }
	}
	break;

    case F_DEICONIFY:
    case F_ICONIFY:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	if (tmp_win->isicon || !tmp_win->mapped )
	{
	    DeIconify(tmp_win);
	}
        else if (func == F_ICONIFY)
	{
	    Iconify (tmp_win, eventp->xbutton.x_root - 5,
		     eventp->xbutton.y_root - 5);
	}
	break;

    case F_RAISELOWER:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	if (!WindowMoved) {
	    Window virtual;
	    XWindowChanges xwc;

	    xwc.stack_mode = Opposite;
	    virtual = tmp_win->virtualIcon;
	    if (w != tmp_win->icon.w) {
		w = tmp_win->frame;
		virtual = tmp_win->virtualWindow;
	    }
	    XConfigureWindow (dpy, w, CWStackMode, &xwc);
	    if (tmp_win->sticky && Scr->VirtualDesktop)
		XLowerWindow(dpy, Scr->VirtualDesktop);
	    if (virtual)
		XConfigureWindow (dpy, virtual, CWStackMode, &xwc);
	}
	break;
	
    case F_RAISE:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	/* check to make sure raise is not from the WindowFunction */
	if (w == tmp_win->icon.w && Context != C_ROOT)
	    RaiseIcon(tmp_win);
	else
	    RaiseFrame(tmp_win);

	break;

    case F_LOWER:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	if (w == tmp_win->icon.w)
	    LowerIcon(tmp_win);
	else
	    LowerFrame(tmp_win);

	break;

    case F_FOCUS:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	if (tmp_win->isicon == FALSE)
	{
	    if (!Scr->FocusRoot && Scr->Focus == tmp_win)
	    {
		FocusOnRoot();
	    }
	    else
	    {
		if (Scr->Focus != NULL) {
		    SetBorder (Scr->Focus, False);
		    if (Scr->Focus->hilite_w)
		      XUnmapWindow (dpy, Scr->Focus->hilite_w);
		}

		InstallWindowColormaps (0, tmp_win);
		if (tmp_win->hilite_w) XMapWindow (dpy, tmp_win->hilite_w);
		SetBorder (tmp_win, True);
		SetFocus (tmp_win, eventp->xbutton.time);
		Scr->FocusRoot = FALSE;
		Scr->Focus = tmp_win;
	    }
	}
	break;

    case F_DESTROY:
	if (DeferExecution(context, func, action, Scr->DestroyCursor))
	    return TRUE;

	if (tmp_win->iconmgr)
	    XBell(dpy, 0);
	else if (tmp_win->w == Scr->Panner) {
	    UnmapFrame(tmp_win);
	    tmp_win->mapped = FALSE;
	}
	else
	    XKillClient(dpy, tmp_win->w);
	break;

    case F_DELETE:
	if (DeferExecution(context, func, action, Scr->DestroyCursor))
	    return TRUE;

	if (tmp_win->iconmgr)		/* don't send ourself a message */
	  HideIconManager ();
	else if (tmp_win->protocols & DoesWmDeleteWindow)
	  SendDeleteWindowMessage (tmp_win, LastTimestamp());
	else
	  XBell (dpy, 0);
	break;

    case F_DELETEORDESTROY:
	if (DeferExecution(context, func, action, Scr->DestroyCursor))
	    return TRUE;

	if (tmp_win->iconmgr)		/* don't send ourself a message */
	    HideIconManager();
	else if (tmp_win->w == Scr->Panner) {
	    UnmapFrame(tmp_win);
	    tmp_win->mapped = FALSE;
	} else if (tmp_win->protocols & DoesWmDeleteWindow)
	    SendDeleteWindowMessage (tmp_win, LastTimestamp());
	else
	    XKillClient(dpy, tmp_win->w);
	break;

    case F_SAVEYOURSELF:
	if (DeferExecution (context, func, action, Scr->SelectCursor))
	  return TRUE;

	if (tmp_win->protocols & DoesWmSaveYourself)
	  SendSaveYourselfMessage (tmp_win, LastTimestamp());
	else
	  XBell (dpy, 0);
	break;

    case F_CIRCLEUP:
	if (Scr->VirtualDesktop) {
	    XCirculateSubwindowsUp(dpy, Scr->VirtualDesktop);
	    XCirculateSubwindowsUp(dpy, Scr->Panner);
	}
	else
	    XCirculateSubwindowsUp(dpy, Scr->Root);
	break;

    case F_CIRCLEDOWN:
	if (Scr->VirtualDesktop) {
	    XCirculateSubwindowsDown(dpy, Scr->VirtualDesktop);
	    XCirculateSubwindowsDown(dpy, Scr->Panner);
	}
	else
	    XCirculateSubwindowsDown(dpy, Scr->Root);
	break;

    case F_EXEC:
	PopDownMenu();
	if (!Scr->NoGrabServer) {
	    XUngrabServer (dpy);
	    XSync (dpy, 0);
	}
	(void)Execute(action, tmp_win);
	break;

    case F_WINEXEC:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	PopDownMenu();
	if (!Scr->NoGrabServer)
	    XUngrabServer(dpy);
	XUngrabPointer(dpy, CurrentTime);
	XSync(dpy, 0);
	(void)Execute(action, tmp_win);
	break;

    case F_TESTEXEC:
	PopDownMenu();
	if (!Scr->NoGrabServer) {
	    XUngrabServer (dpy);
	    XSync (dpy, 0);
	}
	do_next_action= (Execute(action, tmp_win)==0);
	if ( !do_next_action )
	    XBell(dpy,0);
	break;

    case F_TESTWINEXEC:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	PopDownMenu();
	if (!Scr->NoGrabServer)
	    XUngrabServer(dpy);
	XUngrabPointer(dpy, CurrentTime);
	XSync(dpy, 0);
	do_next_action = (Execute(action, tmp_win) == 0);
	if (!do_next_action)
	    XBell(dpy, 0);
	break;

    case F_UNFOCUS:
	FocusOnRoot();
	break;

    case F_CUT:
	strcpy(tmp, action);
	strcat(tmp, "\n");
	XStoreBytes(dpy, tmp, strlen(tmp));
	break;

    case F_CUTFILE:
	ptr = XFetchBytes(dpy, &count);
	if (ptr) {
	    if (sscanf (ptr, "%s", tmp) == 1) {
		XFree (ptr);
		ptr = ExpandFilename(tmp);
		if (ptr) {
		    fd = open (ptr, 0);
		    if (fd >= 0) {
			count = read (fd, buff, MAX_FILE_SIZE - 1);
			if (count > 0) XStoreBytes (dpy, buff, count);
			close(fd);
		    } else {
			fprintf (stderr, 
				 "%s:  unable to open cut file \"%s\"\n", 
				 ProgramName, tmp);
		    }
		    if (ptr != tmp) free (ptr);
		} 
	    } else {
		XFree(ptr);
	    }
	} else {
	    fprintf(stderr, "%s:  cut buffer is empty\n", ProgramName);
	}
	break;

    case F_WARPTOSCREEN:
	{
	    if (strcmp (action, WARPSCREEN_NEXT) == 0) {
		WarpToScreen (Scr->screen + 1, 1);
	    } else if (strcmp (action, WARPSCREEN_PREV) == 0) {
		WarpToScreen (Scr->screen - 1, -1);
	    } else if (strcmp (action, WARPSCREEN_BACK) == 0) {
		WarpToScreen (PreviousScreen, 0);
	    } else {
		WarpToScreen (atoi (action), 0);
	    }
	}
	break;

    case F_COLORMAP:
	{
	    if (strcmp (action, COLORMAP_NEXT) == 0) {
		BumpWindowColormap (tmp_win, 1);
	    } else if (strcmp (action, COLORMAP_PREV) == 0) {
		BumpWindowColormap (tmp_win, -1);
	    } else {
		BumpWindowColormap (tmp_win, 0);
	    }
	}
	break;

    case F_WARPTO:
	{
	    register TwmWindow *t;
	    int len;

	    len = strlen(action);

	    if (len == 0) {
		if (!tmp_win)
		    XBell (dpy,0);
		else
		    WarpThere(tmp_win);
	    } else {
		for (t = Scr->TwmRoot.next; t != NULL; t = t->next) {
		    if (!strncmp(action, t->name, len))
			if (WarpThere(t))
			    break;
		}
		if (!t) {
		    for (t = Scr->TwmRoot.next; t != NULL; t = t->next) {
			if (!strncmp(action, t->class.res_name, len)) 
			    if (WarpThere(t))
			    	break;
		    }
		    if (!t) {
			for (t = Scr->TwmRoot.next; t != NULL; t = t->next) {
			    if (!strncmp(action, t->class.res_class, len))
				if (WarpThere(t))
				    break;
			}
		    }
		}

		if (!t)
		    XBell (dpy, 0);
	    }
	}
	break;

    case F_WARPTOICONMGR:
	{
	    TwmWindow *t;
	    int len;
	    Window raisewin = None, iconwin = None;
	    TwmWindow *raiseFrame;

	    len = strlen(action);
	    if (len == 0) {
		if (tmp_win && tmp_win->list) {
		    raisewin = tmp_win->list->iconmgr->twm_win->frame;
		    raiseFrame = tmp_win->list->iconmgr->twm_win;
		    iconwin = tmp_win->list->icon;
		} else if (Scr->iconmgr.active) {
		    raisewin = Scr->iconmgr.twm_win->frame;
		    raiseFrame = Scr->iconmgr.twm_win;
		    iconwin = Scr->iconmgr.active->w;
		}
	    } else {
		for (t = Scr->TwmRoot.next; t != NULL; t = t->next) {
		    if (strncmp (action, t->icon_name, len) == 0) {
			if (t->list && t->list->iconmgr->twm_win->mapped) {
			    raisewin = t->list->iconmgr->twm_win->frame;
			    raiseFrame = t->list->iconmgr->twm_win;
			    iconwin = t->list->icon;
			    break;
			}
		    }
		}
	    }

	    if (raisewin) {
		RaiseFrame (raiseFrame);
		ScrollToQuadrant(raiseFrame);
		XWarpPointer (dpy, None, iconwin, 0,0,0,0, 5, 5);
	    } else {
		XBell (dpy, 0);
	    }
	}
	break;
	
    case F_WARPRING:
	switch (action[0]) {
	  case 'n':
	    WarpAlongRing (&eventp->xbutton, True);
	    break;
	  case 'p':
	    WarpAlongRing (&eventp->xbutton, False);
	    break;
	  default:
	    XBell (dpy, 0);
	    break;
	}
	break;

    case F_FILE:
	action = ExpandFilename(action);
	fd = open(action, 0);
	if (fd >= 0)
	{
	    count = read(fd, buff, MAX_FILE_SIZE - 1);
	    if (count > 0)
		XStoreBytes(dpy, buff, count);

	    close(fd);
	}
	else
	{
	    fprintf (stderr, "%s:  unable to open file \"%s\"\n", 
		     ProgramName, action);
	}
	break;

    case F_REFRESH:
	{
	    XSetWindowAttributes attributes;
	    unsigned long valuemask;

	    valuemask = (CWBackPixel | CWBackingStore | CWSaveUnder);
	    attributes.background_pixel = Scr->Black;
	    attributes.backing_store = NotUseful;
	    attributes.save_under = False;
	    w = XCreateWindow (dpy, Scr->Root, 0, 0,
			       (unsigned int) Scr->MyDisplayWidth,
			       (unsigned int) Scr->MyDisplayHeight,
			       (unsigned int) 0,
			       CopyFromParent, (unsigned int) CopyFromParent,
			       (Visual *) CopyFromParent, valuemask,
			       &attributes);
	    XMapWindow (dpy, w);
	    XDestroyWindow (dpy, w);
	    XFlush (dpy);
	}
	break;

    case F_WINREFRESH:
	if (DeferExecution(context, func, action, Scr->SelectCursor))
	    return TRUE;

	if (context == C_ICON && tmp_win->icon.w)
	    w = XCreateSimpleWindow(dpy, tmp_win->icon.w,
		0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);
	else
	    w = XCreateSimpleWindow(dpy, tmp_win->frame,
		0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);

	XMapWindow(dpy, w);
	XDestroyWindow(dpy, w);
	XFlush(dpy);
	break;

    case F_RELATIVEMOVE:
	if (tmp_win && !tmp_win->isicon && tmp_win->mapped)
	    RelativeMove(tmp_win, action);
	break;

    case F_QUIT:
	Done();
	break;
    }

    if (ButtonPressed == -1) XUngrabPointer(dpy, CurrentTime);
    return do_next_action;
}



/***********************************************************************
 *
 *  Procedure:
 *	DeferExecution - defer the execution of a function to the
 *	    next button press if the context is C_ROOT
 *
 *  Inputs:
 *	context	- the context in which the mouse button was pressed
 *	func	- the function to defer
 *	cursor	- the cursor to display while waiting
 *
 ***********************************************************************
 */

int
DeferExecution(context, func, action, cursor)
int context, func;
char *action;
Cursor cursor;
{
    if (context == C_ROOT)
    {
	LastCursor = cursor;
	XGrabPointer(dpy, Scr->Root, True,
	    ButtonPressMask | ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync,
	    Scr->Root, cursor, CurrentTime);

	RootAction = action;
	RootFunction = func;

	return (TRUE);
    }
    
    return (FALSE);
}



/***********************************************************************
 *
 *  Procedure:
 *	ReGrab - regrab the pointer with the LastCursor;
 *
 ***********************************************************************
 */

ReGrab()
{
    XGrabPointer(dpy, Scr->Root, True,
	ButtonPressMask | ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync,
	Scr->Root, LastCursor, CurrentTime);
}



/***********************************************************************
 *
 *  Procedure:
 *	NeedToDefer - checks each function in the list to see if it
 *		is one that needs to be defered.
 *
 *  Inputs:
 *	root	- the menu root to check
 *
 ***********************************************************************
 */

Deferrable(func)
int func;
{
    switch (func) {
	case F_IDENTIFY:
	case F_RESIZE:
	case F_MOVE:
	case F_FORCEMOVE:
	case F_DEICONIFY:
	case F_ICONIFY:
	case F_RAISELOWER:
	case F_RAISE:
	case F_LOWER:
	case F_FOCUS:
	case F_DESTROY:
	case F_DELETE:
	case F_DELETEORDESTROY:
	case F_WINREFRESH:
	case F_ZOOM:
	case F_FULLZOOM:
	case F_HORIZOOM:
        case F_RIGHTZOOM:
        case F_LEFTZOOM:
        case F_TOPZOOM:
        case F_BOTTOMZOOM:
	case F_AUTORAISE:
	case F_WINEXEC:
	case F_TESTWINEXEC:
	    return TRUE;
	default:
	    return FALSE;
    }
}

NeedToDefer(root)
MenuRoot *root;
{
    MenuItem *mitem;

    for (mitem = root->first; mitem != NULL; mitem = mitem->next)
	if (Deferrable(mitem->func))
	    return TRUE;
    return FALSE;
}



/***********************************************************************
 *
 *  Procedure:
 *	Execute - execute the string by /bin/sh
 *
 *  Inputs:
 *	s	- the string containing the command
 *
 ***********************************************************************
 */

int
Execute(s, tmp_win)
    char *s;
    TwmWindow *tmp_win;
{
    static char buf1[256];
    static char buf2[256];
    static char buf3[256];
    static char buf4[256];
    static char buf5[256];
    static char buf6[256];
    char *ds = DisplayString (dpy);
    char *colon, *dot1;
    char oldDisplay[256];
    char *doisplay;
    int restorevar = 0;
    int status;

    oldDisplay[0] = '\0';
    doisplay=getenv("DISPLAY");
    if (doisplay)
	strcpy (oldDisplay, doisplay);

    /*
     * Build a display string using the current screen number, so that
     * X programs which get fired up from a menu come up on the screen
     * that they were invoked from, unless specifically overridden on
     * their command line.
     */
    colon = rindex (ds, ':');
    if (colon) {			/* if host[:]:dpy */
	strcpy (buf1, "DISPLAY=");
	strcat (buf1, ds);
	colon = buf1 + 8 + (colon - ds);	/* use version in buf */
	dot1 = index (colon, '.');	/* first period after colon */
	if (!dot1) dot1 = colon + strlen (colon);  /* if not there, append */
	(void) sprintf (dot1, ".%d", Scr->screen);
	putenv (buf1);
	restorevar = 1;
    }

    if (tmp_win != NULL ) {
	sprintf(buf2, "TWM_WIN=0x%lx", tmp_win->w);
	putenv(buf2);
	sprintf(buf3, "TWM_WIN_NAME=%s", tmp_win->full_name);
	putenv(buf3);
	sprintf(buf4, "TWM_ICON_NAME=%s", tmp_win->icon_name);
	putenv(buf4);
	sprintf(buf5, "TWM_RES_NAME=%s", tmp_win->class.res_name);
	putenv(buf5);
	sprintf(buf6, "TWM_RES_CLASS=%s", tmp_win->class.res_class);
	putenv(buf6);
    }

    status=system (s);

    /* this next bit may be horribly BSD specific. */
    if ( (status&0xff) == 0 )
	status = (status &0xff00) >> 8;

    if (restorevar) {		/* why bother? */
	(void) sprintf (buf1, "DISPLAY=%s", oldDisplay);
	putenv (buf1);
    }

    return status;
}



/***********************************************************************
 *
 *  Procedure:
 *	FocusOnRoot - put input focus on the root window
 *
 ***********************************************************************
 */

void
FocusOnRoot()
{
    SetFocus ((TwmWindow *) NULL, LastTimestamp());
    if (Scr->Focus != NULL)
    {
	SetBorder (Scr->Focus, False);
	if (Scr->Focus->hilite_w) XUnmapWindow (dpy, Scr->Focus->hilite_w);
    }
    InstallWindowColormaps(0, &Scr->TwmRoot);
    Scr->Focus = NULL;
    Scr->FocusRoot = TRUE;
}

DeIconify(tmp_win)
TwmWindow *tmp_win;
{
    TwmWindow *t;

    /* de-iconify the main window */
    if (tmp_win->isicon)
    {
	if (tmp_win->icon_on)
	    Zoom(tmp_win->icon.w, tmp_win->frame);
	else if (tmp_win->group != 0)
	{
	    for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
	    {
		if (tmp_win->group == t->w && t->icon_on)
		{
		    Zoom(t->icon.w, tmp_win->frame);
		    break;
		}
	    }
	}
    }


    tmp_win->mapped = TRUE;
    if (!Scr->NoRaiseDeicon)
	RaiseFrame(tmp_win);
    MapFrame(tmp_win);
    SetMapStateProp(tmp_win, NormalState);

    if (tmp_win->icon.w) {
	UnmapIcon(tmp_win);
	IconDown (tmp_win);
    }
    if (tmp_win->list)
	XUnmapWindow(dpy, tmp_win->list->icon);
    if ((Scr->WarpCursor ||
	 LookInList(Scr->WarpCursorL, tmp_win->full_name, &tmp_win->class)) &&
	tmp_win->isicon)
      WarpToWindow (tmp_win);
    tmp_win->isicon = FALSE;
    tmp_win->icon_on = FALSE;


    /* now de-iconify transients */
      for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
      {
	if (t->transient && t->transientfor == tmp_win->w)
	  {
	    if (t->icon_on)
	      Zoom(t->icon.w, t->frame);
	    else
	      Zoom(tmp_win->icon.w, t->frame);

	    t->mapped = TRUE;
	    if (!Scr->NoRaiseDeicon)
	      RaiseFrame(t);
	    MapFrame(t);
	    SetMapStateProp(t, NormalState);

	    if (t->icon.w) {
	      UnmapIcon(t);
	      IconDown (t);
	    }
	    if (t->list) XUnmapWindow(dpy, t->list->icon);
	    t->isicon = FALSE;
	    t->icon_on = FALSE;
	  }
	}

    XSync (dpy, 0);
}



Iconify(tmp_win, def_x, def_y)
TwmWindow *tmp_win;
int def_x, def_y;
{
    TwmWindow *t;
    int iconify;
    XWindowAttributes winattrs;
    unsigned long eventMask;

    iconify = ((!tmp_win->iconify_by_unmapping) || tmp_win->transient);
    if (iconify)
    {
	if (tmp_win->icon.w == None)
	    CreateIconWindow(tmp_win, def_x, def_y);
	else
	    IconUp(tmp_win);
	MapIcon(tmp_win);
    }
    if (tmp_win->list)
	XMapWindow(dpy, tmp_win->list->icon);

    XGetWindowAttributes(dpy, tmp_win->w, &winattrs);
    eventMask = winattrs.your_event_mask;

    /* iconify transients first */
	for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
	{
	    if (t->transient && t->transientfor == tmp_win->w)
	    {
		if (iconify)
		{
		    if (t->icon_on)
			Zoom(t->icon.w, tmp_win->icon.w);
		    else
			Zoom(t->frame, tmp_win->icon.w);
		}

		/*
		 * Prevent the receipt of an UnmapNotify, since that would
		 * cause a transition to the Withdrawn state.
		 */
		t->mapped = FALSE;
		XSelectInput(dpy, t->w, eventMask & ~StructureNotifyMask);
		UnmapFrame(t);
		XSelectInput(dpy, t->w, eventMask);
		if (t->icon.w)
		    UnmapIcon(t);
		SetMapStateProp(t, IconicState);
		SetBorder (t, False);
		if (t == Scr->Focus)
		{
		    SetFocus ((TwmWindow *) NULL, LastTimestamp());
		    Scr->Focus = NULL;
		    Scr->FocusRoot = TRUE;
		}
		if (t->list) XMapWindow(dpy, t->list->icon);
		t->isicon = TRUE;
		t->icon_on = FALSE;
	    }
	}

    if (iconify)
	Zoom(tmp_win->frame, tmp_win->icon.w);

    /*
     * Prevent the receipt of an UnmapNotify, since that would
     * cause a transition to the Withdrawn state.
     */
    tmp_win->mapped = FALSE;
    XSelectInput(dpy, tmp_win->w, eventMask & ~StructureNotifyMask);
    UnmapFrame(tmp_win);
    XSelectInput(dpy, tmp_win->w, eventMask);
    SetMapStateProp(tmp_win, IconicState);

    SetBorder (tmp_win, False);
    if (tmp_win == Scr->Focus)
    {
	SetFocus ((TwmWindow *) NULL, LastTimestamp());
	Scr->Focus = NULL;
	Scr->FocusRoot = TRUE;
    }
    tmp_win->isicon = TRUE;
    if (iconify)
	tmp_win->icon_on = TRUE;
    else
	tmp_win->icon_on = FALSE;
    XSync (dpy, 0);
}



static void Identify (t)
TwmWindow *t;
{
    int i, n, twidth, width, height;
    int x, y;
    unsigned int wwidth, wheight, bw, depth;
    Window junk;
    int px, py, dummy;
    unsigned udummy;
    char *s;

    n = 0;

    if (Scr->Identification) {
	(void) sprintf(Info[n++], "%s", Scr->Identification);
	Info[n++][0] = '\0';
    }

    (void) sprintf(Info[n++], "tvtwm version:  %s", Version);
#ifdef BETA
    (void) sprintf(Info[n++], "tvtwm Patchlevel %d%s", PATCHLEVEL, BETA);
#else
    (void) sprintf(Info[n++], "tvtwm Patchlevel %d", PATCHLEVEL);
#endif

    Info[n++][0] = '\0';

    if (t) {
	XGetGeometry (dpy, t->w, &JunkRoot, &JunkX, &JunkY,
		      &wwidth, &wheight, &bw, &depth);
	(void) XTranslateCoordinates (dpy, t->w, Scr->Root, 0, 0,
				      &x, &y, &junk);
	(void) sprintf(Info[n++], "Name             = \"%s\"", t->full_name);
	(void) sprintf(Info[n++], "Class.res_name   = \"%s\"", t->class.res_name);
	(void) sprintf(Info[n++], "Class.res_class  = \"%s\"", t->class.res_class);
	if ((s = GetPropertyString(t->w, XA_WM_CLIENT_MACHINE)) != NULL)
	    (void) sprintf(Info[n++], "Host             = \"%s\"", s);
	else
	    (void) sprintf(Info[n++], "Host             = <empty>");
	Info[n++][0] = '\0';
	(void) sprintf(Info[n++], "Geometry/root    = %dx%d+%d+%d", wwidth, wheight,
		x, y);
	(void) sprintf(Info[n++], "Border width     = %d", bw);
	(void) sprintf(Info[n++], "Depth            = %d", depth);
    }

    Info[n++][0] = '\0';
    (void) sprintf(Info[n++], "Click to dismiss...");

    /* figure out the width and height of the info window */
    height = n * (Scr->DefaultFont.height+2);
    width = 1;
    for (i = 0; i < n; i++)
    {
	twidth = XTextWidth(Scr->DefaultFont.font, Info[i],
	    strlen(Info[i]));
	if (twidth > width)
	    width = twidth;
    }
    if (InfoLines) XUnmapWindow(dpy, Scr->InfoWindow);

    width += 10;		/* some padding */
    if (XQueryPointer (dpy, Scr->Root, &JunkRoot, &JunkChild, &px, &py,
		       &dummy, &dummy, &udummy)) {
	px -= (width / 2);
	py -= (height / 3);
	if (px + width + BW2 >= Scr->MyDisplayWidth) 
	  px = Scr->MyDisplayWidth - width - BW2;
	if (py + height + BW2 >= Scr->MyDisplayHeight) 
	  py = Scr->MyDisplayHeight - height - BW2;
	if (px < 0) px = 0;
	if (py < 0) py = 0;
    } else {
	px = py = 0;
    }
    XMoveResizeWindow(dpy, Scr->InfoWindow, px, py, width, height);
    XMapRaised(dpy, Scr->InfoWindow);
#ifdef XPM
    if (!t) {
# include "emblem.h"
/* XXX - need to support the None color, at some point here. */
/* currently, the automatic masking stuff in XPM doesn't work */
/* for this image.        - cross  */
	Pixmap pixmap;
	int XPMret;
	XpmColorSymbol bgcolor;
	XpmAttributes attribs;

	bgcolor.name = "background";
	bgcolor.value = NULL;
	bgcolor.pixel = Scr->DefaultC.back;

	attribs.valuemask = XpmColorSymbols | XpmVisual |
				XpmColormap | XpmDepth;
	attribs.colorsymbols = &bgcolor;
	attribs.numsymbols = 1;
	attribs.visual = Scr->d_visual;
	attribs.colormap = DefaultColormap(dpy, Scr->screen);
	attribs.depth = Scr->d_depth;

	if ((XPMret = XpmCreatePixmapFromData(dpy, RootWindow(dpy, Scr->screen),
			emblem, &pixmap, NULL, &attribs)) >= 0) {

		XCopyArea(dpy, pixmap, Scr->InfoWindow,
			DefaultGC(dpy, Scr->screen), 0, 0,
			attribs.width, attribs.height,
			(width-attribs.width), (height-attribs.height));
			XFlush(dpy);
	} else fprintf(stderr, "tvtwm: Error creating info-box pixmap (Error code: %d)\n", XPMret); 
    }
#endif
    InfoLines = n;
}



SetMapStateProp(tmp_win, state)
TwmWindow *tmp_win;
int state;
{
    unsigned long data[2];		/* "suggested" by ICCCM version 1 */
  
    data[0] = (unsigned long) state;
    data[1] = (unsigned long) (tmp_win->iconify_by_unmapping ? None : 
			   tmp_win->icon.w);

    XChangeProperty (dpy, tmp_win->w, _XA_WM_STATE, _XA_WM_STATE, 32, 
		 PropModeReplace, (unsigned char *) data, 2);
}



Bool GetWMState (w, statep, iwp)
    Window w;
    int *statep;
    Window *iwp;
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    unsigned long *datap = NULL;
    Bool retval = False;

    if (XGetWindowProperty (dpy, w, _XA_WM_STATE, 0L, 2L, False, _XA_WM_STATE,
			    &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &datap) != Success || !datap)
      return False;

    if (nitems <= 2) {			/* "suggested" by ICCCM version 1 */
	*statep = (int) datap[0];
	*iwp = (Window) datap[1];
	retval = True;
    }

    XFree ((char *) datap);
    return retval;
}



void
WarpToScreen (n, inc)
    int n, inc;
{
    Window dumwin;
    int x, y, dumint;
    unsigned int dummask;
    ScreenInfo *newscr = NULL;

    while (!newscr) {
					/* wrap around */
	if (n < 0) 
	  n = NumScreens - 1;
	else if (n >= NumScreens)
	  n = 0;

	newscr = ScreenList[n];
	if (!newscr) {			/* make sure screen is managed */
	    if (inc) {			/* walk around the list */
		n += inc;
		continue;
	    }
	    fprintf (stderr, "%s:  unable to warp to unmanaged screen %d\n", 
		     ProgramName, n);
	    XBell (dpy, 0);
	    return;
	}
    }

    if (Scr->screen == n) return;	/* already on that screen */

    PreviousScreen = Scr->screen;
    XQueryPointer (dpy, Scr->Root, &dumwin, &dumwin, &x, &y,
		   &dumint, &dumint, &dummask);

    XWarpPointer (dpy, None, newscr->Root, 0, 0, 0, 0, x, y);
}



/*
 * BumpWindowColormap - rotate our internal copy of WM_COLORMAP_WINDOWS
 */
void
BumpWindowColormap (tmp, inc)
    TwmWindow *tmp;
    int inc;
{
    int i, j, previously_installed;
    ColormapWindow **cwins;

    if (!tmp) return;

    if (inc && tmp->cmaps.number_cwins > 0) {
	cwins = (ColormapWindow **) malloc(sizeof(ColormapWindow *)*
					   tmp->cmaps.number_cwins);
	if (cwins) {
	    if (previously_installed =
		/* SUPPRESS 560 */(Scr->cmapInfo.cmaps == &tmp->cmaps &&
		tmp->cmaps.number_cwins)) {
		for (i = tmp->cmaps.number_cwins; i-- > 0; )
		    tmp->cmaps.cwins[i]->colormap->state = 0;
	    }

	    for (i = 0; i < tmp->cmaps.number_cwins; i++) {
		j = i - inc;
		if (j >= tmp->cmaps.number_cwins)
		    j -= tmp->cmaps.number_cwins;
		else if (j < 0)
		    j += tmp->cmaps.number_cwins;
		cwins[j] = tmp->cmaps.cwins[i];
	    }

	    free((char *) tmp->cmaps.cwins);

	    tmp->cmaps.cwins = cwins;

	    if (tmp->cmaps.number_cwins > 1)
		bzero (tmp->cmaps.scoreboard, 
		       ColormapsScoreboardLength(&tmp->cmaps));

	    if (previously_installed)
		InstallWindowColormaps(PropertyNotify, (TwmWindow *) NULL);
	}
    } else
	FetchWmColormapWindows (tmp);
}



HideIconManager ()
{
    SetMapStateProp (Scr->iconmgr.twm_win, WithdrawnState);
    UnmapFrame(Scr->iconmgr.twm_win);
    if (Scr->iconmgr.twm_win->icon.w)
      UnmapIcon(Scr->iconmgr.twm_win);
    Scr->iconmgr.twm_win->mapped = FALSE;
    Scr->iconmgr.twm_win->isicon = TRUE;
}



SetBorder (tmp, onoroff)
    TwmWindow *tmp;
    Bool onoroff;
{
    if (tmp->highlight) {
	if (onoroff) {
	    XSetWindowBorder (dpy, tmp->frame, tmp->border);
	    if (tmp->title_w) 
	      XSetWindowBorder (dpy, tmp->title_w, tmp->border);
	} else {
	    XSetWindowBorderPixmap (dpy, tmp->frame, tmp->gray);
	    if (tmp->title_w) 
	      XSetWindowBorderPixmap (dpy, tmp->title_w, tmp->gray);
	}
    }
}



DestroyMenu (menu)
    MenuRoot *menu;
{
    MenuItem *item;

    if (menu->w) {
	XDeleteContext (dpy, menu->w, MenuContext);
	XDeleteContext (dpy, menu->w, ScreenContext);
	if (Scr->Shadow) XDestroyWindow (dpy, menu->shadow);
	XDestroyWindow(dpy, menu->w);
    }

    for (item = menu->first; item; ) {
	MenuItem *tmp = item;
	item = item->next;
	free ((char *) tmp);
    }
}



/*
 * warping routines
 */
void WarpAlongRing (ev, forward)
    XButtonEvent *ev;
    Bool forward;
{
    TwmWindow *r, *head;

    if (Scr->RingLeader)
      head = Scr->RingLeader;
    else if (!(head = Scr->Ring)) 
      return;

    if (forward) {
	for (r = head->ring.next; r != head; r = r->ring.next) {
	    if (!r || r->mapped) break;
	}
    } else {
	for (r = head->ring.prev; r != head; r = r->ring.prev) {
	    if (!r || r->mapped) break;
	}
    }

    if (r && r != head) {
	TwmWindow *p = Scr->RingLeader, *t;

	Scr->RingLeader = r;
	WarpToWindow (r);

	if (p && p->mapped &&
	    XFindContext (dpy, ev->window, TwmContext, (caddr_t *)&t) == XCSUCCESS &&
	    p == t) {
	    p->ring.cursor_valid = True;
	    p->ring.curs_x = ev->x_root - t->frame_x;
	    p->ring.curs_y = ev->y_root - t->frame_y;
	    if (p->ring.curs_x < -p->frame_bw || 
		p->ring.curs_x >= p->frame_width + p->frame_bw ||
		p->ring.curs_y < -p->frame_bw || 
		p->ring.curs_y >= p->frame_height + p->frame_bw) {
		/* somehow out of window */
		p->ring.curs_x = p->frame_width / 2;
		p->ring.curs_y = p->frame_height / 2;
	    }
	}
    }
}



void WarpToWindow (t)
    TwmWindow *t;
{
    int x, y;

    /* if we have the virtual desktop, attempt to make the window
     * somewhat visible
     */
    if (Scr->VirtualDesktop)
	ScrollToQuadrant(t);

    if (t->auto_raise || !Scr->NoRaiseWarp) AutoRaiseWindow (t);
    if (t->ring.cursor_valid) {
	x = t->ring.curs_x;
	y = t->ring.curs_y;
    } else {
	x = t->frame_width / 2;
	y = t->frame_height / 2;
    }
    XWarpPointer (dpy, None, t->frame, 0, 0, 0, 0, x, y);
}



/*
 * ICCCM Client Messages - Section 4.2.8 of the ICCCM dictates that all
 * client messages will have the following form:
 *
 *     event type	ClientMessage
 *     message type	_XA_WM_PROTOCOLS
 *     window		tmp->w
 *     format		32
 *     data[0]		message atom
 *     data[1]		time stamp
 */
static void send_clientmessage (w, a, timestamp)
    Window w;
    Atom a;
    Time timestamp;
{
    XClientMessageEvent ev;

    ev.type = ClientMessage;
    ev.window = w;
    ev.message_type = _XA_WM_PROTOCOLS;
    ev.format = 32;
    ev.data.l[0] = a;
    ev.data.l[1] = timestamp;
    XSendEvent (dpy, w, False, 0L, (XEvent *) &ev);
}

SendDeleteWindowMessage (tmp, timestamp)
    TwmWindow *tmp;
    Time timestamp;
{
    send_clientmessage (tmp->w, _XA_WM_DELETE_WINDOW, timestamp);
}

SendSaveYourselfMessage (tmp, timestamp)
    TwmWindow *tmp;
    Time timestamp;
{
    send_clientmessage (tmp->w, _XA_WM_SAVE_YOURSELF, timestamp);
}


SendTakeFocusMessage (tmp, timestamp)
    TwmWindow *tmp;
    Time timestamp;
{
    send_clientmessage (tmp->w, _XA_WM_TAKE_FOCUS, timestamp);
}




/* ------------------------------------------------------------------------ */

/*
 * Pie Menus.
 * By Don Hopkins.
 *
 */

/* ------------------------------------------------------------------------ */

/*
 * This pie menu tracking code determines the slice the cursor 
 * is in by representing slice edge angles as (quadrant, slope) 
 * pairs that can be quickly computed and compared. 
 *
 * The slope is defined such that it is greater than or equal to zero,
 * less than infinity, and increasing counter-clockwise around the menu. 
 * Each of the four quadrants encompasses one range of slope.
 *
 *                 Y
 *               ^
 *               |     x>0, y>=0
 *  x<=0, y>0 <--+       y/x
 *    -x/y       |        ^
 *        quad 1 | quad 0 |     X
 * -----+--------+--------+----> 
 *      | quad 2 | quad 3
 *      V        |      -x/y
 *   x<0, y<=0   +--> x>=0, y<0
 *     y/x       |
 *               |
 * 
 * The quadrants and slopes of the item edges are all precalculated,
 * during menu layout.
 * The quadrant and slope of the cursor must be calculated frequently
 * during menu tracking, so we just calculate the numerator and
 * denominator of the slope, and avoid an unnecessary division.
 * Instead of calculating "slope = numerator / denominator" then
 * testing "slope < it->slope", every time the cursor moves, we can
 * just test "numerator < (denominator * it->slope)".
 *
 * This algorithm works in a right-side-up coordinate space, but the final
 * results are tranformed into X-windows's up-side-down coordinate system 
 * by subtracting the y values from the window height. 
 */

/* ------------------------------------------------------------------------ */

#define CALC_QUADRANT_SLOPE(x, y, quadrant, numerator, denominator) \
    if ((y) > 0) (quadrant) = ((x) > 0 ? 0 : 1); \
    else if ((y) < 0) (quadrant) = ((x) < 0 ? 2 : 3); \
    else (quadrant) = ((x) > 0 ? 0 : 2); \
    if ((quadrant) & 1) { \
	(numerator) = ABS((x)); (denominator) = ABS((y)); \
    } else { \
	(numerator) = ABS((y)); (denominator) = ABS((x)); \
    }

/* ------------------------------------------------------------------------ */

static MenuItem *
CalcPieMenuItem(MenuRoot *menu, int x, int y)
{
    register MenuItem *it, *last_it;
    int i, j, order, quadrant;
    int numerator, denominator;
    int first, last_i, last_order;

    /*
     * Translate x and y so they are relative to the menu center,
     * in right side up coordinates.
     */

    x = (x - menu->center_x) + 1;
/*    y = ((menu->height - y) - menu->center_y) - 1; */
    y = (menu->center_y - y) - 1;

    /*
     * If there are no menu items,
     * or we are within the inactive region in the menu center,
     * then there is no item selected.
     */
    if ((menu->first == NULL) ||
	((x * x) + (y * y) <
	 (PIE_INACTIVE_RADIUS * PIE_INACTIVE_RADIUS))) {
        menu->current = -1;
        return(NULL);
    }

    /*
     * If there's only one item, then that must be it. 
     */
    if (menu->items == 1) {
        menu->current = 0;
        return(menu->first);
    }

    /*
     * Calculate the quadrant, slope numerator, and slope denominator of
     * the cursor slope, to be used for comparisons.
     */
    CALC_QUADRANT_SLOPE(x, y, quadrant, numerator, denominator);

    /*
     * In most cases, during cursor tracking, the menu item that the
     * cursor is over will be the same as it was before (almost all
     * of the time), or one of the neighboring items (most of the
     * rest of the time). So we check those items first. But to keep
     * things simple, instead of actually checking the items in order of
     * frequency (the current, the two neighbors, then the rest), we just
     * start our loop around the menu items at the item *before* the
     * last selected menu item, so we still check the three most common
     * cases first (neighbor, current, neighbor, rest), without having 
     * to complicate the code with special cases. Another strategy, that
     * might be good for menus with ridiculously many items, would be
     * [to check the current item first, then the two neighbors, then]
     * to do a binary search of the menu items (since they are ordered).
     * But that's more complicated and you shouldn't have that many menu
     * items anyway.
     */

    /*
     * Start at the item before current one.
     */
    first = menu->current - 1;
    if (first < 0)
        first = menu->items - 1;

    /*
     * Initialize last_order such that we will go through the loop
     * at least once, validating last_i, last_order, and last_it for
     * the next time through the loop.
     */
    last_i = last_order = -1;
    i = first;
    
    for (it = menu->first, j = 0; j < i; j++)
      it = it->next;

    while (1) {

/* Legend: c = cursor, e = edge
   <cursor quad>,<edge quad>
         quad 1 | quad 0
         -------+-------
         quad 2 | quad 3
*/

        /* Set order = 1, if shortest direction from edge to cursor is ccw */
        switch ((quadrant - it->quadrant) & 3) {

case 0: /*
		 0,0	 1,1	 2,2	 3,3
		  |ce	ce|	  |	  |	
		--+--	--+--	--+--	--+--	
		  |	  |	ce|	  |ce
	*/
	    /* slope >= it->slope */
	    order = ((float)numerator >= (float)(denominator * it->slope));
	    break;

case 1: /*
		 1,0	 2,1	 3,2	 0,3
		 c|e	 e|	  |	  |c
		--+--	--+--	--+--	--+--	
		  |	 c|	 e|c	  |e
	*/
	    order = 1;
	    break;

case 2: /*
		 2,0	 3,1	 0,2	 1,3
		  |e	 e|	  |c	 c|
		--+--	--+--	--+--	--+--	
		 c|	  |c	 e|	  |e
	*/
	    /* slope < it->slope */
	    order = ((float)numerator < (float)(denominator * it->slope));
	    break;

case 3: /*
		 3,0	 0,1	 1,2	 2,3
		  |e	 e|c	 c|	  |
		--+--	--+--	--+--	--+--	
		  |c	  |	 e|	 c|e
	*/
	    order = 0;
	    break;
	}

	/*
	 * If we were counter-clockwise of the last leading edge,
	 * and we're clockwise of this leading edge,
	 * then we were in the last menu item.
	 * (Note: first time through this loop last_order = -1 so we'll
	 * go back through the loop at least once, after validating
	 * last_order, last_i, and last_it.)
	 */
	if ((last_order == 1) && (order == 0)) {
	    menu->current = last_i;
	    return(last_it);
	}
	last_order = order;

	/*
	 * Remember this menu item index, and move on to the next one
	 * counter-clockwise around the circle.
	 */
	last_i = i; last_it = it;
	if ((++i >= menu->items) || ((it = it->next) == NULL)) {
	    i = 0; it = menu->first;
	}

	/* 
         * If we've checked all the others, then that must have been it. 
	 * This saves us from checking the leading edge of the first
	 * item again (It's also insurance against layout bugs.)
	 */
	if (i == first) {
	    menu->current = last_i;
	    return(last_it);
	}
    }
}

static void
LayoutPieMenu(MenuRoot *menu)
{
  int i;
  int total_slice, radius;
  int minx, miny, maxx, maxy;
  int title_width;
  float angle;
  MenuItem *it, *last;
  MyFont *font, *titlefont;

  /*
   * Calculate the sum of the menu item slice sizes.
   * Each menu item will get a (slice / total_slice) sized slice of the pie.
   */
  #define EACH_SLICE 100
  total_slice = 0;
  for (it = menu->first; it != NULL; it = it->next) {
    total_slice += EACH_SLICE;
  }

  font = &(Scr->MenuFont);
  if (Scr->MenuTitleFont.name != NULL)
    titlefont = &(Scr->MenuTitleFont);
  else
    titlefont = font;

  /*
   * Calculate the subtend, angle, cosine, sine, quadrant, slope,
   * and size of each menu item.
   */
  angle = menu->initial_angle;
  for (it = menu->first; it != NULL; it = it->next) {
    register float edge_dx, edge_dy, numerator, denominator;
    register int quadrant;

    it->subtend = TWO_PI * EACH_SLICE / total_slice;
    if (it != menu->first) angle += it->subtend / 2.0;
    it->angle = angle;
    it->dx = cos(angle);
    it->dy = sin(angle);
    edge_dx = cos(angle - it->subtend / 2.0);
    edge_dy = sin(angle - it->subtend / 2.0);
    CALC_QUADRANT_SLOPE(edge_dx, edge_dy, quadrant, numerator, denominator);
    it->quadrant = quadrant;
    it->slope = (float)numerator / (float)denominator;
    angle += it->subtend / 2.0;

    if (it->item[0] == '_') {
      static Bool isXpm[1] = { False };

      it->icon = (Pixmap)LookInNameList(Scr->Icons, &it->item[1]);
      if (it->icon == None) {
	it->icon = FindPixmap (&it->item[1], &JunkWidth, &JunkHeight,
			       isXpm, NULL, &it->mask);
      }
      if (it->icon != None) {
	unsigned int bw, depth, w, h;

	AddToList(&Scr->Icons, &it->item[1], (char *)it->icon);
	if (*isXpm == True)
	  AddToList(&Scr->IconsPixmapType, &it->item[1], "p");
	else
	  AddToList(&Scr->IconsPixmapType, &it->item[1], "b");

	XGetGeometry (dpy, it->icon, &JunkRoot, &JunkX, &JunkY,
		      &w, &h, &bw, &depth);
	it->width = w; it->height = h;
      }
    }

    if (!it->icon) {
      if ((it->func == F_PIEMENU) || (it->func == F_MENU)) {
	it->width = XTextWidth(titlefont->font, it->item, it->strlen);
	it->height = titlefont->height;
      } else {
	it->width = XTextWidth(font->font, it->item, it->strlen);
	it->height = font->height;
      }
    }
  }

  radius = PIE_LABEL_RADIUS_MIN;
  last = menu->last;
  for (i = 0, it = menu->first; i <= menu->items && menu->items > 1;
       i++, it = (it->next != NULL) ? it->next : menu->first) {
    float dx, dy, ldx, ldy;
    int width, height, lwidth, lheight;

    dx = it->dx;  dy = it->dy;
    width = it->width;  height = it->height;
    ldx = last->dx;  ldy = last->dy;
    lwidth = last->width;  lheight = last->height;
    while (1) {
      register int x, y, lx, ly, 
      		   x0max, y0max, x1min, y1min;

      x = dx * radius;  y = dy * radius;
      lx = ldx * radius; ly = ldy * radius;

      /* Translate x y with respect to label size and position */
      if (ABS(x) <= 2) {
	x -= width/2;
	if (y < 0)
	  y -= height;
      } else {
	if (x < 0)
	  x -= width;
	y -= height/2;
      }

      if (ABS(lx) <= 2) {
	lx -= lwidth/2;
	if (ly < 0)
	  ly -= lheight;
      } else {
	if (lx < 0)
	  lx -= lwidth;
	ly -= lheight/2;
      }

      /* Do rects (x y width height) and (lx ly lwidth lheight) overlap? */
      x0max = x > lx ? x : lx;
      y0max = y > ly ? y : ly;
      x1min = x+width < lx+lwidth ? x+width : lx+lwidth;
      y1min = y+height < ly+lheight ? y+height : ly+lheight;
      if (!(x0max<x1min && y0max<y1min)) { /* If they don't overlap */
	/* They are far enough out so they don't overlap, move on. */
	break;
      }
      /* Push the menu radius out a step and try again */
      radius += PIE_LABEL_RADIUS_STEP;
    }
    /* Loop on to next menu item */
    last = it;
  }

  radius += PIE_LABEL_RADIUS_EXTRA;
  menu->label_radius = radius;

  /* Finally position all the menu labels at the same radius.
     Figure out the bounding box of the labels. */
  minx = miny = maxx = maxy = 0;
  for (it = menu->first; it != NULL; it = it->next) {
    it->x = radius * it->dx;
    it->y = radius * it->dy;

    /* Translate x y with respect to label size and position */
    if (ABS(it->x) <= 2) {
      it->x -= it->width/2;
      if (it->y < 0)
	it->y -= it->height;
    } else {
      if (it->x < 0)
	it->x -= it->width;
      it->y -= it->height/2;
    }

    if (it->x < minx) minx = it->x;
    if ((it->x + it->width) > maxx) maxx = (it->x + it->width);
    if (it->y < miny) miny = it->y;
    if ((it->y + it->height) > maxy) maxy = (it->y + it->height);
  }

  title_width = XTextWidth(titlefont->font, menu->name, strlen(menu->name));
  menu->title_height = titlefont->height;
  if (-(title_width / 2) < minx)
    minx = -(title_width / 2);
  if ((title_width / 2) > maxx)
    maxx = (title_width / 2);

  maxy += (2 * PIE_BORDER) + menu->title_height;

  minx -= PIE_BORDER;  miny -= PIE_BORDER;
  maxx += PIE_BORDER;  maxy += PIE_BORDER;

  menu->center_x = -minx;
  menu->center_y = maxy; /* y flip */
  menu->width = maxx - minx;
  menu->height = maxy - miny;

  menu->title_y = PIE_BORDER + titlefont->y;
  menu->title_x = menu->center_x - (title_width / 2);

  /* Translate the menu items to the center of the menu, in X coordinates. */
  for (it = menu->first; it != NULL; it = it->next) {
    it->x = menu->center_x + it->x;
    it->y = (menu->center_y - it->y) - it->height; /* y flip */
  }

  if (menu->segments != NULL) {
      free(menu->segments);
  }
  menu->segments = (XSegment *)
    malloc(menu->items * sizeof(XSegment));

  if (menu->items > 1) {
    XSegment *seg = menu->segments;

    angle = menu->initial_angle - (menu->first->subtend / 2.0);
    for (it = menu->first; it != NULL; it = it->next) {
	seg->x1 = menu->center_x + (cos(angle) * PIE_INACTIVE_RADIUS);
	seg->y1 = menu->center_y - (sin(angle) * PIE_INACTIVE_RADIUS);
	seg->x2 = menu->center_x +
	  (cos(angle) * (menu->label_radius - PIE_LABEL_RADIUS_EXTRA));
	seg->y2 = menu->center_y -
	  (sin(angle) * (menu->label_radius - PIE_LABEL_RADIUS_EXTRA));
	seg++;
	angle += it->subtend;
    }
  }
}

static void
PaintPieMenu(MenuRoot *mr)
{
  MenuItem *mi;
  MyFont *font;

  FB(Scr->MenuC.fore, Scr->MenuC.back);
  XFillRectangle(dpy, mr->w, Scr->NormalGC,
		 0, 0, mr->width, mr->title_height + (2 * PIE_BORDER));

  if (Scr->MenuTitleFont.name != NULL) {
      font = &(Scr->MenuTitleFont);
  } else {
      font= &(Scr->MenuFont);
  }

  FBF(Scr->MenuC.back, Scr->MenuC.fore, font->font->fid);
  XDrawString(dpy, mr->w, Scr->NormalGC, mr->title_x, mr->title_y,
	      mr->name, strlen(mr->name));

  if (mr->segments) { 
    XSetLineAttributes(dpy, Scr->NormalGC,
		       0, LineSolid, CapButt, JoinMiter);
    FB(Scr->MenuC.fore, Scr->MenuC.back);
    XDrawSegments(dpy, mr->w, Scr->NormalGC, mr->segments, mr->items);
  }

  for (mi = mr->first; mi != NULL; mi = mi->next) {
    PaintEntry(mr, mi, True);
  }   
}

static void
PaintPieMenuEntry(MenuRoot *mr, MenuItem *mi, int exposure)
{
    int x, y, width, height, text_y;
    MyFont *font;

    if (((mi->func == F_PIEMENU) ||
	 (mi->func == F_MENU)) &&
	(Scr->MenuTitleFont.name != NULL)) {
	font = &(Scr->MenuTitleFont);
    } else {
	font = &(Scr->MenuFont);
    }

    x = mi->x; y = mi->y;
    width = mi->width; height = mi->height;
    text_y = y + font->y;

    if (mi->state) {
	XSetForeground(dpy, Scr->NormalGC, mi->hi_back);

	XFillRectangle(dpy, mr->w, Scr->NormalGC, x-1, y-1,
		       width+2, height+2);

	XSetForeground(dpy, Scr->NormalGC, mi->back);

	FBF(mi->hi_fore, mi->hi_back, font->font->fid);

    } else {
	if (mi->user_colors || !exposure) {
	    XSetForeground(dpy, Scr->NormalGC, mi->back);

	    XFillRectangle(dpy, mr->w, Scr->NormalGC, x-1, y-1,
		width+2, height+2);

	    XSetForeground(dpy, Scr->NormalGC, mi->hi_back);
	}
	FBF(mi->fore, mi->back, font->font->fid);
    }

    if (mi->icon != None) {
      XSetClipMask(dpy, Scr->NormalGC, mi->mask);
      XSetClipOrigin(dpy, Scr->NormalGC, x, y);
      XCopyArea(dpy, mi->icon, mr->w, Scr->NormalGC,
		0, 0, width, height, x, y);
      XSetClipMask(dpy, Scr->NormalGC, None);
      XSetClipOrigin(dpy, Scr->NormalGC, 0, 0);
    } else {
      XDrawString(dpy, mr->w, Scr->NormalGC, x, text_y,
		  mi->item, mi->strlen);
    }
}

extern int HasShape;

static void
ShapePieMenu(MenuRoot *mr)
{
  Window shape;
  GC gc;
  XGCValues values;
  MenuItem *mi;

  if (HasShape == 0)
    return;

  shape = XCreatePixmap(dpy, DefaultRootWindow(dpy),
			mr->width, mr->height, 1);
  gc = XCreateGC(dpy, shape, 0, &values);

  XSetForeground(dpy, gc, 0);
  XFillRectangle(dpy, shape, gc,
		 0, 0,
		 mr->width, mr->height);

  XSetForeground(dpy, gc, 1);
  if (*mr->name != 0) {
    int bw = 1 /* borderWidth */;

    XFillRectangle(dpy, shape, gc,
		   bw, bw,
		   mr->width - bw*2, mr->title_height + bw*2);
  }

  for (mi = mr->first; mi != NULL; mi = mi->next) {
    XFillRectangle(dpy, shape, gc,
		   mi->x-1, mi->y-1,
		   mi->width+2, mi->height+2);
  }

  XFreeGC(dpy, gc);
  XShapeCombineMask(dpy, mr->w, ShapeBounding, 0, 0, shape, ShapeSet);
}
