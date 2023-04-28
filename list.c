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

/**********************************************************************
 *
 * $XConsortium: list.c,v 1.20 91/01/09 17:13:30 rws Exp $
 *
 * TWM code to deal with the name lists for the NoTitle list and
 * the AutoRaise list
 *
 * 11-Apr-88 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#include <stdio.h>
#include <X11/Xatom.h>

#include "twm.h"
#include "screen.h"
#include "gram.h"
#include "util.h"
#include "regexp.h"

#define CACHE_REGEXP

struct name_list_struct
{
    name_list *next;		/* pointer to the next name */
    char *name;			/* the name of the window */
    int namelen;		/* strlen(name) */
#ifdef CACHE_REGEXP
    regexp *regexp;		/* compile only once */
#endif
    short type;			/* what type of match */
    Atom property;		/* if type=property */
    char *ptr;			/* list dependent data */
};

/***********************************************************************
 *
 * Wrappers to allow code to step through a list
 *
 ***********************************************************************/

name_list *
next_entry(list)
name_list *list;
{
    return list->next;
}

char *
contents_of_entry(list)
name_list *list;
{
    return list->ptr;
}

/***********************************************************************
 *
 *  Procedure:
 *	AddToList - add a window name to the appropriate list
 *
 *  Inputs:
 *	list	- the address of the pointer to the head of a list
 *	name	- a pointer to the name of the window 
 *	type	- a bitmask of what to match against
 *	property- a window propery to match against
 *	ptr	- pointer to list dependent data
 *
 *  Special Considerations
 *	If the list does not use the ptr value, a non-null value 
 *	should be placed in it.  LookInList returns this ptr value
 *	and procedures calling LookInList will check for a non-null 
 *	return value as an indication of success.
 *
 ***********************************************************************
 */

void
AddToList(list_head, name, type, /* property,*/ ptr)
name_list **list_head;
char *name;
short type;
/*
Atom property;
*/
char *ptr;
{
    Atom property = None;
    name_list *nptr;

    if (!list_head) return;	/* ignore empty inserts */

    nptr = (name_list *)malloc(sizeof(name_list));
    if (nptr == NULL)
    {
	twmrc_error_prefix();
	fprintf (stderr, "unable to allocate %d bytes for name_list\n",
		 sizeof(name_list));
	Done();
    }

    while (*list_head)
	list_head = &((*list_head)->next);

    nptr->next = NULL;
    nptr->name = name;
    if (type & LTYPE_HOST) {
	nptr->type = (type & ~LTYPE_HOST) | LTYPE_PROPERTY;
	nptr->property = XA_WM_CLIENT_MACHINE;
    } else {
	nptr->type = type;
	nptr->property = property;
    }
    nptr->namelen = strlen(name);
    nptr->ptr = (ptr == NULL) ? (char *)TRUE : ptr;
    nptr->regexp = NULL;
    *list_head = nptr;
}    

void
printNameList(name, nptr)
char *name;
name_list *nptr;
{
    printf("%s=[", name);

    while (nptr) {
	printf(" `%s':%d", nptr->name, nptr->type);
	nptr = nptr->next;
    }
    printf(" ]\n");
}

 /********************************************************************\
 *                                                                    *
 * New LookInList code by RJC.                                        *
 *                                                                    *
 * Since we want to be able to look for multiple matches (eg, to      *
 * check which relevant icon regions are on the screen), the basic    *
 * procedure is now MultiLookInList and uses a (pseudo-)continuation  *
 * to keep track of where it is.                                      *
 *                                                                    *
 * LookInList is a trivial specialisation of that.                    *
 *                                                                    *
 * Also, we now allow regular expressions in lists, so here we use    *
 * Henry Spencer's regular expression code.  It is possible that we   *
 * should pre-compile all the regular expressions for maximum         *
 * speed.                                                             *
 *                                                                    *
 \********************************************************************/

static char *regexp_error = NULL;

void regerror(txt)
char *txt;
{
    regexp_error = txt;
}

static int
MatchName(name, pattern, length, compiled, type)
char *name;
char *pattern;
int length;
regexp *compiled;
short type;
{
    /* fprintf(stderr, "\tcompare %s with %s\n", name, pattern); */

    if (type & LTYPE_ANYTHING)
	return 1;

    if (type & LTYPE_REGEXP) {
	regexp *matcher;
	int result;

	regexp_error = "";
	if ((matcher = regcomp(pattern)) == NULL) {
	    fprintf(stderr, "%s: Error in regexp `%s'\n", ProgramName,
			regexp_error, name);
	    return 0;
	}

	result = regexec(matcher, name, 1);
	free(matcher);
	return result;
    } else if (type & LTYPE_C_REGEXP)
	return regexec(compiled, name, 1);
    else if (type & LTYPE_STRING)
	return (!strcmp(name, pattern));
    else {
	fprintf(stderr,
		"%s: WARNING!  Bad list type (%d), comparing `%s' with `%s'\n",
		ProgramName, type, name, pattern);
	return 0;
    }
}

char *
MultiLookInList(list_head, name, class, continuation)
name_list *list_head;
char *name;
XClassHint *class;
name_list **continuation;
{
    name_list *nptr;

    /* fprintf(stderr, "looking for %s\n", name); */

    for (nptr = list_head ; nptr ; nptr = nptr->next) {
#ifdef CACHE_REGEXP
	if (nptr->type & LTYPE_REGEXP) {
	    regexp *matcher;

	    regexp_error = "";

	    if ((matcher = regcomp(nptr->name)) == NULL) {
		fprintf(stderr, "%s: %s in regexp /%s/\n",
			ProgramName, regexp_error, nptr->name);
		nptr->type &= ~LTYPE_REGEXP;
		nptr->type |=  LTYPE_NOTHING;
	    } else {
		nptr->regexp = matcher;
		nptr->type &= ~LTYPE_REGEXP;
		nptr->type |=  LTYPE_C_REGEXP;
	    }
	}
#endif

	if (nptr->type & LTYPE_NOTHING) 
	    continue;				/* skip illegal entry */

	if (nptr->type & LTYPE_ANYTHING) {
	    *continuation = nptr->next;
	    return nptr->ptr;
	}
	if (nptr->type & LTYPE_NAME)
	    if (MatchName(name, nptr->name, nptr->namelen, nptr->regexp, nptr->type)) {
		*continuation = nptr->next;
		return nptr->ptr;
	    }
	if (class) {
	    if (nptr->type & LTYPE_RES_NAME)
		if (MatchName(class->res_name, nptr->name, nptr->namelen, nptr->regexp, nptr->type)) {
		    *continuation = nptr->next;
		    return nptr->ptr;
		}
	    if (nptr->type & LTYPE_RES_CLASS)
		if (MatchName(class->res_class, nptr->name, nptr->namelen, nptr->regexp, nptr->type)) {
		    *continuation = nptr->next;
		    return nptr->ptr;
		}
	}
    }
    *continuation = NULL;
    return NULL;
}

char *
LookInList(list_head, name, class)
name_list *list_head;
char *name;
XClassHint *class;
{
    name_list *nptr;
    name_list *rest;
    char *return_name = MultiLookInList(list_head, name, class, &rest);

    if ((Scr->ListRings == TRUE) && (return_name != NULL)
	&& (list_head->next != NULL)) 
    {
	/* To implement a ring on the linked list where we cant change the */
	/* list_head, use a simple unlink/link-at-end alg. unless you need */
	/* to move the first link.   In that case swap the contents of the */
	/* first link with the contents of the second then proceed as */
	/* normal.  */
	name_list *tmp_namelist;
	
	if (list_head->ptr == return_name)
	{
	    char *tmp_name;
	    short tmp_type;
	    int   tmp_namelen;
	    char *tmp_ptr;
	    
	    tmp_name = list_head->name;
	    tmp_type = list_head->type;
	    tmp_namelen = list_head->namelen;
	    tmp_ptr = list_head->ptr;
	    
	    list_head->name = list_head->next->name;
	    list_head->type = list_head->next->type;
	    list_head->namelen = list_head->next->namelen;
	    list_head->ptr = list_head->next->ptr;
	    
	    list_head->next->name = tmp_name;
	    list_head->next->type = tmp_type;
	    list_head->next->namelen = tmp_namelen;
	    list_head->next->ptr = tmp_ptr;
	}
	
	for (nptr = list_head; nptr->next != NULL; nptr = nptr->next)
	{
	    if (nptr->next->ptr == return_name)
	      break;
	}
	
	if (nptr->next->next != NULL)
	{
	    tmp_namelist = nptr->next;
	    nptr->next = nptr->next->next;
	    
	    for (nptr = nptr->next; nptr->next != NULL; nptr = nptr->next);
	    nptr->next = tmp_namelist;
	    nptr->next->next = NULL;
	}
    }
    
    return return_name;
}

char *
MultiLookInNameList(list_head, name, continuation)
name_list *list_head;
char *name;
name_list **continuation;
{
    return (MultiLookInList(list_head, name, NULL, /*None,*/ continuation));
}

char *
LookInNameList(list_head, name)
name_list *list_head;
char *name;
{
    return (MultiLookInList(list_head, name, NULL, /*None,*/ &list_head));
}

/***********************************************************************
 *
 *  Procedure:
 *	GetColorFromList - look through a list for a window name, or class
 *
 *  Returned Value:
 *	TRUE if the name was found
 *	FALSE if the name was not found
 *
 *  Inputs:
 *	list	- a pointer to the head of a list
 *	name	- a pointer to the name to look for
 *	class	- a pointer to the class to look for
 *
 *  Outputs:
 *	ptr	- fill in the list value if the name was found
 *
 ***********************************************************************
 */

int GetColorFromList(list_head, name, class, ptr)
name_list *list_head;
char *name;
XClassHint *class;
Pixel *ptr;
{
    int save;
    char *val = LookInList(list_head, name, class);

    if (val) {
	save = Scr->FirstTime;
	Scr->FirstTime = TRUE;
	GetColor(Scr->Monochrome, ptr, val);
	Scr->FirstTime = save;
	return (TRUE);
    }

    return (FALSE);
}

/***********************************************************************
 *
 *  Procedure:
 *	FreeList - free up a list
 *
 ***********************************************************************
 */

void FreeList(list)
name_list **list;
{
    name_list *nptr;
    name_list *tmp;

    for (nptr = *list; nptr != NULL; )
    {
	tmp = nptr->next;
	if (nptr->regexp != NULL)
	    free((char *)nptr->regexp);
	free((char *) nptr);
	nptr = tmp;
    }
    *list = NULL;
}
