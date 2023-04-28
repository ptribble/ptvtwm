/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/***********************************************************************
 *
 * Merge of piewm 1.04 with tvtwm-pl11
 * Portions Copyright 2010 Peter C Tribble <peter.tribble@gmail.com>
 *
 ***********************************************************************/

/**********************************************************************
 *
 * $XConsortium: icons.c,v 1.22 91/07/12 09:58:38 dave Exp $
 *
 * Icon releated routines
 *
 * $Log: icons.c,v $
 * Revision 1.17  1994/05/10  20:37:38  cross
 * Back to old code.  Still broken, I'm sure.
 *
 * Revision 1.17  1994/05/10  20:37:38  cross
 * Back to old code.  Still broken, I'm sure.
 *
 * Revision 1.15  1993/05/20  21:53:52  cross
 * Alpha stuff, prototypes weren't happy, and vdt.c needed DisplayString.
 *
 * Revision 1.14  1992/11/04  01:17:22  cross
 * Added code from rjc, with filtering.
 * See text in CHANGES for details.
 *
 * Revision 1.13  1992/08/27  15:34:57  cross
 * Added a pre-dec for a function
 *
 * Revision 1.12  1992/05/09  02:55:08  cross
 * Need an explicit return from pm_n_mask_to_pm().
 * Patch contributed by moss@BRL.MIL (Gary S. Moss)
 *
 * Revision 1.11  1992/05/09  02:48:05  cross
 * Add a function to do backwards contemptability for Richard.
 * Also, the "official" reorganization of the code in pm_n_mask_to_pm()
 *
 * Revision 1.10  1992/05/04  17:53:04  cross
 * Fixed an improper call to XDeleteContext...
 *
 * Revision 1.9  1992/05/02  20:09:59  cross
 * `Fixed a bug in the code to gen a pixmap from pixmap and mask.
 * When there's no mask, it *still* has to do an XCreatePixmap
 * so the the dest pixmap has valid geometry (width and height)
 *
 * Revision 1.7  1992/01/08  18:55:49  cross
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
 * Revision 1.6  1991/10/29  22:28:11  cross
 * Updated to be a strict superset of X11R5's twm, not X11R4's.
 *
 * Revision 1.5  1991/10/03  00:13:31  cross
 * Fixed some bogus uses of the NULL symbol
 *
 * Revision 1.4  1991/10/01  20:13:10  cross
 * Fixed things so that the shape_mask returned from an Xpm with
 * shaping in it works.  (Yay!)
 *
 * Revision 1.3  1991/09/26  00:32:30  cross
 * changed GetBitmap() calls to GetPixmap()
 *
 * Revision 1.2  1991/09/26  00:09:45  cross
 * Changed FindBitmap calls to FindPixmap.  And, did a little in the way of
 * correctly using the returned shape_mask, but I doubt it'll work right.
 *
 * Revision 1.1  1991/09/25  23:48:20  cross
 * Initial revision
 *
 * Revision 10.0  91/06/12  09:05:35  toml
 * Revision bump
 * 
 * Revision 9.0  91/04/23  07:40:37  toml
 * Revision bump
 * 
 * Revision 8.0  90/11/15  20:02:40  toml
 * Revision bump
 * 
 * Revision 7.3  90/11/12  21:34:53  toml
 * Implemented Scr->StickyAbove
 * 
 * Revision 7.2  90/11/12  19:57:23  toml
 * Patches to allow sticky windows to lower
 * 
 * Revision 1.2  90/11/04  18:30:24  brossard
 * When non-sticky client provide their own icons, don't forget to
 * reparent the root.
 * Removed the test above for stickyness since all windows are now child
 * of the virtual root.
 * 
 *
 * 10-Apr-89 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#include <stdio.h>
#include "twm.h"
#include "screen.h"
#include "icons.h"
#include "gram.h"
#include "parse.h"
#include "util.h"
#include "vdt.h"

#define iconWidth(w)	(w->icon.width + 2 * w->icon.bw)
#define iconHeight(w)	(w->icon.height + 2 * w->icon.bw)

#ifdef SHAPE
Pixmap SetIconMask();
Pixmap SetIconClip();
#endif

static
void
splitEntry (ie, grav1, grav2, w, h)
    IconEntry	*ie;
    int		grav1, grav2;
    int		w, h;
{
    IconEntry	*new;

    switch (grav1) {
    case D_NORTH:
    case D_SOUTH:
	if (w != ie->w)
	    splitEntry (ie, grav2, grav1, w, ie->h);
	if (h != ie->h) {
	    new = (IconEntry *)malloc (sizeof (IconEntry));
	    new->twm_win = 0;
	    new->used = 0;
	    new->next = ie->next;
	    ie->next = new;
	    new->x = ie->x;
	    new->h = (ie->h - h);
	    new->w = ie->w;
	    ie->h = h;
	    if (grav1 == D_SOUTH) {
		new->y = ie->y;
		ie->y = new->y + new->h;
	    } else
		new->y = ie->y + ie->h;
	}
	break;
    case D_EAST:
    case D_WEST:
	if (h != ie->h)
	    splitEntry (ie, grav2, grav1, ie->w, h);
	if (w != ie->w) {
	    new = (IconEntry *)malloc (sizeof (IconEntry));
	    new->twm_win = 0;
	    new->used = 0;
	    new->next = ie->next;
	    ie->next = new;
	    new->y = ie->y;
	    new->w = (ie->w - w);
	    new->h = ie->h;
	    ie->w = w;
	    if (grav1 == D_EAST) {
		new->x = ie->x;
		ie->x = new->x + new->w;
	    } else
		new->x = ie->x + ie->w;
	}
	break;
    }
}

int
roundUp (v, multiple)
int	v;
int	multiple;
{
    return ((v + multiple - 1) / multiple) * multiple;
}

void
PlaceIcon(tmp_win, def_x, def_y, final_x, final_y)
TwmWindow *tmp_win;
int def_x, def_y;
int *final_x, *final_y;
{
    IconRegion	*ir;
    IconEntry	*ie;
    int		w = 0, h = 0;
    name_list	*ir_list;

    ie = 0;
    ir_list = Scr->IconRegions;

    while (ir_list)
	if (ir = (IconRegion *)MultiLookInList(ir_list, tmp_win->full_name,
						&tmp_win->class, &ir_list)) {
	    w = roundUp (iconWidth (tmp_win), ir->stepx);
	    h = roundUp (iconHeight (tmp_win), ir->stepy);
	    for (ie = ir->entries; ie; ie=ie->next) {
		if (ie->used)
		    continue;
		/* If the icon position is off-screen, ignore it */
		if (tmp_win->root == Scr->VirtualDesktop &&
			((ie->x + w > Scr->vdtPositionX + Scr->MyDisplayWidth)
			|| (ie->x + ie->w - w <  Scr->vdtPositionX)
			|| (ie->y + h > Scr->vdtPositionY + Scr->MyDisplayHeight)
			|| (ie->y + ie->h - h < Scr->vdtPositionY)))
		    continue;
		if (ie->w >= w && ie->h >= h)
		    break;
	    }
	if (ie)
	    break;
    }
    if (ie) {
	splitEntry (ie, ir->grav1, ir->grav2, w, h);
	ie->used = 1;
	ie->twm_win = tmp_win;
	*final_x = ie->x + (ie->w - iconWidth (tmp_win)) / 2;
	*final_y = ie->y + (ie->h - iconHeight (tmp_win)) / 2;
    } else {
	if (tmp_win->root == Scr->VirtualDesktop) {
	    def_x += Scr->vdtPositionX;
	    def_y += Scr->vdtPositionY;
	}
	*final_x = def_x;
	*final_y = def_y;
    }
    return;
}

static IconEntry *
FindIconEntry (tmp_win, irp)
    TwmWindow   *tmp_win;
    IconRegion	**irp;
{
    IconRegion	*ir;
    name_list	*ir_list;
    IconEntry	*ie;

    for (ir_list = Scr->IconRegions ; ir_list ; ir_list = next_entry(ir_list)) {
	ir = (IconRegion *)contents_of_entry(ir_list);
	for (ie = ir->entries; ie; ie=ie->next)
	    if (ie->twm_win == tmp_win) {
		if (irp)
		    *irp = ir;
		return ie;
	    }
    }
    return 0;
}

void
IconUp (tmp_win)
    TwmWindow   *tmp_win;
{
    int		x, y;
    int		defx, defy;
    struct IconRegion *ir;
    name_list	*ir_list;

    /*
     * If the client specified a particular location, let's use it (this might
     * want to be an option at some point).  Otherwise, try to fit within the
     * icon region.
     */
    if (tmp_win->wmhints && (tmp_win->wmhints->flags & IconPositionHint))
      return;

    if (tmp_win->icon_moved) {
	if (!XGetGeometry (dpy, tmp_win->icon.w, &JunkRoot, &defx, &defy,
			   &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth))
	  return;

	x = defx + ((int) JunkWidth) / 2;
	y = defy + ((int) JunkHeight) / 2;

	ir_list = Scr->IconRegions;
	while (ir_list)
	    if (ir = (IconRegion *)MultiLookInList(ir_list, tmp_win->full_name,
						&tmp_win->class, &ir_list)) {
		if (x >= ir->x && x < (ir->x + ir->w) &&
			y >= ir->y && y < (ir->y + ir->h))
		    break;
	}
	if (!ir) return;		/* outside icon regions, leave alone */
    }

    defx = -100;
    defy = -100;
    PlaceIcon(tmp_win, defx, defy, &x, &y);
    if (tmp_win->root == Scr->VirtualDesktop) {
	defx += Scr->vdtPositionX;
	defy += Scr->vdtPositionY;
    }
    if (x != defx || y != defy) {
	MoveIcon(tmp_win, x, y);
	tmp_win->icon_moved = FALSE;	/* since we've restored it */
    }
}

static IconEntry *
prevIconEntry (ie, ir)
    IconEntry	*ie;
    IconRegion	*ir;
{
    IconEntry	*ip;

    if (ie == ir->entries)
	return 0;
    for (ip = ir->entries; ip->next != ie; ip=ip->next)
	;
    return ip;
}

/* old is being freed; and is adjacent to ie.  Merge
 * regions together
 */

static
void
mergeEntries (old, ie)
    IconEntry	*old, *ie;
{
    if (old->y == ie->y) {
	ie->w = old->w + ie->w;
	if (old->x < ie->x)
	    ie->x = old->x;
    } else {
	ie->h = old->h + ie->h;
	if (old->y < ie->y)
	    ie->y = old->y;
    }
}

void
IconDown (tmp_win)
    TwmWindow   *tmp_win;
{
    IconEntry	*ie, *ip, *in;
    IconRegion	*ir;

    ie = FindIconEntry (tmp_win, &ir);
    if (ie) {
	ie->twm_win = 0;
	ie->used = 0;
	ip = prevIconEntry (ie, ir);
	in = ie->next;
	for (;;) {
	    if (ip && ip->used == 0 &&
	       ((ip->x == ie->x && ip->w == ie->w) ||
	        (ip->y == ie->y && ip->h == ie->h)))
	    {
	    	ip->next = ie->next;
	    	mergeEntries (ie, ip);
	    	free ((char *) ie);
		ie = ip;
	    	ip = prevIconEntry (ip, ir);
	    } else if (in && in->used == 0 &&
	       ((in->x == ie->x && in->w == ie->w) ||
	        (in->y == ie->y && in->h == ie->h)))
	    {
	    	ie->next = in->next;
	    	mergeEntries (in, ie);
	    	free ((char *) in);
	    	in = ie->next;
	    } else
		break;
	}
    }
}

static void AddOneIconRegion();

void
AddIconRegion(name, type, geom, itterate, grav1, grav2, stepx, stepy)
char *name;
short type;
char *geom;
Bool itterate;
int grav1, grav2, stepx, stepy;
{
    int mask, x, y, w, h;

    mask = XParseGeometry(geom, &x, &y, (unsigned int *)&w, (unsigned int *)&h);

    if (mask & XNegative)
	x += Scr->MyDisplayWidth - w;

    if (mask & YNegative)
	y += Scr->MyDisplayHeight - h;

    if (itterate && Scr->VirtualDesktop) {
	int x1, y1;
	int vdtw = Scr->vdtWidth;
	int vdth = Scr->vdtHeight;

	if (vdtw < Scr->MyDisplayWidth)
	    vdtw *= Scr->MyDisplayWidth;
	if (vdth < Scr->MyDisplayHeight)
	    vdth *= Scr->MyDisplayHeight;

	for (x1 = x ; x1 < vdtw ; x1 += Scr->MyDisplayWidth)
	    for (y1 = y ; y1 < vdth ; y1 += Scr->MyDisplayHeight)
		AddOneIconRegion(name, type, x1, y1, w, h,
				grav1, grav2, stepx, stepy);
    } else
	AddOneIconRegion(name, type, x, y, w, h, grav1, grav2, stepx, stepy);
}

static
void
AddOneIconRegion(name, type, x, y, w, h, grav1, grav2, stepx, stepy)
char *name;
short type;
int   x, y, w, h;
int grav1, grav2, stepx, stepy;
{
    IconRegion *ir;

    ir = (IconRegion *)malloc(sizeof(IconRegion));
    AddToList(&(Scr->IconRegions), name, type, (char *)ir);

    ir->entries = NULL;
    ir->name = name;
    ir->grav1 = grav1;
    ir->grav2 = grav2;
    if (stepx <= 0)
	stepx = 1;
    if (stepy <= 0)
	stepy = 1;
    ir->stepx = stepx;
    ir->stepy = stepy;
    ir->x = x;
    ir->y = y;
    ir->w = w;
    ir->h = h;

    ir->entries = (IconEntry *)malloc(sizeof(IconEntry));
    ir->entries->next = 0;
    ir->entries->x = ir->x;
    ir->entries->y = ir->y;
    ir->entries->w = ir->w;
    ir->entries->h = ir->h;
    ir->entries->twm_win = 0;
    ir->entries->used = 0;
}

#ifdef comment
FreeIconEntries (ir)
    IconRegion	*ir;
{
    IconEntry	*ie, *tmp;

    for (ie = ir->entries; ie; ie=tmp)
    {
	tmp = ie->next;
	free ((char *) ie);
    }
}
FreeIconRegions()
{
    IconRegion *ir, *tmp;

    for (ir = Scr->FirstRegion; ir != NULL;)
    {
	tmp = ir;
	FreeIconEntries (ir);
	ir = ir->next;
	free((char *) tmp);
    }
    Scr->FirstRegion = NULL;
    Scr->LastRegion = NULL;
}
#endif

/*
 * Here is the code to create an icon. There are three things to do.
 *	1) Create the windows making up the icon.
 *	2) Create the shape.
 *	3) Position the icon.
 * We split these out. The shaping code needs to be called from event.c
 * too to handle changes in icon name size.
 */

static int
pm_n_mask_to_pm(pm, mask, depth,
		       width, height,
		       foreground,
		       background,
		       border_pixel,
		       pmp)
Pixmap pm, mask;
int depth;
int width, height;
Pixel foreground, background, border_pixel;
Pixmap *pmp;
{
    if ((*pmp = XCreatePixmap(dpy, Scr->Root, width,
				height, Scr->d_depth)) == None)
	    return False;

    if (mask != None) {
	GC maskedGC;
	XGCValues vals;

	/* Set pm to background colour */

	FB(background, 0);
	XFillRectangle(dpy, *pmp, Scr->NormalGC,
		   0, 0, width, height);

	vals.clip_mask = mask;
	vals.clip_x_origin = 0;
	vals.clip_y_origin = 0;
	vals.foreground = foreground;
	vals.background = background;

	maskedGC = XCreateGC(dpy, *pmp,
			 GCClipMask | GCClipXOrigin | GCClipYOrigin
			 | GCForeground | GCBackground,
			 &vals);
	if (depth > 1)
	    XCopyArea(dpy, pm, *pmp, maskedGC, 0, 0, width, height, 0, 0);
	else
	    XCopyPlane(dpy, pm, *pmp, maskedGC, 0, 0, width, height, 0, 0, 1);
	XFreeGC(dpy, maskedGC);
    } else {
	FB(foreground, background);

	if (depth > 1)
	    XCopyArea(dpy, pm, *pmp, Scr->NormalGC, 0, 0, width, height, 0, 0);
	else
	    XCopyPlane(dpy, pm, *pmp, Scr->NormalGC, 0, 0, width, height, 0, 0, 1);
    }
    return True;
}

#ifdef RJC

/*
 * For backwards compatability, this looks up bitmaps and masks and builds
 * an image which can later be used.
 */

char *
CacheAndNameIconImage(bm_name, mask_name)
char *bm_name, *mask_name;
{
    char *full_name;
    Pixmap pm, mask, clip;
    struct cent {
	Pixmap pm;
	Pixmap mask;
	Pixmap clip;
	unsigned int width, height;
	int depth;
    } *cached;
    int isXpm;
#ifdef XPM
    XpmColorSymbol ctrans[2];

    ctrans[0].name = "foreground";
    ctrans[0].value = NULL;
    ctrans[0].pixel = Scr->IconC.fore;
    ctrans[1].name = "background";
    ctrans[1].value = NULL;
    ctrans[1].pixel = Scr->IconC.back;
#endif

    cached = (struct cent *)malloc(sizeof(struct cent));

    if ((cached->pm = FindPixmap(bm_name, &(cached->width), &(cached->height),
				&isXpm, NULL,
#ifdef XPM
				ctrans, 2,
#else
				NULL, 0,
#endif
				NULL)) == None) {
	free(cached);
	return NULL;
    }

    if (isXpm) {
	free(cached);
	return NULL;
    }

    if ((cached->mask = FindPixmap(mask_name, &(cached->width),
				&(cached->height), &isXpm, NULL,
#ifdef XPM
				ctrans, 2,
#else
				NULL, 0,
#endif
				NULL)) == None) {
	free(cached);
	return NULL;
    }

    if (isXpm) {
	free(cached);
	return NULL;
    }

#ifdef ADD_BORDER_TO_MASK
    if (cached->mask) {
	extern Pixmap MaskToClip();

	cached->clip = MaskToClip(cached->mask);
    } else
	cached->clip = None;
#else
    cached->clip = cached->mask;
#endif  /* ADD_BORDER_TO_MASK */

    cached->depth = 1;
    full_name = (char *)malloc(strlen(bm_name)+strlen(mask_name)+4);
    sprintf(full_name, "%s / %s", bm_name, mask_name);
    AddToList(&Scr->Icons, full_name, LTYPE_EXACT_NAME, cached);

    return full_name;
}
#endif

/*
 * CreateIconImage builds an icon image as a pixmap, a mask and a clip mask.
 */

static int
CreateIconImage(tmp_win, parent, name, 
		imagep, maskp, clipp,
		widthp, heightp)
TwmWindow *tmp_win;
Window parent;
char *name;
Window *imagep;
Pixmap *maskp, *clipp;
unsigned int *widthp, *heightp;
{
    struct cent {
	Pixmap pm;
	Pixmap mask;
	Pixmap clip;
	unsigned int width, height;
	int depth;
    } *cached;
    Window image;
    XSetWindowAttributes attributes;
    Pixmap pm, clip;
    Bool isXpm;

    /* check cache first */
    if ((cached=(struct cent *)LookInNameList(Scr->Icons, name)) == None) {
#ifdef XPM
	XpmColorSymbol ctrans[2];

	ctrans[0].name = "foreground";
	ctrans[0].value = NULL;
	ctrans[0].pixel = Scr->IconC.fore;
	ctrans[1].name = "background";
	ctrans[1].value = NULL;
	ctrans[1].pixel = Scr->IconC.back;
#endif

	cached=(struct cent *)malloc(sizeof(struct cent));

	if ((cached->pm = FindPixmap (name,
					&(cached->width),
					&(cached->height),
					&isXpm, NULL,
#ifdef XPM
					ctrans, 2, 
#else
					NULL, 0,
#endif
					&(cached->mask))) == None) {
	    free(cached);
	    return False;
	}

#ifdef ADD_BORDER_TO_MASK
	if ( cached->mask ) {
	    extern Pixmap MaskToClip();
	    cached->clip = MaskToClip(cached->mask);
        } else
	    cached->clip = None;
#else
	cached->clip = cached->mask;
#endif /* ADD_BORDER_TO_MASK */

	cached->depth = isXpm?Scr->d_depth:1;

	AddToList(&Scr->Icons, name, LTYPE_EXACT_NAME, cached);
    }

    if (!pm_n_mask_to_pm(cached->pm, cached->mask, cached->depth,
			    cached->width, cached->height,
			    tmp_win->iconc.fore,
			    tmp_win->iconc.back,
			    tmp_win->icon_border,
			    &pm))
	return False;

    attributes.background_pixmap = pm;
    attributes.border_pixel = tmp_win->icon_border;

    /* Ok, we have the pixmap, now create the window */
    if ((image=XCreateWindow(dpy, parent,
				0,0,
				cached->width, cached->height,
				0,
				CopyFromParent, 
				(unsigned int) CopyFromParent,
				(Visual *) CopyFromParent,
				CWBackPixmap|CWBorderPixel, 
				&attributes))==None)
	return False;

    if ( pm != cached->pm )
	XFreePixmap(dpy, pm);

    *imagep = image;
    *maskp = cached->mask;
    *clipp = cached->clip;
    *widthp = cached->width;
    *heightp = cached->height;

    return True;
}

/*
 * CreateSuppliedImage is similar but uses a pixmap and mask
 * handed to it -- this is for application supplied icons.
 */

static int
CreateSuppliedImage(tmp_win, parent,
		    pm, mask,
		    imagep, maskp, clipp,
		    widthp, heightp)
TwmWindow *tmp_win;
Window parent;
Pixmap pm, mask;
Window *imagep;
Pixmap *maskp, *clipp;
unsigned int *widthp, *heightp;
{
    Window image;
    XSetWindowAttributes attributes;
    unsigned int width, height, depth;
    Pixmap newpm;
    Pixmap clip;

    XGetGeometry(dpy, pm, &JunkRoot, &JunkX, &JunkY,
		&width,
		&height,
		&JunkBW, &depth);

    if (!pm_n_mask_to_pm(pm, mask, depth,
			width, height,
			tmp_win->iconc.fore,
			tmp_win->iconc.back,
			tmp_win->icon_border,
			&newpm))
	return False;

#ifdef ADD_BORDER_TO_MASK
    if (mask) {
	extern Pixmap MaskToClip();

	clip = MaskToClip(mask);
    } else
	clip = None;
#else
    clip = mask;
#endif /* ADD_BORDER_TO_MASK */


    attributes.background_pixmap = newpm;
    attributes.border_pixel = tmp_win->icon_border;

    /* Ok, we have the pixmap, now create the window */
    if ((image=XCreateWindow(dpy, parent,
				0, 0,
				width, height,
				0,
				CopyFromParent, 
				(unsigned int) CopyFromParent,
				(Visual *) CopyFromParent,
				CWBackPixmap | CWBorderPixel,
				&attributes))==None)
	return False;

    if (newpm != pm)
	XFreePixmap(dpy, newpm);

    *imagep=image;
    *maskp=mask;
    *clipp=clip;
    *widthp=width;
    *heightp=height;

    return True;
}

/*
 * CreateIconWindows Actually creates the windows representing the icon.
 */
void
CreateIconWindows(tmp_win)
TwmWindow *tmp_win;
{
    Window frame_w;
    XSetWindowAttributes attributes;
    TwmIcon *icon = &(tmp_win->icon);

    /* create the icon base window. This is not given a reasonable size yet. */

    icon->has_image = False;
    icon->has_text = False;
    icon->image_not_ours = False;

    icon->text_w=None;
    icon->image_w=None;

    icon->text = NULL;
    icon->mask = None;
    icon->clip = None;

    icon->w = frame_w = XCreateSimpleWindow(dpy, tmp_win->root,
			      0, 0, 1, 1,
			      Scr->IconBorderWidth, 
			      tmp_win->icon_border, 
			      tmp_win->iconc.back);

    if (Scr->VirtualDesktop)
	tmp_win->virtualIcon = MakeVirtual(tmp_win, 0, 0, 0, 0,
					tmp_win->virtual.back,
					tmp_win->icon_border);

    tmp_win->forced = FALSE;

    /* Part one, create the image window */

    /* first, try forcing it */

    if (Scr->ForceIcon) {
	char *icon_name;

	if ((icon_name = LookInList(Scr->IconNames, tmp_win->full_name,
			   &tmp_win->class)))
	    icon->has_image = CreateIconImage(tmp_win, frame_w, icon_name, 
						&(icon->image_w),
						&(icon->mask),
						&(icon->clip),
						&(icon->width),
						&(icon->height));

	tmp_win->forced = icon->has_image;
    }

    /* next try looking for a Wm hint pixmap */
    if (!icon->has_image
	&& tmp_win->wmhints 
	&& (tmp_win->wmhints->flags & IconPixmapHint)) {
	Pixmap pm, mask;

	pm = tmp_win->wmhints->icon_pixmap;
    
	if (tmp_win->wmhints->flags & IconMaskHint)
	    mask = tmp_win->wmhints->icon_mask;
	else
	    mask = None;

	icon->has_image = CreateSuppliedImage(tmp_win, frame_w, 
						pm, mask,
						&(icon->image_w),
						&(icon->mask),
						&(icon->clip),
						&(icon->width),
						&(icon->height));
    }

    /* Next look for a Wm hint window */

    if (!icon->has_image && tmp_win->wmhints 
	&& (tmp_win->wmhints->flags & IconWindowHint)) {
	if (tmp_win->icon_title) {
	    icon->image_w = tmp_win->wmhints->icon_window;
    
	    XGetGeometry(dpy, tmp_win->wmhints->icon_window, &JunkRoot,
			&JunkX, &JunkY, 
			(unsigned int *)&icon->image_width,
			(unsigned int *)&icon->image_height,
			&JunkBW, &JunkDepth);

	    /* put it on top of our frame */

	    XReparentWindow (dpy, icon->image_w, icon->w, 0, 0);
	} else {
	    /* Nasty special case for stand alone icon windows */
	    XDestroyWindow(dpy, icon->w);
	    icon->w = frame_w = icon->image_w = tmp_win->wmhints->icon_window;
	
	    XReparentWindow (dpy, icon->w, tmp_win->root, 0, 0);
	}
	XAddToSaveSet(dpy, icon->image_w);

	icon->has_image = icon->image_not_ours = True;
    }
	
    /* if we get to here and still havn't got one
     * then look in the list again.
     */

    if (!icon->has_image && !Scr->ForceIcon) {
	char *icon_name;
    
	if ((icon_name = LookInList(Scr->IconNames, tmp_win->full_name,
				&tmp_win->class)))
	    icon->has_image = CreateIconImage(tmp_win, frame_w, icon_name, 
						&(icon->image_w),
						&(icon->mask),
						&(icon->clip),
						&(icon->width),
						&(icon->height));
    }

    /* Finally try for the unknown icon */

    if (!icon->has_image && Scr->Unknown.name) {
	icon->has_image = CreateIconImage(tmp_win, frame_w, Scr->Unknown.name,
						&(icon->image_w),
						&(icon->mask),
						&(icon->clip),
						&(icon->width),
						&(icon->height));
    }

    /* Part two, create the text window */

    if (icon->w != icon->image_w && (tmp_win->icon_title 
	|| !icon->has_image)) /* must have either title or image */
    {
	icon->has_text = True;
	icon->text = tmp_win->icon_name;
    
	icon->text_width = XTextWidth(Scr->IconFont.font, icon->text, 
				strlen(icon->text)) + ICON_TEXT_HBORDER*2;

	icon->text_height = Scr->IconFont.height + ICON_TEXT_VBORDER*2;

	attributes.background_pixel = tmp_win->iconc.back;
	attributes.border_pixel = tmp_win->icon_border;
	attributes.event_mask = ExposureMask;

	icon->text_w = XCreateWindow(dpy, icon->w,
				0, 0,
				icon->text_width,
				icon->text_height,
				0,
				CopyFromParent, 
				(unsigned int) CopyFromParent,
				(Visual *) CopyFromParent,
				CWBackPixel | CWBorderPixel | CWEventMask,
				&attributes);
    }

    XSaveContext(dpy, icon->w, TwmContext, (caddr_t)tmp_win);
    XSaveContext(dpy, icon->w, ScreenContext, (caddr_t)Scr);

    if (icon->has_text) {
	XSaveContext(dpy, icon->text_w, TwmContext, (caddr_t)tmp_win);
	XSaveContext(dpy, icon->text_w, ScreenContext, (caddr_t)Scr);
    }

    if (icon->has_image && icon->w != icon->image_w)
    {
	XSaveContext(dpy, icon->image_w, TwmContext, (caddr_t)tmp_win);
	XSaveContext(dpy, icon->image_w, ScreenContext, (caddr_t)Scr);
    }

    XSelectInput (dpy, frame_w,
			KeyPressMask | ButtonPressMask | ButtonReleaseMask);

    XDefineCursor(dpy, frame_w, Scr->IconCursor);

}

/*
 * ConfigureIconWindows Places and sizes the windows.
 */

#define max(x,y) ((x)>(y)?(x):(y))

void
ConfigureIconWindows(tmp_win)
TwmWindow *tmp_win;
{
    TwmIcon *icon = &(tmp_win->icon);

    if ( icon->w != icon->image_w) {
	/* first find out how big the image and/or text is */
	if (icon->has_image) {
	    XGetGeometry(dpy, icon->image_w, &JunkRoot, 
			&JunkX, &JunkY,
			&(icon->image_width),
			&(icon->image_height),
			&(icon->image_bw), &JunkDepth);
	} else
	    icon->image_bw=icon->image_width=icon->image_height=0;

	if (icon->has_text) {
	    icon->text_width = XTextWidth(Scr->IconFont.font, icon->text, 
				strlen(icon->text)) + ICON_TEXT_HBORDER * 2;

	    icon->text_height = Scr->IconFont.height + ICON_TEXT_VBORDER*2;
	    icon->text_bw=0;
	} else
	    icon->text_bw=icon->text_width=icon->text_height=0;

	/* the main window must hold either. */

	icon->width = max(icon->text_width+2*icon->text_bw, 
				icon->image_width+2*icon->image_bw);
	icon->height = icon->text_height + icon->image_height 
			+ 2*icon->image_bw + 2*icon->text_bw;

	icon->bw = Scr->IconBorderWidth;

	/* the image is at the top center */

	icon->image_y = 0;
	icon->image_x = (icon->width-icon->image_width)/2 - icon->image_bw;

	/* text is below the image and centered */
	icon->text_y = icon->image_y + icon->image_height 
			+ 2* icon->image_bw;
	icon->text_x = (icon->width-icon->text_width)/2 - icon->text_bw;

	/* move the text down a bit cos it looks nice this way */

	if ( icon->has_text && (tmp_win->squeeze_icon || icon->mask )) {
	    icon->text_y += icon->bw;
	    icon->height += icon->bw;
	}
    } else {
	XGetGeometry(dpy, icon->image_w, &JunkRoot, 
			&JunkX, &JunkY,
			&(icon->image_width),
			&(icon->image_height),
			&(icon->image_bw), &JunkDepth);

	icon->width = icon->image_width;
	icon->height = icon->image_height;
	if (icon->image_bw)
	    icon->bw = icon->image_bw;
	else
	    icon->bw = Scr->IconBorderWidth;
    }

    /* Now we enforce this configuration */

    XResizeWindow(dpy, icon->w, icon->width, icon->height);
    XSetWindowBorderWidth(dpy, icon->w, icon->bw);

    ResizeVirtual(tmp_win->virtualIcon, 
		  icon->width, icon->height);

    if (icon->has_text)
	XMoveResizeWindow(dpy, icon->text_w, 
			  icon->text_x, icon->text_y,
			  icon->text_width, icon->text_height);
    if (icon->has_image && icon->w != icon->image_w)
	XMoveResizeWindow(dpy, icon->image_w, 
			  icon->image_x, icon->image_y,
			  icon->image_width, icon->image_height);

#ifdef DEBUG
    printf("icon config for %s\n", tmp_win->icon_name);
    printf("\timage = %dx%d + %d at (%d,%d)\n", icon->image_width, icon->image_height, icon->image_bw, icon->image_x, icon->image_y);
    printf("\ttext = %dx%d + %d at (%d,%d)\n", icon->text_width, icon->text_height, icon->text_bw, icon->text_x, icon->text_y);
    printf("\ttotal = %dx%d + %d\n", icon->width, icon->height, icon->bw);
#endif
}

/*
 * ShapeIconWindows Shapes the windows.
 *
 * There are two reasons to shape the icon. Either the image is shaped
 * or SqueezeIcon has been turned on for this window.
 */
void
ShapeIconWindows(tmp_win)
TwmWindow *tmp_win;
{
    XRectangle rect;
    TwmIcon *icon = &(tmp_win->icon);

#ifdef SHAPE
    if (HasShape && icon->w != icon->image_w 
	&& (icon->mask || tmp_win->squeeze_icon)) {
	if (icon->clip) {
	    XShapeCombineMask(dpy, icon->w,
				ShapeBounding,
				icon->image_x, icon->image_y,
				icon->clip, ShapeSet);
	    XShapeCombineMask(dpy, icon->w,
				ShapeClip,
				icon->image_x, icon->image_y,
				icon->mask, ShapeSet);
	} else {
	    /* must be squeeze icon */
	    rect.x = icon->image_x;
	    rect.y = icon->image_y;
	    rect.width = icon->image_width + 2 * icon->image_bw;
	    rect.height = icon->image_height + 2 * icon->image_bw;

	    XShapeCombineRectangles(dpy, icon->w,
					ShapeClip,
					0, 0,
					&rect, 1,
					ShapeSet,
					Unsorted); 

	    rect.x -= icon->bw;
	    rect.y -= icon->bw;
	    rect.width += 2 * icon->bw;
	    rect.height += 2 * icon->bw;

	    XShapeCombineRectangles(dpy, icon->w,
					ShapeBounding,
					0, 0,
					&rect, 1,
					ShapeSet,
					Unsorted); 
	}

	/* a window to see the text through */
	if (icon->has_text) {
	    if (tmp_win -> squeeze_icon) {
		rect.x = icon->text_x;
		rect.y = icon->text_y;
		rect.width = icon->text_width;
		rect.height = icon->text_height;
	    } else {
		rect.x = 0;
		rect.y = icon->text_y;
		rect.width = icon->width;
		rect.height = icon->text_height;
	    }
	    XShapeCombineRectangles(dpy, icon->w,
					ShapeClip,
					0, 0,
					&rect, 1,
					ShapeUnion,
					Unsorted); 
	    rect.x -= icon->bw;
	    rect.y -= icon->bw;
	    rect.width += 2 * icon->bw;
	    rect.height += 2 * icon->bw;

	    XShapeCombineRectangles(dpy,
					icon->w,
					ShapeBounding,
					0, 0,
					&rect, 1,
					ShapeUnion,
					Unsorted); 
	}
    }
#endif /* SHAPE */
}

/*
 * PlaceIconWindows figures out where to put it.
 */
void
PlaceIconWindows(tmp_win, def_x, def_y)
TwmWindow *tmp_win;
int def_x, def_y;
{
    int final_x, final_y;
    TwmIcon *icon = &(tmp_win->icon);

    if (tmp_win->wmhints && tmp_win->wmhints->flags & IconPositionHint) {
	final_x = tmp_win->wmhints->icon_x;
	final_y = tmp_win->wmhints->icon_y;
    } else
	PlaceIcon(tmp_win, def_x, def_y, &final_x, &final_y);

    if (tmp_win->root == Scr->Root) {
	if (final_x > Scr->MyDisplayWidth)
	    final_x = Scr->MyDisplayWidth - icon->width -
			(2 * Scr->IconBorderWidth);

	if (final_y > Scr->MyDisplayHeight)
	    final_y = Scr->MyDisplayHeight - icon->height -
			Scr->IconFont.height - 4 - (2 * Scr->IconBorderWidth);
    }

    MoveIcon(tmp_win, final_x, final_y);
}

/*
 * Finally, here is the top level function.
 */
void
CreateIconWindow(tmp_win, def_x, def_y)
TwmWindow *tmp_win;
int def_x, def_y;
{

    CreateIconWindows(tmp_win);
    ConfigureIconWindows(tmp_win);
    ShapeIconWindows(tmp_win);
    PlaceIconWindows(tmp_win, def_x, def_y);

    tmp_win->iconified = TRUE;

    XMapSubwindows(dpy, tmp_win->icon.w);
}

void
DestroyIconWindow(tmp_win)
TwmWindow *tmp_win;
{
    TwmIcon *icon = &(tmp_win->icon);

    IconDown(tmp_win);			/* remove from icon manager */

    if (tmp_win->icon.has_text) {
	XDeleteContext(dpy, tmp_win->icon.text_w, TwmContext);
	XDeleteContext(dpy, tmp_win->icon.text_w, ScreenContext);
	XDestroyWindow(dpy, tmp_win->icon.text_w);
    }

    if (tmp_win->icon.has_image && icon->w != icon->image_w) {
	XDeleteContext(dpy, tmp_win->icon.image_w, TwmContext);
	XDeleteContext(dpy, tmp_win->icon.image_w, ScreenContext);
	if (tmp_win->icon.image_not_ours) {
	    XReparentWindow(dpy, tmp_win->icon.image_w, tmp_win->root, 0, 0);
	    XUnmapWindow(dpy, tmp_win->icon.image_w);
	} else
	    XDestroyWindow(dpy, tmp_win->icon.image_w);
    }

    XDeleteContext(dpy, tmp_win->icon.w, TwmContext);
    XDeleteContext(dpy, tmp_win->icon.w, ScreenContext);
    if (icon->w == icon->image_w && tmp_win->icon.image_not_ours) {
	XReparentWindow(dpy, tmp_win->icon.image_w, tmp_win->root, 0, 0);
	XUnmapWindow(dpy, tmp_win->icon.image_w);
    } else
	XDestroyWindow(dpy, tmp_win->icon.w);

    tmp_win->icon.w = None;
    tmp_win->icon.image_w = None;
    tmp_win->icon.text_w = None;

    tmp_win->icon.has_image = FALSE;
    tmp_win->icon.has_text = FALSE;

    tmp_win->icon.text = NULL;
    tmp_win->icon.mask = None;
    tmp_win->icon.clip = None;
}

#ifdef ADD_BORDER_TO_MASK

/*
 * Code to make a clip mask which is bigger than the
 * shape mask for an image.
 */

#define ERR_RETURN 0
static char *Format_image(image, resultsize) /* Stolen from lib/X/XWrBitF.c */
XImage *image;
int *resultsize;
{
    register int x, c, b;
    register char *ptr;
    int y;
    char *data;
    int width, height;
    int bytes_per_line;
    
    width = image->width;
    height = image->height;
    
    bytes_per_line = (width+7)/8;
    *resultsize = bytes_per_line * height;    /* Calculate size of data */

    data = (char *) malloc( *resultsize );     /* Get space for data */
    if (!data)
      return(ERR_RETURN);

    /*
     * The slow but robust brute force method of converting the image:
     */
    ptr = data;
    c = 0; b=1;
    for (y=0; y<height; y++) {
	for (x=0; x<width;) {
	    if (XGetPixel(image, x, y))
	      c |= b;
	    b <<= 1;
	    if (!(++x & 7)) {
		*(ptr++)=c;
		c=0; b=1;
	    }
	}
	if (x & 7) {
	    *(ptr++)=c;
	    c=0; b=1;
	}
    }
    return(data);
}

#define tst_bit(i, j, w, b) (b[(i+j*(w*8))/8] &  (1 << ((i+j*(w*8)) % 8)))
#define set_bit(i, j, w, b) (b[(i+j*(w*8))/8] |= (1 << ((i+j*(w*8)) % 8)))

Pixmap
MaskToClip(bm)
Pixmap bm;
{
    Pixmap bmmask;
    unsigned int height, width;
    int size;
    XImage *image;
    char *dataold, *datanew;
    int i, j, ii, jj, w, border;

    border = Scr->IconBorderWidth;

    XGetGeometry(dpy, bm, &JunkRoot, &JunkX, &JunkY,
		(unsigned int *)&width, (unsigned int *)&height,
		&JunkBW, &JunkDepth);
    image = XGetImage(dpy, bm, 0,0, width, height, 1L, XYPixmap);
    dataold = Format_image(image, &size);
    datanew = (char *) malloc(size);
    for (i=0; i<size; i++)
	datanew[i] = 0;
    XDestroyImage(image);
    w = (width+7)/8;
    for (j = 0; j < height; j++) {
	for (i = 0; i < 8*w; i++) {
	    if (!tst_bit(i, j, w, dataold)) continue;
	    for (jj = j-border; jj <= j+border; jj++) {
		for ( ii = i-border; ii <= i+border; ii++) {
		    if (jj < 0         || ii < 0        ||
			jj >= height   || ii >= w*8) continue;
		    set_bit(ii, jj, w, datanew);
		}
	    }
	}
    }
    bmmask = XCreateBitmapFromData(dpy, Scr->Root, 
				   (char *)datanew, width, height);
    free(dataold);
    free(datanew);
    return (bmmask);
}

#endif  /* ADD_BORDER_TO_MASK */
