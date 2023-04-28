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

/**********************************************************************
 *
 * $XConsortium: icons.h,v 1.4 89/07/18 17:16:24 jim Exp $
 *
 * Icon releated definitions
 *
 * 10-Apr-89 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef ICONS_H
#define ICONS_H

typedef struct IconRegion
{
    struct IconRegion	*next;
    char		*name;
    int			x, y, w, h;
    int			grav1, grav2;
    int			stepx, stepy;	/* allocation granularity */
    struct IconEntry	*entries;
} IconRegion;

typedef struct IconEntry
{
    struct IconEntry	*next;
    int			x, y, w, h;
    struct TwmWindow	*twm_win;
    short 		used;
}IconEntry;

/*
 * Structure describing an icon.
 */
typedef struct TwmIcon {
    /* flags saying what is what. */
    unsigned int
	has_image:1,
	has_text:1,
	image_not_ours:1; 

    /* The windows making up an icon */
    Window	w;		/* The main icon window */
    unsigned int width, height, bw;
    Window	image_w;	/* The picture */
    int		image_x, image_y;
    unsigned int image_width, image_height, image_bw;
    Window	text_w;		/* The text */
    int		text_x, text_y;
    unsigned int text_width, text_height, text_bw;

    /* now information needed to redraw those windows */
    char	*text;		/* text of the label */
    Pixmap	mask;		/* mask to shape image */
    Pixmap	clip;		/* mask to shape image+border */
} TwmIcon;

/* These are used as margins around the text */

#define ICON_TEXT_VBORDER 1
#define ICON_TEXT_HBORDER 2

#endif /* ICONS_H */
