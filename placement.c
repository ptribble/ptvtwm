
 /********************************************************************\
 *                                                                    *
 * Code to do window placement by searching fo a good position. It    *
 * evaluates positions by looking at their weighted intersection      *
 * with other windows.                                                *
 *                                                                    *
 * In all of the following code, variables with names ending in two   *
 * digits are used to signify values atthe corner of rectangles as    *
 * follows...                                                         *
 *                                                                    *
 *                 x00-------------x10                                *
 *                  |               |                                 *
 *                 x01-------------x11                                *
 *                                                                    *
 \********************************************************************/

#include <stdio.h>
#include "twm.h"
#include "screen.h"
#include "icons.h"
#include "list.h"
#include "parse.h"

/* #define DEBUG_PLACEMENT */

/* some interpolation macros, doing this with macros isn't nice
   because it causes lots of repeated expressions, but looks clean. 
   Good compilers will spot this.
   Maybe I should rewrite these. Of course inline functions would be best.
   */
   
/* interpolate weight on a line */
#define interpolate_line(d, dmax, w0, w1) \
    		(((double)d)/(dmax) * ((w1)-(w0)) + (w0))

/* interpolate weight on a plane */
#define interpolate_plane(x,y,xmax,ymax,w00,w10,w01,w11) \
    interpolate_line((x), (xmax), \
		     interpolate_line((y), (ymax), (w00), (w01)), \
		     interpolate_line((y), (ymax), (w10), (w11)))

/* I think this is the integral. */

#define integrate_weights(xmin, ymin, xmax, ymax, w00, w10, w01, w11) \
    (((xmax)-(xmin))*((ymax)-(ymin))*((w00)+(w01)+(w10)+(w11))/4.0)

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

typedef struct 
    {
    int x, y, width, height;
    } Rectangle;

 /********************************************************************\
 *                                                                    *
 * Procedure OverlapWeight                                            *
 *                                                                    *
 * Inputs:                                                            *
 *         lower   - the lower rectangle                              *
 *         lw??    - weights on corners of lower                      *
 *         upper   - the upper rectangle                              *
 *                                                                    *
 * Returns:                                                           *
 *         The integral of the weight on the intersection of the      *
 *                 two rectangles using the above interpolation       *
 *                 function.                                          *
 *                                                                    *
 \********************************************************************/

	
static double
OverlapWeight(lower, lw00, lw10, lw01, lw11, upper)

Rectangle lower;
double lw00, lw01, lw10, lw11;	/* weights for the corners of lower */
Rectangle upper;

{
int xmin, xmax, ymin, ymax;
double cw00, cw01, cw10, cw11;

/* Calculate clip rectangle */

xmin = min(max(lower.x, upper.x),lower.x + lower.width);
xmax = max(min(lower.x+lower.width, upper.x+upper.width), lower.x);
ymin = min(max(lower.y, upper.y),lower.y + lower.height);
ymax = max(min(lower.y+lower.height, upper.y+upper.height), lower.y);

#if DEBUG_POSITIONING > 1
printf("lower=%d, %d, %d, %d\n", lower.x, lower.x+lower.width, lower.y, lower.y+lower.height);
printf("upper=%d, %d, %d, %d\n", upper.x, upper.x+upper.width, upper.y, upper.y+upper.height);
printf("inter=%d, %d, %d, %d\n", xmin, xmax, ymin, ymax);
#endif

if (xmin==xmax || ymin==ymax)
    return 0.0;

/* Calculate clip rectangle corner weights, 
   this could be enormously optimised by using interpolate_line
   for the cases where a point is on an edge of `lower'*/

cw00=interpolate_plane(xmin-lower.x, ymin-lower.y,
		       lower.width, lower.height,
		       lw00, lw10, lw01, lw11);
cw10=interpolate_plane(xmax-lower.x, ymin-lower.y,
		       lower.width, lower.height,
		       lw00, lw10, lw01, lw11);
cw01=interpolate_plane(xmin-lower.x, ymax-lower.y,
		       lower.width, lower.height,
		       lw00, lw10, lw01, lw11);
cw11=interpolate_plane(xmax-lower.x, ymax-lower.y,
		       lower.width, lower.height,
		       lw00, lw10, lw01, lw11);

/* Finally do the integration over the surface.
   I think I have the calculus right. */

return integrate_weights(xmin, ymin, xmax, ymax, cw00, cw10, cw01, cw11);
}

 /********************************************************************\
 *                                                                    *
 * Procedure EvaluatePosition - See how good a position is.           *
 *                                                                    *
 *  Inputs:                                                           *
 *         x, y, w, h      - the suggested position (screen           *
 *                           coordiantes)                             *
 *                                                                    *
 *  Returns:                                                          *
 *         A total weight for that position.                          *
 *                                                                    *
 * This should really only count the currently visable parts of the   *
 * windows etc.                                                       *
 *                                                                    *
 \********************************************************************/

static double
EvaluatePosition(x, y, width, height, weighting)


int x, y, width, height;
WindowWeighting *weighting;

{
double weight;
Rectangle rect, window_rect;
TwmWindow *w;
IconRegion *ir;
name_list *p;
int i, j, ii, jj, istep, jstep;
WindowWeighting *ww;

weight= 0.0;

/* The major part of the weight is to sum the weighted intersections
   for all of the open windows. */

for (w = Scr->TwmRoot.next; w != NULL; w=w->next)
    if (w->mapped)		/* New window is not mapped yet */
	{
#ifdef DEBUG_PLACEMENT
	printf("\t%s(0x%x) mapped", w->name, w->w);
#endif
	if ( w->sticky && Scr->StickyAbove)
	    {
	    /* Sticky above windows will obscure new window, so clip
	       new window's weights against sticky window */

	    rect.x=w->frame_x;
	    rect.y=w->frame_y;
	    rect.width=w->frame_width;
	    rect.height=w->frame_height;

	    if ( w->root == Scr->VirtualDesktop)
		{
		rect.x -= Scr->vdtPositionX;
		rect.y -= Scr->vdtPositionY;
		}

	    for(i=0;i<3;i++)
		for(j=0;j<3;j++)
		    {
		    window_rect.x=x+i*width/3.0;
		    window_rect.y=y+j*height/3.0;
		    window_rect.width=width/3.0;
		    window_rect.height=height/3.0;

		    weight += OverlapWeight(window_rect, 
					    weighting->weights[i][j],
					    weighting->weights[i+1][j],
					    weighting->weights[i][j+1],
					    weighting->weights[i+1][j+1],
					    rect);
		    }
	    }
	else 
	    {
	    /* Other windows will be (at first at least) obscured by 
	       the new one, so clip their weights against it */

	    if (w->sticky)
		ww= w->StickyWeighting;
	    else
		ww= w->Weighting;

	    window_rect.x=x;
	    window_rect.y=y;
	    window_rect.width=width;
	    window_rect.height=height;

	    for(i=0;i<3;i++)
		for(j=0;j<3;j++)
		    {
		    rect.x=w->frame_x + i*w->frame_width/3.0;
		    rect.y=w->frame_y + j*w->frame_height/3.0;
		    rect.width=w->frame_width/3.0;
		    rect.height=w->frame_height/3.0;

		    if ( w->root == Scr->VirtualDesktop)
			{
			rect.x -= Scr->vdtPositionX;
			rect.y -= Scr->vdtPositionY;
			}

		    weight += OverlapWeight(rect, 
					    ww->weights[i][j],
					    ww->weights[i+1][j],
					    ww->weights[i][j+1],
					    ww->weights[i+1][j+1],
					    window_rect);
		    }
	    }
#ifdef DEBUG_PLACEMENT
	printf(" weight=%f\n", weight);
#endif
	}

/* Icon Regions obscured. the weights for icon regions are rotated accordin
   to the gravity */

window_rect.x=x;
window_rect.y=y;
window_rect.width=width;
window_rect.height=height;

for(p=Scr->IconRegions; p!=NULL; p=next_entry(p))
    {
    ir=(IconRegion *)contents_of_entry(p);
#ifdef DEBUG_PLACEMENT
    printf("\tRegion %s ", ir->name);
#endif
    if (ir->grav1==D_EAST || ir->grav2==D_EAST)
	{
	ii=3;
	istep=-1;
	}
    else
	{
	ii=0;
	istep=1;
	}

    for(i=0;i<3;i++)
	{
	
	if (ir->grav1==D_SOUTH || ir->grav2==D_SOUTH)
	    {
	    jj=3;
	    jstep=-1;
	    }
	else
	    {
	    jj=0;
	    jstep=1;
	    }

	for(j=0;j<3;j++)
	    {
	    rect.x=ir->x+i*ir->w/3.0;
	    rect.y=ir->y+j*ir->h/3.0;
	    rect.width=ir->w/3.0;
	    rect.height=ir->h/3.0;

	    if ( Scr->VirtualDesktop != None)
		{
		rect.x -= Scr->vdtPositionX;
		rect.y -= Scr->vdtPositionY;
		}

	    weight += OverlapWeight(rect, 
				    Scr->IconRegionWindowWeighting.weights[ii][jj],
				    Scr->IconRegionWindowWeighting.weights[ii+istep][jj],
				    Scr->IconRegionWindowWeighting.weights[ii][jj+jstep],
				    Scr->IconRegionWindowWeighting.weights[ii+istep][jj+jstep],
				    window_rect);
	    jj += jstep;
	    }
	ii+=istep;
	}
#ifdef DEBUG_PLACEMENT
    printf(" weight=%f\n", weight);
#endif
    }

/* now add in a factor for the screen area consumed */

for(i=0;i<3;i++)
    for(j=0;j<3;j++)
	{
	rect.x= i*Scr->MyDisplayWidth/3.0;
	rect.y= j*Scr->MyDisplayHeight/3.0;
	rect.width=Scr->MyDisplayWidth/3.0;
	rect.height=Scr->MyDisplayHeight/3.0;

	weight += OverlapWeight(rect, 
				Scr->ScreenWindowWeighting.weights[i][j],
				Scr->ScreenWindowWeighting.weights[i+1][j],
				Scr->ScreenWindowWeighting.weights[i][j+1],
				Scr->ScreenWindowWeighting.weights[i+1][j+1],
				window_rect);
	}

/* finally a factor for how much of the new window would be off screen.
   This is calculated as the weight for the entire window minus the weight 
   for the bit on screen. */

rect.x=rect.y=0;
rect.width=Scr->MyDisplayWidth;
rect.height=Scr->MyDisplayHeight;

for(i=0;i<3;i++)
    for(j=0;j<3;j++)
	{
	window_rect.x=x+i*width/3.0;
	window_rect.y=y+j*height/3.0;
	window_rect.width=width/3.0;
	window_rect.height=height/3.0;

	weight += integrate_weights(window_rect.x,window_rect.y,
				    window_rect.x+window_rect.width,
				    window_rect.y+window_rect.height,
				    weighting->weights[i][j],
				    weighting->weights[i+1][j],
				    weighting->weights[i][j+1],
				    weighting->weights[i+1][j+1])
	    - OverlapWeight(window_rect, 
			    weighting->weights[i][j],
			    weighting->weights[i+1][j],
			    weighting->weights[i][j+1],
			    weighting->weights[i+1][j+1],
			    rect);
	}

/* that's all folks */

return weight;
}
	
 /********************************************************************\
 *                                                                    *
 *  Procedure:                                                        *
 *         FindPlace - Find a place for the window which will         *
 *                         minimalise overlap.                        *
 *                                                                    *
 *  Inputs:                                                           *
 *         tmp_win   - Window structure                               *
 *                                                                    *
 *  Results:                                                          *
 *         xp, yp    - returned screen coordinates for top left       *
 *                         hand corner.                               *
 *                                                                    *
 * This uses a simple 8-direction hill climbing algorithm.            *
 *                                                                    *
 \********************************************************************/

void
FindPlace(tmp_win, xp, yp)

TwmWindow *tmp_win;
int *xp, *yp;

{
int x, y, xstep, ystep, width, height;
int best_x, best_y, tx, ty;
double best_weight, last_best, weight;
WindowWeighting *ww;

if (tmp_win->sticky)
    ww=tmp_win->StickyWeighting;
else
    ww=tmp_win->Weighting;

width=tmp_win->frame_width;
height=tmp_win->frame_height;

/* start in the middle of the screen */

x= Scr->MyDisplayWidth/2;
y= Scr->MyDisplayHeight/2;
xstep = x/2;
ystep = y/2;

last_best=best_weight=EvaluatePosition(x-width/2, y-height/2, width, height, ww);
best_x=x;
best_y=y;

if (Scr->WatchPlacement)
    {
    MoveOutline(Scr->Root, x-width/2, y-height/2, width, height, 1,1);
    XFlush(dpy);
    }

while(xstep >0 || ystep >0)
    {
    for(ty=y-ystep; ty<=y+ystep; ty+=ystep)
	{
	for(tx=x-xstep; tx<=x+xstep; tx+=xstep)
	    {
#ifdef DEBUG_PLACEMENT
	    printf("(%d, %d) ->", tx, ty);
#endif
	    if (tx!=x || ty!=y)
		{
		if (Scr->WatchPlacement)
		    {
		    MoveOutline(Scr->Root, tx-width/2, ty-height/2, width, height, 1,1);
		    XFlush(dpy);
		    }
		weight=EvaluatePosition(tx-width/2, ty-height/2, width, height, ww);
#ifdef DEBUG_PLACEMENT
		printf("= %5.2f", weight);
#endif
		if (weight<best_weight)
		    {
		    best_x=tx;
		    best_y=ty;
		    best_weight=weight;
		    }
		}
	    else
		{
#ifdef DEBUG_PLACEMENT
		printf("still = %5.2f", last_best);
#endif
		}
	    if ( xstep == 0 )
		break;
	    }
#ifdef DEBUG_PLACEMENT
	putchar('\n');
#endif
	if ( ystep == 0 )
	    break;
	}

#ifdef DEBUG_PLACEMENT
    printf("\nbest=%5.2f @ %d, %d\n\n", best_weight, best_x, best_y);
    getchar();
#endif
    x=best_x;
    y=best_y;
    last_best=best_weight;
    xstep /=2;
    ystep /=2;
    }

if (Scr->WatchPlacement)
    {
    MoveOutline(Scr->Root, 0,0,0,0,0,0);
    XFlush(dpy);
    }

*xp=x-width/2;
*yp=y-height/2+tmp_win->title_height;
}

