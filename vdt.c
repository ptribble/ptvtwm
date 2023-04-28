/*****************************************************************************/
/**               Copyright 1990 by Solbourne Computer Inc.                 **/
/**                          Longmont, Colorado                             **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name of Solbourne not be used in advertising                         **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    SOLBOURNE COMPUTER INC. DISCLAIMS ALL WARRANTIES WITH REGARD         **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL SOLBOURNE                **/
/**    BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-           **/
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

/**********************************************************************
 *
 * $XConsortium: vdt.c,v 1.140 90/03/23 11:42:33 jim Exp $
 *
 * $Log: vdt.c,v $
 * Revision 1.13  1994/05/04  00:03:02  cross
 * bug fix (from Tom Ekberg <ekberg@asl.dl.nec.com>)
 *
 * Revision 1.12  1993/05/20  21:53:52  cross
 * Alpha stuff, prototypes weren't happy, and vdt.c needed DisplayString.
 *
 * Revision 1.11  1993/05/20  15:40:06  cross
 * pl9b5.  Mostly double-headed machine stuff
 * XPM fixes to such.
 *
 * Revision 1.10  1993/02/26  15:47:14  cross
 * Fixed some thinko's.  These were reported by someone, who's
 * name I lost. (*Sigh*)
 *
 * Revision 1.9  1993/01/21  01:34:30  cross
 * Fixed the SIGCHLD handler.  Was hanging because it accidentally
 * wait()ed on something inside of the Xpm library...  (popen(zcat))
 *
 * Revision 1.8  1993/01/20  23:14:34  cross
 * RJC changes
 *
 * Revision 1.7  1992/11/04  01:17:22  cross
 * Added code from rjc, with filtering.
 * See text in CHANGES for details.
 *
 * Revision 1.6  1992/05/02  19:46:35  cross
 * Added changes by RJC.  A few bug fixes, reorganization of lots of
 * code, and many new features
 *
 * Revision 1.5  1992/02/21  21:29:12  cross
 * Free'd the alloc'd memory for size/wm/class hints
 *
 * Revision 1.4  1992/01/22  22:59:49  cross
 * Added the WrapVirtual functionality
 *
 * Revision 1.3  1992/01/08  18:55:49  cross
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
 * Revision 1.2  1991/09/25  23:04:32  cross
 * Modified all calls to FindBitmap to make them FindPixmap
 * Also, took out the code that makes the returned bitmap into
 * a pixmap, cause XcprFilePixmapFile now returns a pixmap all
 * the time...
 *
 * Revision 1.1  1991/09/25  22:54:13  cross
 * Initial revision
 *
 * Revision 10.0  91/06/12  09:05:53  toml
 * Revision bump
 * 
 * Revision 9.1  91/06/12  08:39:24  toml
 * Added patch to do gridded movement in panner
 * 
 * Revision 9.0  91/04/23  07:40:54  toml
 * Revision bump
 * 
 * Revision 8.5  91/04/18  08:19:20  toml
 * Disable backing store on the virtual desktop
 * 
 * Revision 8.4  91/04/15  14:54:18  toml
 * Fixes panner interaction under OpenWindows
 * 
 * Revision 8.3  90/12/29  15:26:03  toml
 * Made PannerOpaqueScroll only take effect if StickyAbove is also set.
 * 
 * Revision 8.2  90/12/29  11:24:22  toml
 * Added PannerOpaqueScroll
 * 
 * Revision 8.1  90/12/29  09:56:37  toml
 * Disallow unmapnotify is window is panner
 * 
 * Revision 8.0  90/11/15  20:02:56  toml
 * Revision bump
 * 
 * Revision 7.4  90/11/12  21:34:52  toml
 * Implemented Scr->StickyAbove
 * 
 * Revision 7.3  90/11/12  20:39:37  toml
 * Fixes to stickyroot patches
 * 
 * Revision 7.2  90/11/12  19:57:21  toml
 * Patches to allow sticky windows to lower
 * 
 * Revision 1.3  90/11/04  19:23:50  brossard
 * When mapping a window and warping the cursor to it, only scroll
 * the desktop if the window is not visible on screen.  (This was
 * actually done in the previous revistion, this one just fixes
 * the maths to do it right).
 * 
 * Revision 1.2  90/11/04  18:38:36  brossard
 * Sticky windows are now child of the virtual root.
 * This has the advantage that they can now be raised and lowered like
 * any other window.  They no longuer are above everything else.
 * It has the disadvantage that when you move the desktop, the
 * sticky windows have to be moved back after scrolling the desktop.
 * 
 *
 * Virtual Desktop routines
 *
 * 22-Aug-90 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#if !defined(lint) && !defined(SABER)
static char RCSinfo[]=
"$XConsortium: vdt.c,v 1.140 90/03/23 11:42:33 jim Exp $";
#endif

#include <stdio.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include "twm.h"
#include "screen.h"
#include "vdt.h"
#include "parse.h"
#include "move.h"
#include "util.h"
#include "resize.h"

#define MAX_VDT_WIDTH	32000
#define MAX_VDT_HEIGHT	32000

static int pointerX;
static int pointerY;

#ifdef XLOADIMAGE
int xloadim_pid;
#endif

/* private swm atoms */
/* it would be nice at some point to get these atoms blessed so that
 * we don't have to make them swm specific
 */

/* version 1.2 of swm sends synthetic ConfigureNotify events
 * with respect to the actual root window rather than a 
 * clients logical root window.  OI clients lokk for this 
 * version string on the root window to determine how to
 * interpret the ConfigureNotify event
 */
#define SWM_VERSION "1.2"

Atom XA_SWM_ROOT = None;
Atom XA_SWM_VROOT = None;
Atom XA_SWM_VERSION = None;

/***********************************************************************
 *
 *  Procedure:
 *	InitVirtualDesktop - init vdt stuff
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
InitVirtualDesktop()
{
    XA_SWM_ROOT          = XInternAtom(dpy, "__SWM_ROOT", False);
    XA_SWM_VROOT         = XInternAtom(dpy, "__SWM_VROOT", False);
    XA_SWM_VERSION       = XInternAtom(dpy, "__SWM_VERSION", False);
}

/***********************************************************************
 *
 *  Procedure:
 *	SetSWM_ROOT - set the XA_SWM_ROOT property 
 *		This property always indicates to the application
 *		what its "root" window is.  This is currently needed
 *		by OI based clients, other toolkits don't know about
 *		virtual desktops.
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
SetSWM_ROOT(tmp_win)
TwmWindow *tmp_win;
{
    if (Scr->VirtualDesktop && !tmp_win->iconmgr)
    {
	XChangeProperty(dpy, tmp_win->w, XA_SWM_ROOT, XA_WINDOW,
	    32, PropModeReplace, (unsigned char *)&tmp_win->root, 1);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	SetSWM_VERSION - set the XA_SWM_VERSION property 
 *		This property always indicates to the application
 *		what its "root" window is.  This is needed by OI
 *		clients so they can know how to interpret the
 *		incoming synthetic ConfigureNotify events.
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
SetSWM_VERSION()
{
    static char *version = SWM_VERSION;
    XChangeProperty(dpy,Scr->Root,XA_SWM_VERSION, XA_STRING, 8,
	PropModeReplace, (unsigned char *)version, strlen(version));
}

/***********************************************************************
 *
 *  Procedure:
 *	RemoveSWM_VERSION - remove the XA_SWM_VERSION property 
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
RemoveSWM_VERSION()
{
    XDeleteProperty(dpy, Scr->Root, XA_SWM_VERSION);
}

/***********************************************************************
 *
 *  Procedure:
 *	MakeVirtual - make a small virtual window
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

Window
MakeVirtual(tmp_win, x, y, width, height, background, border)
TwmWindow *tmp_win;
int x;
int y;
int width;
int height;
long background;
long border;
{
    Window virtual;

    width /= Scr->PannerScale;
    height /= Scr->PannerScale;
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    if (width > MAX_VDT_WIDTH) width = MAX_VDT_WIDTH;
    if (height > MAX_VDT_HEIGHT) height = MAX_VDT_HEIGHT;
    virtual = XCreateSimpleWindow(dpy, Scr->Panner, x, y,
	width, height, 1, border, background);
    XGrabButton(dpy, Button2, 0L, virtual,
	True, ButtonPressMask | ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync, Scr->Panner, None);
    XSelectInput(dpy, virtual, KeyPressMask | ExposureMask );
    XSaveContext(dpy, virtual, TwmContext, (caddr_t) tmp_win);
    XSaveContext(dpy, virtual, VirtualContext, (caddr_t) tmp_win);
    XSaveContext(dpy, virtual, ScreenContext, (caddr_t) Scr);
    return (virtual);
}

/***********************************************************************
 *
 *  Procedure:
 *	ResizeVirtual - resize one of the small virtual windows
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
ResizeVirtual(window, width, height)
Window window;
int width;
int height;
{
    if (window) {
	width /= Scr->PannerScale;
	height /= Scr->PannerScale;
	if (width <= 0) width = 1;
	if (height <= 0) height = 1;
	XResizeWindow(dpy, window, width, height);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	MapFrame - map a client window and its frame
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
MapFrame(tmp_win)
TwmWindow *tmp_win;
{
    if (Scr->RememberScreenPosition && tmp_win->screen_x >= 0 ) {
	tmp_win->frame_x = tmp_win->screen_x + Scr->vdtPositionX;
	tmp_win->frame_y = tmp_win->screen_y + Scr->vdtPositionY;
	XMoveWindow(dpy, tmp_win->frame, tmp_win->frame_x, tmp_win->frame_y);
	if (tmp_win->virtualWindow) {
	    XMoveWindow(dpy, tmp_win->virtualWindow,
			tmp_win->frame_x / Scr->PannerScale,
			tmp_win->frame_y / Scr->PannerScale);
	}
    }
    XMapWindow(dpy, tmp_win->w);
    XMapWindow(dpy, tmp_win->frame);
    if (tmp_win->virtualWindow && !tmp_win->sticky)
	XMapWindow(dpy, tmp_win->virtualWindow);
}

/***********************************************************************
 *
 *  Procedure:
 *	UnmapFrame - unmap a client window and its frame
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
UnmapFrame(tmp_win)
TwmWindow *tmp_win;
{
    XUnmapWindow(dpy, tmp_win->frame);
    if (!tmp_win->iconmgr && tmp_win->w != Scr->Panner)
	XUnmapWindow(dpy, tmp_win->w);
    if (tmp_win->virtualWindow && !tmp_win->sticky)
	XUnmapWindow(dpy, tmp_win->virtualWindow);
    if (Scr->RememberScreenPosition 
	&& tmp_win->frame_x >= Scr->vdtPositionX
	&& tmp_win->frame_y >= Scr->vdtPositionY
	&& tmp_win->frame_x <=  Scr->vdtPositionX + Scr->MyDisplayWidth
	&& tmp_win->frame_y <=  Scr->vdtPositionY + Scr->MyDisplayHeight) {
	tmp_win->screen_x = tmp_win->frame_x - Scr->vdtPositionX;
	tmp_win->screen_y = tmp_win->frame_y - Scr->vdtPositionY;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	RaiseFrame - raise a client window and its frame
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
RaiseFrame(tmp_win)
TwmWindow *tmp_win;
{
    XRaiseWindow(dpy, tmp_win->frame);
    if (tmp_win->virtualWindow && !tmp_win->sticky)
	XRaiseWindow(dpy, tmp_win->virtualWindow);
}

/***********************************************************************
 *
 *  Procedure:
 *	LowerFrame - lower a client window and its frame
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
LowerFrame(tmp_win)
TwmWindow *tmp_win;
{
    XWindowChanges xwc;

    if (Scr->StickyAbove && tmp_win->sticky && Scr->VirtualDesktop) {
	xwc.sibling = Scr->VirtualDesktop;
	xwc.stack_mode = Above;
	XConfigureWindow(dpy, tmp_win->frame, CWSibling|CWStackMode, &xwc);
    }
    else
	XLowerWindow(dpy, tmp_win->frame);
    if (tmp_win->virtualWindow && !tmp_win->sticky)
	XLowerWindow(dpy, tmp_win->virtualWindow);
}

/***********************************************************************
 *
 *  Procedure:
 *	MapIcon - map the icon window
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
MapIcon(tmp_win)
TwmWindow *tmp_win;
{
    XMapRaised(dpy, tmp_win->icon.w);
    if (tmp_win->virtualIcon && !tmp_win->sticky)
	XMapRaised(dpy, tmp_win->virtualIcon);
}

/***********************************************************************
 *
 *  Procedure:
 *	UnmapIcon - unmap the icon window
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
UnmapIcon(tmp_win)
TwmWindow *tmp_win;
{
    XUnmapWindow(dpy, tmp_win->icon.w);
    if (tmp_win->virtualIcon && !tmp_win->sticky)
	XUnmapWindow(dpy, tmp_win->virtualIcon);
}

/***********************************************************************
 *
 *  Procedure:
 *	RaiseIcon - raise a client icon
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
RaiseIcon(tmp_win)
TwmWindow *tmp_win;
{
    XRaiseWindow(dpy, tmp_win->icon.w);
    if (tmp_win->virtualIcon && !tmp_win->sticky)
	XRaiseWindow(dpy, tmp_win->virtualIcon);
}

/***********************************************************************
 *
 *  Procedure:
 *	LowerIcon - lower an icon
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
LowerIcon(tmp_win)
TwmWindow *tmp_win;
{
    XWindowChanges xwc;

    if (Scr->StickyAbove && tmp_win->sticky && Scr->VirtualDesktop) {
	xwc.sibling = Scr->VirtualDesktop;
	xwc.stack_mode = Above;
	XConfigureWindow(dpy, tmp_win->icon.w, CWSibling|CWStackMode, &xwc);
    }
    else
	XLowerWindow(dpy, tmp_win->icon.w);
    if (tmp_win->virtualIcon && !tmp_win->sticky)
	XLowerWindow(dpy, tmp_win->virtualIcon);
}

/***********************************************************************
 *
 *  Procedure:
 *	MoveIcon - move an icon
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
MoveIcon(tmp_win, x, y)
TwmWindow *tmp_win;
int x, y;
{
    XMoveWindow(dpy, tmp_win->icon.w, x, y);
    tmp_win->icon_loc_x = x;
    tmp_win->icon_loc_y = y;
    if (tmp_win->virtualIcon)
	XMoveWindow(dpy, tmp_win->virtualIcon, x/Scr->PannerScale, y/Scr->PannerScale);
}

/***********************************************************************
 *
 *  Procedure:
 *	MakeVirtualDesktop - create the virtual desktop window
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
MakeVirtualDesktop(width, height)
int width, height;
{
    Pixmap pm;
    XSetWindowAttributes attr;
    unsigned attrMask;
    char display[80];
    char *ptr;
    int fres;
    char fg_text[14], bg_text[14];
    XColor colors[2];

/*
 *  Make vdt size an integral multiple of physical screen size
 *  if the given width and/or height are less than physical
 *  width and/or height.
 *  WM Kules, wmk@fed.frb.gov, Oct 15, 1990
 */
    if (width < Scr->MyDisplayWidth)
       width = width * Scr->MyDisplayWidth;
    if (height < Scr->MyDisplayHeight)
       height = height * Scr->MyDisplayHeight;
  
    pm = None;
    if (Scr->vdtPixmap) {
/*
 *   If XLOADIMAGE is not defined, this will be used.  Else, use the
 * xloadimage program (fork & exec) to load root images.  This allows
 * loading more type of images than just Xbm and Xpm.  If XLOADIMAGE
 * is defined, it should be defined as the full-path name of the
 * xloadimage executable.
 *                     Chris P. Ross	12-90
 */
#ifndef XLOADIMAGE
# ifdef XPM
	XpmColorSymbol ctrans[2];

	ctrans[0].name = "foreground";
	ctrans[0].value = NULL;
	ctrans[0].pixel = Scr->vdtC.fore;
	ctrans[1].name = "background";
	ctrans[1].value = NULL;
	ctrans[1].pixel = Scr->vdtC.back;

	pm = FindPixmap(Scr->vdtPixmap, &JunkWidth, &JunkHeight, JunkIsXpm,
			NULL, ctrans, 2, NULL);
# else
	pm = FindPixmap(Scr->vdtPixmap, &JunkWidth, &JunkHeight, JunkIsXpm,
			NULL, NULL, 0, NULL);
# endif  /* XPM */

#else  /* defined(XLOADIMAGE) */
	/* Build a display-name string to pass to xloadimage */
	bcopy(DisplayString(dpy), display, strlen(DisplayString(dpy))+1);
	ptr = display;
	while (*ptr != ':') ptr++;
	while ((*ptr != '.') && (*ptr)) ptr++;
	sprintf(ptr, ".%d", Scr->screen);

	/* get values for the colors for fg and bg */
	colors[0].pixel = Scr->vdtC.fore;
	colors[1].pixel = Scr->vdtC.back;
	XQueryColors(dpy, DefaultColormap(dpy, Scr->screen),
			colors, 2);
	/* build strings */
	sprintf(fg_text, "#%04x%04x%04x", colors[0].red,
		colors[0].green, colors[0].blue);
	sprintf(bg_text, "#%04x%04x%04x", colors[1].red,
		colors[1].green, colors[1].blue);
	/* Fork the process */
	fres = fork();
	if (fres < 0) fprintf(stderr, "%s: error in fork()...\n");
	if (fres == 0) {		/* This is the child */
		execlp("xloadimage", "xsetbg", "-display",
			display, "-foreground",
			fg_text, "-background", bg_text,
			Scr->vdtPixmap, NULL);
		/* No xloadimage in path, call it explicitly... */
		execlp(XLOADIMAGE, "xsetbg", "-display",
			display, "-foreground",
			fg_text, "-background", bg_text,
			Scr->vdtPixmap, NULL);
		/* Give up, you don't get a background... */
		exit(0);
	}
	/* fres > 0 :  Parent, therefore continue */
	xloadim_pid = fres;
#endif  /* XLOADIMAGE */
    }
    attrMask = CWOverrideRedirect | CWEventMask | CWBackPixmap | CWBackingStore;
    attr.override_redirect = True;
    attr.event_mask = SubstructureRedirectMask|SubstructureNotifyMask;
    attr.backing_store = NotUseful;
    attr.background_pixmap = Scr->rootWeave;
    if (pm)
	attr.background_pixmap = pm;
    else if (Scr->vdtBackgroundSet) {
	attrMask &= ~CWBackPixmap;
	attrMask |= CWBackPixel;
	attr.background_pixel = Scr->vdtC.back;
    }

    Scr->VirtualDesktop = XCreateWindow(dpy, Scr->Root, 0, 0, 
	    width+50, height+50, 0, Scr->d_depth, CopyFromParent,
	    Scr->d_visual, attrMask, &attr);
    Scr->vdtWidth = width;
    Scr->vdtHeight = height;

    /* this property allows ssetroot to find the virtual root window */
    XChangeProperty(dpy, Scr->VirtualDesktop, XA_SWM_VROOT, XA_WINDOW, 32,
	PropModeReplace, (unsigned char *)&Scr->VirtualDesktop, 1);

    if (pm)
	XFreePixmap(dpy, pm);
}

/***********************************************************************
 *
 *  Procedure:
 *	MakePanner - create the virtual desktop panner window
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
MakePanner()
{
    static XTextProperty wName={(unsigned char *) "Virtual Desktop",XA_STRING,8,15};
    static XTextProperty iName={(unsigned char *) "Desktop",XA_STRING,8,7};
    XSetWindowAttributes attr;
    XSizeHints *sizeHints;
    XWMHints *wmHints;
    XClassHint *classHints;
    int x, y, width, height;
    Pixmap pm;
    unsigned attrMask;
#ifdef XPM
    XpmColorSymbol ctrans[2];
#endif

    width = Scr->vdtWidth/Scr->PannerScale;
    height = Scr->vdtHeight/Scr->PannerScale;

    sizeHints = XAllocSizeHints();
    sizeHints->flags = PBaseSize;
    sizeHints->base_width = width;
    sizeHints->base_height = height;
    sizeHints->min_width = Scr->MyDisplayWidth/Scr->PannerScale;
    sizeHints->min_height = Scr->MyDisplayHeight/Scr->PannerScale;
    sizeHints->max_width = MAX_VDT_WIDTH/Scr->PannerScale;
    sizeHints->max_height = MAX_VDT_HEIGHT/Scr->PannerScale;
    XWMGeometry(dpy, Scr->screen, Scr->PannerGeometry, NULL,
	1, sizeHints, &x, &y, &width, &height, &sizeHints->win_gravity);
    sizeHints->flags = USPosition|PWinGravity|PMinSize|PMaxSize;

    wmHints = XAllocWMHints();
    wmHints->initial_state = Scr->PannerState;
    wmHints->flags = StateHint;

    classHints = XAllocClassHint();
    classHints->res_name = "virtualDesktop";
    classHints->res_class = "Twm";

#ifdef XPM
    /*
     * Fill the color translation table, so that bitmaps will be loaded
     * and converted into properly colored pixmaps by the call to
     * FindPixmap().
     */

    ctrans[0].name = "foreground";
    ctrans[0].value = NULL;
    ctrans[0].pixel = Scr->PannerC.fore;
    ctrans[1].name = "background";
    ctrans[1].value = NULL;
    ctrans[1].pixel = Scr->PannerC.back;
#endif

    pm = None;
    if (Scr->PannerPixmap)
	pm = FindPixmap(Scr->PannerPixmap, &JunkWidth, &JunkHeight, JunkIsXpm,
		NULL,
#ifdef XPM
		ctrans, 2,
#else
		NULL, 0,
#endif
		NULL);

    attrMask = CWBackPixmap|CWEventMask;
    attr.background_pixmap = Scr->rootWeave;
    attr.event_mask = ExposureMask | ButtonPressMask;
    if (pm)
	attr.background_pixmap = pm;
    else if (Scr->PannerBackgroundSet) {
	attrMask &= ~CWBackPixmap;
	attrMask |= CWBackPixel;
	attr.background_pixel = Scr->PannerC.back;
    }
    Scr->Panner = XCreateWindow(dpy, Scr->Root, x, y, 
	    sizeHints->base_width, sizeHints->base_height, 1,
	    Scr->d_depth, CopyFromParent,
	    Scr->d_visual, attrMask, &attr);
    
    /* we really, really, want one... */
    if (Scr->Panner == None) {
	fprintf(stderr, "%s: error creating panner\n", ProgramName);
	Scr->Panner = XCreateSimpleWindow(dpy, Scr->Root, x, y,
					sizeHints->base_width,
					sizeHints->base_height, 1,
					Scr->Black, Scr->Black);
    }
    XSaveContext(dpy, Scr->Panner, ScreenContext, (caddr_t)Scr);
    Scr->PannerWidth = sizeHints->base_width;
    Scr->PannerHeight = sizeHints->base_height;

    XGrabButton(dpy, Button1, 0L, Scr->Panner,
	True, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	GrabModeAsync, GrabModeAsync, Scr->Panner, None);
    XGrabButton(dpy, Button3, 0L, Scr->Panner,
	True, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	GrabModeAsync, GrabModeAsync, Scr->Panner, None);
    XSetWMProperties(dpy, Scr->Panner, &wName, &iName, NULL, 0,
	sizeHints, wmHints, classHints);

    if (Scr->PannerState != WithdrawnState)
	XMapRaised(dpy, Scr->Panner);

    XFree(sizeHints);
    XFree(wmHints);
    XFree(classHints);
}

void
HandlePannerExpose(ev)
XEvent *ev;
{
    XEvent dummy;

    if (ev->xexpose.count)
	return;

    if (!Scr->PannerOutlineWidth) {
	Scr->PannerOutlineWidth = Scr->MyDisplayWidth / Scr->PannerScale -1;
	Scr->PannerOutlineHeight = Scr->MyDisplayHeight / Scr->PannerScale -1;
    }
    XClearWindow(dpy, Scr->Panner);
    XDrawRectangle(dpy, Scr->Panner, Scr->PannerGC,
	Scr->PannerOutlineX, Scr->PannerOutlineY,
	Scr->PannerOutlineWidth, Scr->PannerOutlineHeight);

    /* flush other expose events */
    while (XCheckTypedWindowEvent (dpy, Scr->Panner, Expose, &dummy)) ;
}

#define GridMask (ShiftMask | ControlMask | Mod1Mask | Mod2Mask)

static int pannerButton = 0;
static int use_grid = 0;

void
HandlePannerButtonPress(ev)
XEvent *ev;
{
    if (!pannerButton) {
	pannerButton = ev->xbutton.button;
	pointerX = Scr->PannerOutlineX + Scr->PannerOutlineWidth/2;
	pointerY = Scr->PannerOutlineY + Scr->PannerOutlineHeight/2;
	XWarpPointer(dpy, None, Scr->Panner, 0,0,0,0,
	    pointerX, pointerY);
	XClearWindow(dpy, Scr->Panner);
	XDrawRectangle(dpy, Scr->Panner, Scr->DrawGC,
	    Scr->PannerOutlineX, Scr->PannerOutlineY,
	    Scr->PannerOutlineWidth, Scr->PannerOutlineHeight);
    }
}

void
HandlePannerButtonRelease(ev)
XEvent *ev;
{
    if (ev->xbutton.button == pannerButton) {
	pannerButton = 0;
	use_grid = ev->xmotion.state & GridMask;

	XDrawRectangle(dpy, Scr->Panner, Scr->DrawGC,
	    Scr->PannerOutlineX, Scr->PannerOutlineY,
	    Scr->PannerOutlineWidth, Scr->PannerOutlineHeight);
	XClearWindow(dpy, Scr->Panner);
	XDrawRectangle(dpy, Scr->Panner, Scr->PannerGC,
	    Scr->PannerOutlineX, Scr->PannerOutlineY,
	    Scr->PannerOutlineWidth, Scr->PannerOutlineHeight);

	if (use_grid) {
	    int xx = Scr->PannerOutlineX * Scr->PannerScale;
	    int yy = Scr->PannerOutlineY * Scr->PannerScale;

	    xx = xx - xx % Scr->MyDisplayWidth;
	    yy = yy - yy % Scr->MyDisplayHeight;
	    MoveDesktop(xx, yy);
	}
	else {
	    MoveDesktop(Scr->PannerOutlineX*Scr->PannerScale,
			Scr->PannerOutlineY*Scr->PannerScale);
	}
    }
}

static int TRUNCATE(pos, grid)
    int pos, grid;
{
    pos *= Scr->PannerScale;
    return ((pos - pos % grid) + Scr->PannerScale - 1) / Scr->PannerScale;
}

void
HandlePannerMotionNotify(ev)
XEvent *ev;
{
    XEvent dummyev;
    int deltaX, deltaY;
    int newOutlineX, newOutlineY;

    use_grid = ev->xmotion.state & GridMask;

    /* figure out the position of the next outline */
    deltaX = ev->xmotion.x - pointerX;
    deltaY = ev->xmotion.y - pointerY;

    if (use_grid) {
	newOutlineX = ev->xmotion.x;
	newOutlineY = ev->xmotion.y;
    }
    else {
	newOutlineX = Scr->PannerOutlineX + deltaX;
	newOutlineY = Scr->PannerOutlineY + deltaY;
    }

    if (newOutlineX < 0)
	newOutlineX = 0;
    if (newOutlineY < 0)
	newOutlineY = 0;

    if (use_grid) {
	newOutlineX = TRUNCATE (newOutlineX, Scr->MyDisplayWidth);
	newOutlineY = TRUNCATE (newOutlineY, Scr->MyDisplayHeight);
    }

    if ((newOutlineX + Scr->PannerOutlineWidth) >= Scr->PannerWidth)
	newOutlineX = Scr->PannerWidth - Scr->PannerOutlineWidth - 1;
    if ((newOutlineY + Scr->PannerOutlineHeight) >= Scr->PannerHeight)
	newOutlineY = Scr->PannerHeight - Scr->PannerOutlineHeight - 1;

    if (Scr->PannerOutlineX != newOutlineX ||
	Scr->PannerOutlineY != newOutlineY) {
	pointerX = ev->xmotion.x;
	pointerY = ev->xmotion.y;

	/* get rid of the last outline */
	XDrawRectangle(dpy, Scr->Panner, Scr->DrawGC,
		       Scr->PannerOutlineX, Scr->PannerOutlineY,
		       Scr->PannerOutlineWidth, Scr->PannerOutlineHeight);

	Scr->PannerOutlineX = newOutlineX;
	Scr->PannerOutlineY = newOutlineY;

	/* draw the new outline */
	XDrawRectangle(dpy, Scr->Panner, Scr->DrawGC,
		       Scr->PannerOutlineX, Scr->PannerOutlineY,
		       Scr->PannerOutlineWidth, Scr->PannerOutlineHeight);
    }
    while(XCheckTypedEvent(dpy,MotionNotify,&dummyev));
    
    if ((Scr->PannerOpaqueScroll && Scr->StickyAbove) ||
	(Scr->PannerOutlineX != newOutlineX ||
	 Scr->PannerOutlineY != newOutlineY)) {
	if (use_grid) {
	    int xx = Scr->PannerOutlineX * Scr->PannerScale;
	    int yy = Scr->PannerOutlineY * Scr->PannerScale;

	    xx = xx - xx % Scr->MyDisplayWidth;
	    yy = yy - yy % Scr->MyDisplayHeight;
	    MoveDesktop(xx, yy);
	}
	else {
	    MoveDesktop(Scr->PannerOutlineX*Scr->PannerScale,
			Scr->PannerOutlineY*Scr->PannerScale);
	}
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	MoveDesktop - move the desktop to a pixel location
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
MoveDesktop(x, y)
int x;
int y;
{
    TwmWindow *tmp;
    XSetWindowAttributes attributes;
    XEvent ev;
    register int delta_x, delta_y;
#ifdef INSTANT_MOVE_HACK
    Window hack;
#endif

    if (x != Scr->vdtPositionX || y != Scr->vdtPositionY) {
#ifdef INSTANT_MOVE_HACK
	XGrabServer(dpy);
	attributes.save_under = TRUE;
	attributes.backing_store = NotUseful;
	attributes.override_redirect = TRUE;
	attributes.background_pixmap = None;
	attributes.background_pixel = Scr->BorderColor;

	hack = XCreateWindow(dpy, Scr->Root, 0, 0,
				Scr->MyDisplayWidth, Scr->MyDisplayHeight,
				0, Scr->d_depth, InputOutput, Scr->d_visual,
				(CWBackingStore | CWSaveUnder 
				| CWOverrideRedirect | CWBackPixmap),
				&attributes);

	XMapWindow(dpy, hack);
#endif
	Scr->vdtSaveX = Scr->vdtPositionX;
	Scr->vdtSaveY = Scr->vdtPositionY;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if ((x + Scr->MyDisplayWidth) > Scr->vdtWidth)
	    x = Scr->vdtWidth - Scr->MyDisplayWidth;
	if ((y + Scr->MyDisplayHeight) > Scr->vdtHeight)
	    y = Scr->vdtHeight - Scr->MyDisplayHeight;
	XMoveWindow(dpy, Scr->VirtualDesktop, -x, -y);
	Scr->PannerOutlineX = x / Scr->PannerScale;
	Scr->PannerOutlineY = y / Scr->PannerScale;
	delta_x = x - Scr->vdtPositionX; delta_y = y - Scr->vdtPositionY;
	Scr->vdtPositionX = x;
	Scr->vdtPositionY = y;

	for (tmp = Scr->TwmRoot.next; tmp != NULL; tmp = tmp->next) {
	    if (!tmp->sticky)
		SendSyntheticConfigureNotify(tmp);
	    else if (!Scr->StickyAbove) {
		if (tmp->icon.w)
		    MoveIcon(tmp, tmp->icon_loc_x + delta_x, tmp->icon_loc_y + delta_y);
		SetupFrame( tmp, tmp->frame_x + delta_x, tmp->frame_y + delta_y,
				tmp->frame_width, tmp->frame_height, -1, True );
	    }
	}

#ifdef INSTANT_MOVE_HACK
	XDestroyWindow(dpy, hack);
	XUngrabServer(dpy);
#endif

	/* go repaint the panner */
	ev.xexpose.count = 0;
	HandlePannerExpose(&ev);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	ScrollDesktop - scroll the desktop by screenfuls
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
ScrollDesktop(func, pos)
int func;
char *pos;
{
    int x, y;
    int status;
    unsigned int width, height;

    x = Scr->vdtPositionX;
    y = Scr->vdtPositionY;

    switch (func)
    {
	case F_SCROLLBACK:
	    x = Scr->vdtSaveX;
	    y = Scr->vdtSaveY;
	    break;
	case F_SCROLL:
	    status = XParseGeometry(pos, &x, &y, &width, &height);
	    if ((status & (XValue | YValue)) != (XValue | YValue)) {
		fprintf (stderr,
			 "twm: ignoring invalid f.scroll geometry \"%s\"\n", pos);
		return;
	    }
	    x = abs(x) * Scr->MyDisplayWidth;
	    y = abs(y) * Scr->MyDisplayHeight;
	    if (status & XNegative)
		x = Scr->vdtWidth - Scr->MyDisplayWidth - x;
	    if (status & YNegative)
		y = Scr->vdtHeight - Scr->MyDisplayHeight - y;
	    break;
	case F_SCROLLHOME:
	    x = 0;
	    y = 0;
	    break;
  	case F_SCROLLRIGHT:
 	    x += Scr->vdtScrollDistanceX;
	    if (Scr->WrapVirtual)
		if ((x + Scr->MyDisplayWidth) > Scr->vdtWidth)
		    x = 0;
  	    break;
  	case F_SCROLLLEFT:
 	    x -= Scr->vdtScrollDistanceX;
	    if (Scr->WrapVirtual)
		if (x < 0)
		    x = Scr->vdtWidth - Scr->MyDisplayWidth;
  	    break;
  	case F_SCROLLDOWN:
 	    y += Scr->vdtScrollDistanceY;
	    if (Scr->WrapVirtual)
		if ((y + Scr->MyDisplayHeight) > Scr->vdtHeight)
		    y = 0;
  	    break;
  	case F_SCROLLUP:
 	    y -= Scr->vdtScrollDistanceY;
	    if (Scr->WrapVirtual)
		if (y < 0)
		    y = Scr->vdtHeight - Scr->MyDisplayHeight;
  	    break;
    }
    if (x < 0)
	x = 0;
    if (y < 0)
	y = 0;
    if ((x + Scr->MyDisplayWidth) > Scr->vdtWidth)
	x = Scr->vdtWidth - Scr->MyDisplayWidth;
    if ((y + Scr->MyDisplayHeight) > Scr->vdtHeight)
	y = Scr->vdtHeight - Scr->MyDisplayHeight;
    MoveDesktop(x, y);
}

/***********************************************************************
 *
 *  Procedure:
 *	ResizeDesktop - resize the desktop, the coordinates are in panner
 *		units.
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
ResizeDesktop(width, height)
int width;
int height;
{
    int vdtWidth, vdtHeight;

    if (width != Scr->PannerWidth || height != Scr->PannerHeight) {
	Scr->PannerWidth = width;
	Scr->PannerHeight = height;

	vdtWidth = width * Scr->PannerScale;
	vdtHeight = height * Scr->PannerScale;

	if (vdtWidth < Scr->vdtWidth) {
	    if ((Scr->PannerOutlineX + Scr->PannerOutlineWidth) >= Scr->PannerWidth)
		Scr->PannerOutlineX = Scr->PannerWidth-Scr->PannerOutlineWidth - 1;
	}
	if (vdtHeight < Scr->vdtHeight) {
	    if ((Scr->PannerOutlineY+Scr->PannerOutlineHeight) >= Scr->PannerHeight)
		Scr->PannerOutlineY = Scr->PannerHeight-Scr->PannerOutlineHeight-1;
	}
	Scr->vdtWidth = vdtWidth;
	Scr->vdtHeight = vdtHeight;

	MoveDesktop(Scr->PannerOutlineX * Scr->PannerScale,
	    Scr->PannerOutlineY * Scr->PannerScale);

	/* make it just a tad larger than requested */
	XResizeWindow(dpy, Scr->VirtualDesktop, vdtWidth + 2*Scr->PannerScale,
	    vdtHeight + 2*Scr->PannerScale);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandlePannerMove - prepare to move a small panner window
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
HandlePannerMove(ev, tmp_win)
XButtonEvent *ev;
TwmWindow *tmp_win;
{
    int cancel;
    int x_root, y_root;
    unsigned objWidth, objHeight;
    int moving_frame;

    tmp_win->root = Scr->Panner;

    x_root = ev->x_root;
    y_root = ev->y_root;

    if (ev->window == tmp_win->virtualWindow) {
	moving_frame = True;
	objWidth = tmp_win->frame_width + 2*tmp_win->frame_bw;
	objHeight = tmp_win->frame_height + 2*tmp_win->frame_bw;
    }
    else {
	moving_frame = False;
	objWidth = tmp_win->icon.width + 2;
	objHeight = tmp_win->icon.height + 2;
    }

    StartMove(tmp_win, ev->window, tmp_win->title_height,
	&x_root, &y_root, &cancel, IN_PANNER, Scr->PannerScale,
	objWidth, objHeight, False, False);

    tmp_win->root = Scr->VirtualDesktop;

    if (!cancel) {
	if (moving_frame) {
	    SetupWindow (tmp_win, x_root, y_root,
		 tmp_win->frame_width, tmp_win->frame_height, -1);
	}
	else {
	    MoveIcon(tmp_win, x_root, y_root);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	ScrollTo - scroll the virtual desktop to a window of non of
 *		the frame is visible
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
ScrollTo(tmp_win)
TwmWindow *tmp_win;
{
    int x, y;
    int xr, yb;

    if (!tmp_win->sticky) {
	x = Scr->vdtPositionX;
	y = Scr->vdtPositionY;
	xr = x + Scr->MyDisplayWidth;
	yb = y + Scr->MyDisplayHeight;

	if ((tmp_win->frame_x + tmp_win->frame_width) <= x)
	    x = tmp_win->frame_x;
	else if (tmp_win->frame_x >= xr)
	    x = tmp_win->frame_x;

	if ((tmp_win->frame_y + tmp_win->frame_height) <= y)
	    y = tmp_win->frame_y;
	else if (tmp_win->frame_y >= yb)
	    y = tmp_win->frame_y;

	/* if we are going to scroll, we might as well do it in both
	 * directions
	 */
	if (x != Scr->vdtPositionX || y != Scr->vdtPositionY) {
	    x = tmp_win->frame_x;
	    y = tmp_win->frame_y;
	    MoveDesktop(x, y);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	ScrollToQuadrant - scroll the virtual desktop to a the quadrant 
 *		that contains the client window.
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
ScrollToQuadrant(tmp_win)
TwmWindow *tmp_win;
{
    register int x, y;

    if (!tmp_win->sticky) {
	x=tmp_win->frame_x;
	y=tmp_win->frame_y;
	/* 16 is the average cursor width */
	if ( x + tmp_win->frame_width < Scr->vdtPositionX + 16 ||
		x >  Scr->vdtPositionX + Scr->MyDisplayWidth - 16 ||
	     y + tmp_win->frame_height < Scr->vdtPositionY + 16 ||
		y >  Scr->vdtPositionY + Scr->MyDisplayHeight - 16 ){
	    x = (x / Scr->MyDisplayWidth) * Scr->MyDisplayWidth;
	    y = (y / Scr->MyDisplayHeight) * Scr->MyDisplayHeight;
	    MoveDesktop(x, y);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	PaintVirtualWindow - paint the window name in the virtual window
 *	
 *  Returned Value:
 *	None
 *
 ***********************************************************************
 */

void
PaintVirtualWindow(tmp_win)
TwmWindow *tmp_win;
{
    if (Scr->ShowVirtualNames) {
	FBF(tmp_win->virtual.fore, tmp_win->virtual.back,
	    Scr->VirtualFont.font->fid);

	XClearArea(dpy, tmp_win->virtualWindow, 1, 1, 9999, Scr->VirtualFont.height, False);
	XDrawString (dpy, tmp_win->virtualWindow, Scr->NormalGC,
		     1, 1+Scr->VirtualFont.y, 
		     tmp_win->name, strlen(tmp_win->name));
    }
}
