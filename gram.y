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
 * $XConsortium: gram.y,v 1.91 91/02/08 18:21:56 dave Exp $
 *
 * .twmrc command grammer
 *
 * 07-Jan-86 Thomas E. LaStrange	File created
 * 11-Nov-90 Dave Sternlicht            Adding SaveColors
 * 10-Oct-90 David M. Sternlicht        Storing saved colors on root
 *
 ***********************************************************************/

%{
#ifndef YYBISON			/* bison includes stdio.h automatically */
#include <stdio.h>
#endif
#include <ctype.h>
#include "twm.h"
#include "menus.h"
#include "list.h"
#include "util.h"
#include "screen.h"
#include "parse.h"
#include <X11/Xos.h>
#include <X11/Xmu/CharSet.h>

#define PI 3.1415926535897932
#define TWO_PI 6.2831853071795865
#define DEG_TO_RAD(d) (((float)(d) * TWO_PI) / 360.0)

static char *Action = "";
static char *Name = "";
static MenuRoot	*root, *pull = NULL;
static int  menu_default_index;

static MenuRoot *GetRoot();

static Bool CheckWarpScreenArg(), CheckWarpRingArg();
static Bool CheckColormapArg();
static void GotButton(), GotKey(), GotTitleButton();
#ifdef RJC
extern char *CacheAndNameIconImage();
#endif

static char *ptr;
static name_list **list;
static int cont = 0;
static int color;
int mods = 0;
char	*menu_name = NULL;		/* if we are reading a menu */
MenuRoot   *input_menu;

unsigned int mods_used = (ShiftMask | ControlMask | Mod1Mask);

extern int do_single_keyword(), do_string_keyword(), do_number_keyword();
extern name_list **do_colorlist_keyword();
extern int do_color_keyword(), do_string_savecolor();
extern int yylineno;
%}

%union
{
    int num;
    char *ptr;
    struct {
	short  ltype;
	char  *lval;
    } match;
    struct {
	char  *gstring;
	short itterate;
    } geom;
};

%token <num> LB RB LP RP AT MENUS MENU PIEMENU BUTTON DEFAULT_FUNCTION PLUS MINUS
%token <num> ASTERISK GREATER
%token <num> ALL OR CURSORS PIXMAPS ICONS COLOR SAVECOLOR MONOCHROME FUNCTION 
%token <num> ICONMGR_SHOW ICONMGR WINDOW_FUNCTION ZOOM ICONMGRS
%token <num> ICONMGR_GEOMETRY ICONMGR_NOSHOW MAKE_TITLE
%token <num> ICONIFY_BY_UNMAPPING DONT_ICONIFY_BY_UNMAPPING STICKY
%token <num> NO_TITLE AUTO_RAISE NO_HILITE ICON_REGION 
%token <num> META SHIFT LOCK CONTROL WINDOW TITLE ICON ROOT FRAME 
%token <num> COLON EQUALS TILDE SQUEEZE_TITLE DONT_SQUEEZE_TITLE
%token <num> SQUEEZE_ICON DONT_SQUEEZE_ICON
%token <num> START_ICONIFIED NO_TITLE_HILITE TITLE_HILITE
%token <num> MOVE RESIZE WAIT_CURS SELECT KILL LEFT_TITLEBUTTON RIGHT_TITLEBUTTON 
%token <num> NUMBER KEYWORD MKEYWORD NKEYWORD CKEYWORD CLKEYWORD FKEYWORD
%token <num> FSKEYWORD SKEYWORD DKEYWORD JKEYWORD PKEYWORD WINDOW_RING
%token <num> WARP_CURSOR ERRORTOKEN NO_STACKMODE ICON_TITLE NO_ICON_TITLE
%token <ptr> STRING REGEXP

%type <ptr> string regexp
%type <geom> geometry
%type <match> matcher
%type <num> action button number signed_number full fullkey def_indicator

%start file 

/*
 * Notice that this grammar reads tvtwmrc files *and* menu files.  It would
 * make more sense to have two separate parsers, but it is not possible
 * to have two yacc grammars in one program without non portable hackery
 * (so far as I know).
 */

%%
file		: twmrc { input_menu = NULL; }
		| menufile { input_menu = root; }
		| menutreefile {input_menu = GetRoot(menu_name, NULL, NULL); }
		;

menufile	: { root = GetRoot(menu_name, NULL, NULL); }
		  menu_body
		;

menutreefile	: menu_definition
		| menutreefile menu_definition
		;

twmrc		: /* Empty */
		| stmts
		;

stmts		: stmt
		| stmts stmt
		| stmts menu_definition
		;

stmt		: error	{ printf("error in statements\n"); }
		| noarg
		| sarg
		| narg
		| squeeze
		| ICON_REGION geometry DKEYWORD DKEYWORD number number
					{ AddIconRegion("", LTYPE_ANYTHING,
							$2.gstring, $2.itterate,
							$3, $4, $5, $6); }
		| ICON_REGION matcher geometry DKEYWORD DKEYWORD number number
					{ AddIconRegion($2.lval, $2.ltype,
							$3.gstring, $3.itterate,
							$4, $5, $6, $7); }
		| ICONMGR_GEOMETRY string number	{ if (Scr->FirstTime)
						  {
						    Scr->iconmgr.geometry=$2;
						    Scr->iconmgr.columns=$3;
						  }
						}
		| ICONMGR_GEOMETRY string	{ if (Scr->FirstTime)
						    Scr->iconmgr.geometry = $2;
						}
		| ZOOM number		{ if (Scr->FirstTime)
					  {
						Scr->DoZoom = TRUE;
						Scr->ZoomCount = $2;
					  }
					}
		| ZOOM			{ if (Scr->FirstTime) 
						Scr->DoZoom = TRUE; }
		| PIXMAPS pixmap_list	{}
		| CURSORS cursor_list	{}
		| ICONIFY_BY_UNMAPPING	{ list = &Scr->IconifyByUn; }
		  win_list
		| ICONIFY_BY_UNMAPPING	{ if (Scr->FirstTime) 
		    Scr->IconifyByUnmapping = TRUE; }
		| LEFT_TITLEBUTTON string EQUALS action { 
					  GotTitleButton ($2, $4, False);
					}
		| RIGHT_TITLEBUTTON string EQUALS action { 
					  GotTitleButton ($2, $4, True);
					}
		| button string		{ root = GetRoot($2, NULLSTR, NULLSTR);
					  Scr->Mouse[$1][C_ROOT][0].func = F_MENU;
					  Scr->Mouse[$1][C_ROOT][0].menu = root;
					}
		| button action		{ Scr->Mouse[$1][C_ROOT][0].func = $2;
					  if ($2 == F_MENU || $2 == F_PIEMENU)
					  {
					    pull->prev = NULL;
					    Scr->Mouse[$1][C_ROOT][0].menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT,NULLSTR,NULLSTR);
					    Scr->Mouse[$1][C_ROOT][0].item = 
						AddToMenu(root,"x",Action,
							  NULLSTR,$2,NULLSTR,NULLSTR);
					  }
					  Action = "";
					  pull = NULL;
					}
		| string fullkey	{ GotKey($1, $2); }
		| button full		{ GotButton($1, $2); }
		| DONT_ICONIFY_BY_UNMAPPING { list = &Scr->DontIconify; }
		  win_list
		| ICONMGR_NOSHOW	{ list = &Scr->IconMgrNoShow; }
		  win_list
		| ICONMGR_NOSHOW	{ Scr->IconManagerDontShow = TRUE; }
		| ICONMGRS		{ list = &Scr->IconMgrs; }
		  iconm_list
		| ICONMGR_SHOW		{ list = &Scr->IconMgrShow; }
		  win_list
		| NO_TITLE_HILITE	{ list = &Scr->NoTitleHighlight; }
		  win_list
		| NO_TITLE_HILITE	{ if (Scr->FirstTime)
						Scr->TitleHighlight = FALSE; }
		| NO_HILITE		{ list = &Scr->NoHighlight; }
		  win_list
		| NO_HILITE		{ if (Scr->FirstTime)
						Scr->Highlight = FALSE; }
		| NO_STACKMODE		{ list = &Scr->NoStackModeL; }
		  win_list
		| NO_STACKMODE		{ if (Scr->FirstTime)
						Scr->StackMode = FALSE; }
		| NO_TITLE		{ list = &Scr->NoTitle; }
		  win_list
		| NO_TITLE		{ if (Scr->FirstTime)
						Scr->NoTitlebar = TRUE; }
		| NO_ICON_TITLE		{ list = &Scr->NoIconTitleL; }
		  win_list
		| NO_ICON_TITLE		{ if (Scr->FirstTime)
						Scr->NoIconTitle = TRUE; }
		| ICON_TITLE		{ list = &Scr->IconTitleL; }
		  win_list
		| MAKE_TITLE		{ list = &Scr->MakeTitle; }
		  win_list
		| START_ICONIFIED	{ list = &Scr->StartIconified; }
		  win_list
		| AUTO_RAISE		{ list = &Scr->AutoRaise; }
		  win_list
		| STICKY		{ list = &Scr->StickyL; }
		  win_list
		| FUNCTION string	{ root = GetRoot($2, NULLSTR, NULLSTR); }
		  function
		| ICONS 		{ list = &Scr->IconNames; }
		  icon_list
		| COLOR 		{ color = COLOR; }
		  color_list
		| SAVECOLOR		{ }
		  save_color_list
		| MONOCHROME 		{ color = MONOCHROME; }
		  color_list
		| DEFAULT_FUNCTION action { Scr->DefaultFunction.func = $2;
					  if ($2 == F_MENU || $2 == F_PIEMENU)
					  {
					    pull->prev = NULL;
					    Scr->DefaultFunction.menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT,NULLSTR,NULLSTR);
					    Scr->DefaultFunction.item = 
						AddToMenu(root,"x",Action,
							  NULLSTR,$2, NULLSTR, NULLSTR);
					  }
					  Action = "";
					  pull = NULL;
					}
		| WINDOW_FUNCTION action { Scr->WindowFunction.func = $2;
					   root = GetRoot(TWM_ROOT,NULLSTR,NULLSTR);
					   Scr->WindowFunction.item = 
						AddToMenu(root,"x",Action,
							  NULLSTR,$2, NULLSTR, NULLSTR);
					   Action = "";
					   pull = NULL;
					}
		| WARP_CURSOR		{ list = &Scr->WarpCursorL; }
		  win_list
		| WARP_CURSOR		{ if (Scr->FirstTime) 
					    Scr->WarpCursor = TRUE; }
		| WINDOW_RING		{ list = &Scr->WindowRingL; }
		  win_list
		;


noarg		: KEYWORD		{ if (!do_single_keyword ($1)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
					"unknown singleton keyword %d\n",
						     $1);
					    ParseError = 1;
					  }
					}
		;

sarg		: SKEYWORD string	{ if (!do_string_keyword ($1, $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		| SKEYWORD string COLON string {
#ifdef RJC
#define kws_UnknownIcon	7
			if ($1 != kws_UnknownIcon)
			    yyerror("syntax error");
			else if (Scr->FirstTime) {
			    char *full_name=CacheAndNameIconImage($2, $4);
			    if (full_name != NULL) {
				if (!do_string_keyword ($1, full_name)) {
				    twmrc_error_prefix();
				    fprintf (stderr,
			"unknown string keyword %d (value \"%s\")\n",
						$1, full_name);
				    ParseError = 1;
				}
			    } else {
				twmrc_error_prefix();
				fprintf(stderr, "can't access bitmap and mask %s, %s\n",
				$2, $4);
			    }
			}
#else
			yyerror("\"pixmap\" : \"mask\" syntax no longer supported");
#endif
			}
		;

narg		: NKEYWORD number	{ if (!do_number_keyword ($1, $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown numeric keyword %d (value %d)\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		;



full		: EQUALS keys COLON contexts COLON action  { $$ = $6; }
		;

fullkey		: EQUALS keys COLON contextkeys COLON action  { $$ = $6; }
		;

keys		: /* Empty */
		| keys key
		;

key		: META			{ mods |= Mod1Mask; }
		| SHIFT			{ mods |= ShiftMask; }
		| LOCK			{ mods |= LockMask; }
		| CONTROL		{ mods |= ControlMask; }
		| META number		{ if ($2 < 1 || $2 > 5) {
					     twmrc_error_prefix();
					     fprintf (stderr, 
				"bad modifier number (%d), must be 1-5\n",
						      $2);
					     ParseError = 1;
					  } else {
					     mods |= (Mod1Mask << ($2 - 1));
					  }
					}
		| OR			{ }
		;

contexts	: /* Empty */
		| contexts context
		;

context		: WINDOW		{ cont |= C_WINDOW_BIT; }
		| TITLE			{ cont |= C_TITLE_BIT; }
		| ICON			{ cont |= C_ICON_BIT; }
		| ROOT			{ cont |= C_ROOT_BIT; }
		| FRAME			{ cont |= C_FRAME_BIT; }
		| ICONMGR		{ cont |= C_ICONMGR_BIT; }
		| META			{ cont |= C_ICONMGR_BIT; }
		| ALL			{ cont |= C_ALL_BITS; }
		| OR			{  }
		;

contextkeys	: /* Empty */
		| contextkeys contextkey
		;

contextkey	: WINDOW		{ cont |= C_WINDOW_BIT; }
		| TITLE			{ cont |= C_TITLE_BIT; }
		| ICON			{ cont |= C_ICON_BIT; }
		| ROOT			{ cont |= C_ROOT_BIT; }
		| FRAME			{ cont |= C_FRAME_BIT; }
		| ICONMGR		{ cont |= C_ICONMGR_BIT; }
		| META			{ cont |= C_ICONMGR_BIT; }
		| ALL			{ cont |= C_ALL_BITS; }
		| OR			{ }
		| string		{ Name = $1; cont |= C_NAME_BIT; }
		;


pixmap_list	: LB pixmap_entries RB
		{ }
		;

pixmap_entries	: /* Empty */
		| pixmap_entries pixmap_entry
		;

pixmap_entry	:  PKEYWORD string { do_pixmap_keyword($1,$2); }
		;


cursor_list	: LB cursor_entries RB
		{ }
		;

cursor_entries	: /* Empty */
		| cursor_entries cursor_entry
		;

cursor_entry	: FRAME string string {
			NewBitmapCursor(&Scr->FrameCursor, $2, $3); }
		| FRAME string	{
			NewFontCursor(&Scr->FrameCursor, $2); }
		| TITLE string string {
			NewBitmapCursor(&Scr->TitleCursor, $2, $3); }
		| TITLE string {
			NewFontCursor(&Scr->TitleCursor, $2); }
		| ICON string string {
			NewBitmapCursor(&Scr->IconCursor, $2, $3); }
		| ICON string {
			NewFontCursor(&Scr->IconCursor, $2); }
		| ICONMGR string string {
			NewBitmapCursor(&Scr->IconMgrCursor, $2, $3); }
		| ICONMGR string {
			NewFontCursor(&Scr->IconMgrCursor, $2); }
		| BUTTON string string {
			NewBitmapCursor(&Scr->ButtonCursor, $2, $3); }
		| BUTTON string {
			NewFontCursor(&Scr->ButtonCursor, $2); }
		| MOVE string string {
			NewBitmapCursor(&Scr->MoveCursor, $2, $3); }
		| MOVE string {
			NewFontCursor(&Scr->MoveCursor, $2); }
		| RESIZE string string {
			NewBitmapCursor(&Scr->ResizeCursor, $2, $3); }
		| RESIZE string {
			NewFontCursor(&Scr->ResizeCursor, $2); }
		| WAIT_CURS string string {
			NewBitmapCursor(&Scr->WaitCursor, $2, $3); }
		| WAIT_CURS string {
			NewFontCursor(&Scr->WaitCursor, $2); }
		| MENU string string {
			NewBitmapCursor(&Scr->MenuCursor, $2, $3); }
		| MENU string {
			NewFontCursor(&Scr->MenuCursor, $2); }
		| SELECT string string {
			NewBitmapCursor(&Scr->SelectCursor, $2, $3); }
		| SELECT string {
			NewFontCursor(&Scr->SelectCursor, $2); }
		| KILL string string {
			NewBitmapCursor(&Scr->DestroyCursor, $2, $3); }
		| KILL string {
			NewFontCursor(&Scr->DestroyCursor, $2); }
		;

color_list	: LB color_entries RB
		{ }
		;


color_entries	: /* Empty */
		| color_entries color_entry
		;

color_entry	: CLKEYWORD string	{ if (!do_colorlist_keyword ($1, color,
								     $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled list color keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		| CLKEYWORD string	{ list = do_colorlist_keyword($1,color,
								      $2);
					  if (!list) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color list keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		  win_color_list
		| CKEYWORD string	{ if (!do_color_keyword ($1, color,
								 $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		;

save_color_list : LB s_color_entries RB 
		{ }
                ;

s_color_entries : /* Empty */
                | s_color_entries s_color_entry 
                ;

s_color_entry   : string            { do_string_savecolor(color, $1); }
                | CLKEYWORD         { do_var_savecolor($1); }
                ;

win_color_list	: LB win_color_entries RB
		{ }
		;

win_color_entries	: /* Empty */
		| win_color_entries win_color_entry
		;

win_color_entry	: matcher string	{ if (Scr->FirstTime &&
					      color == Scr->Monochrome)
					    AddToList(list, $1.lval, $1.ltype,
							$2); }
		;

squeeze		: SQUEEZE_TITLE { 
				    if (HasShape) Scr->SqueezeTitle = TRUE;
				}
		| SQUEEZE_TITLE { list = &Scr->SqueezeTitleL; }
		  LB win_sqz_entries RB
		| DONT_SQUEEZE_TITLE { Scr->SqueezeTitle = FALSE; }
		| DONT_SQUEEZE_TITLE { list = &Scr->DontSqueezeTitleL; }
		  win_list
		| SQUEEZE_ICON { if (HasShape) Scr->SqueezeIcon = TRUE; }
		| DONT_SQUEEZE_ICON { if (HasShape) Scr->SqueezeIcon = FALSE; }
		| SQUEEZE_ICON { list = &Scr->SqueezeIconL; }
		  win_list
		| DONT_SQUEEZE_ICON { list = &Scr->DontSqueezeIconL; }
		  win_list
		;

win_sqz_entries	: /* Empty */
		| win_sqz_entries matcher JKEYWORD signed_number number	{
				if (Scr->FirstTime) {
				   do_squeeze_entry (list, $2.lval, $2.ltype,
							$3, $4, $5);
				}
			}
		;


iconm_list	: LB iconm_entries RB
		{ }
		;

iconm_entries	: /* Empty */
		| iconm_entries iconm_entry
		;

iconm_entry	: matcher string number	{ if (Scr->FirstTime)
					    AddToList(list, $1.lval, $1.ltype, (char *)
						AllocateIconManager($1.lval, NULLSTR,
							$2, $3));
					}
		| matcher string string number
					{ if (Scr->FirstTime)
					    AddToList(list, $1.lval, $1.ltype, (char *)
						AllocateIconManager($1.lval, $2,
						$3, $4));
					}
		;

win_list	: LB win_entries RB
		{ }
		;

win_entries	: /* Empty */
		| win_entries win_entry
		;

win_entry	: matcher		{ if (Scr->FirstTime)
					    AddToList(list, $1.lval, $1.ltype, 0);
					}
		;

icon_list	: LB icon_entries RB
		{ }
		;

icon_entries	: /* Empty */
		| icon_entries icon_entry
		;

icon_entry	: matcher string		{ if (Scr->FirstTime)
						AddToList(list, $1.lval, $1.ltype, $2); }
		| matcher string COLON string	{
			if (Scr->FirstTime) {
#ifdef RJC
			    char *full_name=CacheAndNameIconImage($2, $4);
			    if (full_name != NULL)
				AddToList(list, $1.lval, $1.ltype, full_name);
			    else {
				twmrc_error_prefix();
				fprintf(stderr, "can't access bitmap and mask %s, %s\n",
				$2, $4);
			    }
#else
			yyerror("\"pixmap\" : \"mask\" syntax no longer supported");
#endif /* RJC */
				}
		}
		;

function	: LB function_entries RB
		{ }
		;

function_entries: /* Empty */
		| function_entries function_entry
		;

function_entry	: action		{ AddToMenu(root, "", Action, NULLSTR, $1,
						NULLSTR, NULLSTR);
					  Action = "";
					}
		;

menu_definition	: MENU string LP string COLON string RP {
					root = GetRoot($2, $4, $6); }
		  menu
		| MENU string { root = GetRoot($2, NULLSTR, NULLSTR); }
		  menu
		| PIEMENU string LP string COLON string RP
					{ root = GetRoot($2, $4, $6);
					  root->pie_menu = 1; }
		  menu
		| PIEMENU string	{ root = GetRoot($2, NULLSTR, NULLSTR);
					  root->pie_menu = 1; }
		  menu
		| PIEMENU string LP string COLON string RP AT number
					{ root = GetRoot($2, $4, $6);
					  root->pie_menu = 1;
					  root->initial_angle = DEG_TO_RAD($9); }
		  menu
		| PIEMENU string AT number
					{ root = GetRoot($2, NULLSTR, NULLSTR);
					  root->pie_menu = 1;
					  root->initial_angle = DEG_TO_RAD($4); }
		  menu
		;

menu		: menu_body
		| string	{
				root->file = ExpandFilename($1);
				root->real_menu = TRUE;
				}
		;

menu_body	: {
			if (Scr->SetupDone == TRUE) {
			    DestroyMenu(root);
			    root->first = NULL;
			    root->items = 0;
			    root->width = 0;
			    root->mapped = NEVER_MAPPED;
			}
			menu_default_index = 0;
		  }
		  LB menu_entries RB {
			if (Scr->SetupDone == TRUE) {
			    MakeMenu(root);
			}
			root->real_menu = TRUE;
			root->def = menu_default_index;
		  }
		;

menu_entries	: /* Empty */
		| menu_entries menu_entry
		;

menu_entry	: def_indicator string action
					{ AddToMenu(root, $2, Action, pull, $3,
						NULLSTR, NULLSTR);
					  Action = "";
					  pull = NULL;
					  if ($1) {
					    if (menu_default_index)
					      yyerror("two defaults in menu");
					    else if ($1 == 1)
					      menu_default_index = root->items;
					    else if ($1 == 2)
					      menu_default_index =
								(-root->items);
					  }
					}
		| def_indicator string LP string COLON string RP action
					{ AddToMenu(root, $2, Action, pull, $8,
						$4, $6);
					  Action = "";
					  pull = NULL;
					  if ($1) {
					    if (menu_default_index)
					      yyerror("two defaults in menu");
					    else if ($1 == 1)
					      menu_default_index = root->items;
					    else if ($1 == 2)
					      menu_default_index =
								(-root->items);
					  }
					}
		;

def_indicator	: /* EMPTY */		{ $$ = 0; }
		| GREATER		{ $$ = 1; }
		| GREATER GREATER	{ $$ = 2; }
		;

action		: FKEYWORD	{ $$ = $1; }
		| FSKEYWORD string {
				$$ = $1;
				Action = $2;
				switch ($1) {
				  case F_MENU:
				  case F_PIEMENU:
				    pull = GetRoot ($2, NULLSTR,NULLSTR);
				    pull->prev = NULL;
				    break;
				  case F_WARPRING:
				    if (!CheckWarpRingArg (Action)) {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.warptoring argument \"%s\"\n",
						 Action);
					$$ = F_NOP;
				    }
				  case F_WARPTOSCREEN:
				    if (!CheckWarpScreenArg (Action)) {
					twmrc_error_prefix();
					fprintf (stderr, 
			"ignoring invalid f.warptoscreen argument \"%s\"\n", 
					         Action);
					$$ = F_NOP;
				    }
				    break;
				  case F_COLORMAP:
				    if (CheckColormapArg (Action)) {
					$$ = F_COLORMAP;
				    } else {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.colormap argument \"%s\"\n", 
						 Action);
					$$ = F_NOP;
				    }
				    break;
				} /* end switch */
			    }
		| FSKEYWORD string COLON string {
				$$ = $1;
				Action = $4;
				pull = GetRoot ($2, NULLSTR,NULLSTR);
				pull->prev = NULL;
				break;
			    }
		;


signed_number	: number		{ $$ = $1; }
		| PLUS number		{ $$ = $2; }
		| MINUS number		{ $$ = -($2); }
		;

button		: BUTTON number		{ $$ = $2;
					  if ($2 == 0)
						yyerror("bad button 0");

					  if ($2 > MAX_BUTTONS)
					  {
						$$ = 0;
						yyerror("button number too large");
					  }
					}
		;

matcher		: string		{
					$$.ltype = LTYPE_ANY_STRING;
					$$.lval = $1;
					}
		| regexp		{
					$$.ltype = LTYPE_ANY_REGEXP;
					$$.lval = $1;
					}
		| MKEYWORD EQUALS string {
					$$.lval = $3;
					$$.ltype = $1 | LTYPE_STRING;
					}
		| MKEYWORD TILDE regexp {
					$$.lval = $3;
					$$.ltype = $1 | LTYPE_REGEXP;
					}
		;

geometry	: string		{ $$.gstring = $1;
					  $$.itterate = FALSE;
					}
		| string ASTERISK	{ $$.gstring = $1;
					  $$.itterate = TRUE;
					}
		;

string		: STRING		{ ptr = (char *)malloc(strlen($1)+1);
					  strcpy(ptr, $1);
					  RemoveDQuote(ptr);
					  $$ = ptr;
					}

regexp		: REGEXP		{ ptr = (char *)malloc(strlen($1));
					  /* chop off the '/'s */
					  strcpy(ptr, $1+1);
					  ptr[strlen(ptr)-1] = '\0';
					  $$ = ptr;
					}

number		: NUMBER		{ $$ = $1; }
		;

%%
yyerror(s) char *s;
{
    twmrc_error_prefix();
    fprintf (stderr, "error in input file:  %s\n", s ? s : "");
    fprintf (stderr, ": %s", current_input_line());
    ParseError = 1;
}
RemoveDQuote(str)
char *str;
{
    register char *i, *o;
    register n;
    register count;

    for (i=str+1, o=str; *i && *i != '\"'; o++)
    {
	if (*i == '\\')
	{
	    switch (*++i)
	    {
	    case 'n':
		*o = '\n';
		i++;
		break;
	    case 'b':
		*o = '\b';
		i++;
		break;
	    case 'r':
		*o = '\r';
		i++;
		break;
	    case 't':
		*o = '\t';
		i++;
		break;
	    case 'f':
		*o = '\f';
		i++;
		break;
	    case '0':
		if (*++i == 'x')
		    goto hex;
		else
		    --i;
	    case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
		n = 0;
		count = 0;
		while (*i >= '0' && *i <= '7' && count < 3)
		{
		    n = (n<<3) + (*i++ - '0');
		    count++;
		}
		*o = n;
		break;
	    hex:
	    case 'x':
		n = 0;
		count = 0;
		while (i++, count++ < 2)
		{
		    if (*i >= '0' && *i <= '9')
			n = (n<<4) + (*i - '0');
		    else if (*i >= 'a' && *i <= 'f')
			n = (n<<4) + (*i - 'a') + 10;
		    else if (*i >= 'A' && *i <= 'F')
			n = (n<<4) + (*i - 'A') + 10;
		    else
			break;
		}
		*o = n;
		break;
	    case '\n':
		i++;	/* punt */
		o--;	/* to account for o++ at end of loop */
		break;
	    case '\"':
	    case '\'':
	    case '\\':
	    default:
		*o = *i++;
		break;
	    }
	}
	else
	    *o = *i++;
    }
    *o = '\0';
}

static MenuRoot *GetRoot(name, fore, back)
char *name;
char *fore, *back;
{
    MenuRoot *tmp;

    tmp = FindMenuRoot(name);
    if (tmp == NULL)
	tmp = NewMenuRoot(name);

    if (fore)
    {
	int save;

	save = Scr->FirstTime;
	Scr->FirstTime = TRUE;
	GetColor(COLOR, &tmp->hi_fore, fore);
	GetColor(COLOR, &tmp->hi_back, back);
	Scr->FirstTime = save;
    }

    return tmp;
}

static void GotButton(butt, func)
int butt, func;
{
    int i;

    for (i = 0; i < NUM_CONTEXTS; i++)
    {
	if ((cont & (1 << i)) == 0)
	    continue;

	Scr->Mouse[butt][i][mods].func = func;
	if (func == F_MENU || func == F_MENUFUNC || func == F_PIEMENU)
	{
	    pull->prev = NULL;
	    Scr->Mouse[butt][i][mods].menu = pull;
	}
	else
	{
	    root = GetRoot(TWM_ROOT, NULLSTR, NULLSTR);
	    Scr->Mouse[butt][i][mods].item = AddToMenu(root,"x",Action,
		    NULLSTR, func, NULLSTR, NULLSTR);
	}
    }
    Action = "";
    pull = NULL;
    cont = 0;
    mods_used |= mods;
    mods = 0;
}

static void GotKey(key, func)
char *key;
int func;
{
    int i;

    for (i = 0; i < NUM_CONTEXTS; i++)
    {
	if ((cont & (1 << i)) == 0) 
	  continue;
	if (!AddFuncKey(key, i, mods, func, Name, Action)) 
	  break;
    }

    Action = "";
    pull = NULL;
    cont = 0;
    mods_used |= mods;
    mods = 0;
}


static void GotTitleButton (bitmapname, func, rightside)
    char *bitmapname;
    int func;
    Bool rightside;
{
    if (!CreateTitleButton (bitmapname, func, Action, pull, rightside, True)) {
	twmrc_error_prefix();
	fprintf (stderr, 
		 "unable to create %s titlebutton \"%s\"\n",
		 rightside ? "right" : "left", bitmapname);
    }
    Action = "";
    pull = NULL;
}

static Bool CheckWarpScreenArg (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s,  WARPSCREEN_NEXT) == 0 ||
	strcmp (s,  WARPSCREEN_PREV) == 0 ||
	strcmp (s,  WARPSCREEN_BACK) == 0)
      return True;

    for (; *s && isascii(*s) && isdigit(*s); s++) ; /* SUPPRESS 530 */
    return (*s ? False : True);
}


static Bool CheckWarpRingArg (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s,  WARPSCREEN_NEXT) == 0 ||
	strcmp (s,  WARPSCREEN_PREV) == 0)
      return True;

    return False;
}


static Bool CheckColormapArg (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s, COLORMAP_NEXT) == 0 ||
	strcmp (s, COLORMAP_PREV) == 0 ||
	strcmp (s, COLORMAP_DEFAULT) == 0)
      return True;

    return False;
}


twmrc_error_prefix ()
{
    fprintf (stderr, "%s:  line %d:  ", ProgramName, yylineno);
}
