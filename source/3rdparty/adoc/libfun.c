/*                                                               -*- C -*-
 *  LIBFUN.C
 *
 *  (c)Copyright 1995 by Tobias Ferber,  All Rights Reserved
 *
 *  This file is part of ADOC.
 *
 *  ADOC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 1 of the License,
 *  or (at your option) any later version.
 *
 *  ADOC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $VER: $Id: libfun.c,v 1.1 2001-07-18 11:05:53 root Exp $ */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libfun.h"

/* strexpand.c */
extern char *strexpand(char *, char **);

/* repair buggy realloc() implementation */
#define my_realloc(ptr,size)  ( (ptr) ? realloc(ptr,size) : malloc(size) )

/* minimum #of chars added to funsec.data with each realloc() */
#define SECMINSIZE 1024

/* list of sections in a function description */

typedef struct funsec
{
  struct funsec *succ, *pred;
  char *name;          /* section title keyword */
  struct {
    char *text;        /* section text */
    size_t len;        /* strlen(text) */
    size_t size;       /* #of chars allocated for text */
  } data;

} funsec_t;


/* library function nodes */

typedef struct libfun
{
  struct libfun *succ, *pred;
  char *name;          /* name of the function: `library/function' */
  struct funsec *sec;  /* list of sections in the function description */

} libfun_t;


/* variables */

static libfun_t *funlist  = (libfun_t *)0;  /* list of all functions */
static libfun_t *funstack = (libfun_t *)0;  /* function on the stack */
static libfun_t *thisfun  = (libfun_t *)0;  /* the current function (the one constructed last) */
static funsec_t *thissec  = (funsec_t *)0;  /* the current section in the current function */

char *pushfun(void) { char *result= funstack ? funstack->name : (char *)0; funstack = thisfun;  return result; }
char *popfun(void)  { char *result= funstack ? funstack->name : (char *)0; thisfun  = funstack; return result; }

/**/

static libfun_t *libfun_head(libfun_t *lf) { return (lf && lf->pred) ? libfun_head(lf->pred) : lf; }
static libfun_t *libfun_tail(libfun_t *lf) { return (lf && lf->succ) ? libfun_tail(lf->succ) : lf; }

static funsec_t *funsec_head(funsec_t *fs) { return (fs && fs->pred) ? funsec_head(fs->pred) : fs; }
static funsec_t *funsec_tail(funsec_t *fs) { return (fs && fs->succ) ? funsec_tail(fs->succ) : fs; }

/****** libfun/newfun ********************************************************
*
*   NAME
*       newfun -- Begin a new function
*
*   SYNOPSIS
*       error= newfun(fun);
*
*       int funindent(char *fun);
*
*   FUNCTION
*       This function adds the function `fun'.  All sections added via
*       newsec() will be made sections of this function.
*
*   INPUTS
*       fun           - The name of the function to add.
*                       (should be of the form "library/function")
*
*   RESULT
*       error         - either 0 on success or !=0 in case of an error
*
*   SEE ALSO
*       newsec(), funfree()
*
******************************************************************************
*
*/

static libfun_t *libfun_new(char *fun)
{
  libfun_t *lf= (libfun_t *)0;

  if(fun)
  {
    lf= (libfun_t *)malloc( sizeof(libfun_t) );

    if(lf)
    {
      lf->name= strdup(fun);

      if(lf->name)
      {
        lf->succ =
        lf->pred = (libfun_t *)0;
        lf->sec  = (funsec_t *)0;
      }
      else
      {
        free(lf);
        lf= (libfun_t *)0;
      }
    }
  }

  return lf;
}

/**/

int newfun(char *fun)
{
  libfun_t *new= libfun_new(fun);

  if(new)
  {
    libfun_t *lf= libfun_tail(funlist);

    if(lf)
    {
      lf->succ= new;
      new->pred= lf;
    }
    else funlist= new;
  }

  thisfun= new;
  thissec= new->sec;

  return new ? 0:1;
}

/****** libfun/funsort *******************************************************
*
*   NAME
*       funsort -- Sort all functions defined via newfun()
*
*   SYNOPSIS
*       funsort();
*
*       void funsort(void);
*
*   FUNCTION
*       Here we sort all functions defined via newfun() alphabetically.
*
*   INPUTS
*       none
*
*   RESULT
*       none
*
*   SEE ALSO
*       newfun(), funfree()
*
******************************************************************************
*
*/

static int funbubble(libfun_t *a)
{
  int result= 0;

  /* perform a recursive bubble-sort on `a' and it's successors
     return the #of bubbles */

  if(a && a->succ)
  {
    libfun_t *b= a->succ;

    if( strcmp(a->name, b->name) > 0 )
    {
      if( (b->pred= a->pred) )
        b->pred->succ= b;

      if( (a->succ= b->succ) )
        a->succ->pred= a;

      b->succ= a;
      a->pred= b;

      ++result;
    }

    result += funbubble(b);
  }

  return result;
}

/**/

void funsort(void)
{
  /* as long as there are bubbles... */
  while( funbubble(libfun_head(funlist)) );

  /* sorting might have made another libfun_t the head of funlist */
  funlist= libfun_head(funlist);

  for(thisfun= funlist; thisfun; thisfun= thisfun->succ)
  {
    /* make sure that thisfun->sec is the head of the sections */
    thisfun->sec= funsec_head(thisfun->sec);
  }

  thisfun= (libfun_t *)0;
  thissec= (funsec_t *)0;
}


/****** libfun/funexpand *****************************************************
*
*   NAME
*       funexpand -- Expand macros in all function bodies (unused)
*
*   SYNOPSIS
*       error= funexpand(macros);
*
*       int funexpand(char **macros);
*
*   FUNCTION
*       This function performs a strexpand() on all body text sections of all
*       functions.
*
*   INPUTS
*       macros        - the expansion table for strexpand()
*
*   RESULT
*       error         - either 0 on success or !=0 in case of an error
*
*   SEE ALSO
*       strexpand()
*
******************************************************************************
*
*/

int funexpand(char **macros)
{
  int err= 0;

  libfun_t *lf;
  funsec_t *fs;

  for(lf= libfun_head(funlist); lf && (err==0); lf= lf->succ)
  {
    for(fs= funsec_head(lf->sec); fs && (err==0); fs= fs->succ)
    {
      if( fs->data.text )
      {
        char *x= strexpand(fs->data.text, macros);

        if(x)
        {
          free(fs->data.text);
          fs->data.text= x;
        }
        else err= 1;
      }
    }
  }

  return err;
}


/****** libfun/funindent *****************************************************
*
*   NAME
*       funindent -- Perform an indentation correction on all functions
*
*   SYNOPSIS
*       error= funindent(indent, tabsize);
*
*       int funindent(int indent, int tabsize);
*
*   FUNCTION
*       This function performs a global rework on all functions defined via
*       newfun().  The first thing this function does is converting tabs in
*       all body text lines into spaces.  Then the indentation of body text
*       lines in all functions is then resized to `indent' spaces.
*       The indentation is minimized if `indent' is 0.   The `tabsize' is
*       needed to compute the minimum indentation of a body text, i.e. the
*       maximum width of a columnar block beginning at column 1 which cuts
*       leading white space without hurting any text.
*
*   INPUTS
*       indent        - The number of spaces used for the new indentation
*                       of all body text lines
*       tabsize       - The number of columns between two tab stops
*
*   RESULT
*       error         - either 0 on success or !=0 in case of an error
*
*   SEE ALSO
*
******************************************************************************
*
*/

int funindent(int indent, int tabsize)
{
  int err= 0;

  libfun_t *lf;
  funsec_t *fs;

  for(lf= libfun_head(funlist); lf && (err==0); lf= lf->succ)
  {
    for(fs= funsec_head(lf->sec); fs && (err==0); fs= fs->succ)
    {
      char *s= fs->data.text;

/*printf("%s: %s\n",lf->name,fs->name);*/

      if(s && *s)
      {
        int num_lines = 1; /* the number of lines in the body text  */
        int num_tabs  = 0; /* number of tabs used in the body text  */
        int i, k;          /* indentation counter                   */
        int i_min= -1;     /* minimum indentation of this body text */

        /* compute the minimum indentation `i_min' in `s' */

        while(*s)
        {
          /* get the indentation `i' of the current line */
          for(i=0; *s==' ' || *s=='\t'; s++) {
            if(*s=='\t') i+= tabsize, i-= (i%tabsize), num_tabs++;
            else i++;
          }

          if( i>0 && (i<i_min || i_min<0) )
            i_min= i;

          for(;*s && *s!='\n';s++)
            if(*s=='\t') ++num_tabs;

          while(*s=='\n')
            ++s, ++num_lines;
        }

        if(i_min > 1)
        {
          /* allocate a new string buffer for the indented fs->data.text */
          char *x= (char *)malloc( (size_t)(strlen(s= fs->data.text) + num_lines*indent + num_tabs*tabsize + 1) );

          if(x)
          {
            char *t= x;

            while(*s)
            {
              /* get the indentation of the current line */
              for(i=0; *s==' ' || *s=='\t'; s++) {
                if(*s=='\t') i+= tabsize, i-= (i%tabsize);
                else i++;
              }

              if( *s && (*s!='\n') )
              {
                /* cut the columnar block & add the new indentation */
                for(k= i - i_min + indent; k>0; k--)
                  *t++= ' ';

                /* `i' still holds the indentation of the old body text line.
                   We need this value to expand tabs as if we would still
                   have the old indentation */

                for(; *s && *s!='\n'; s++)
                {
                  if(*s=='\t')
                  {
                    /* DICE (Amiga) compiles the following line wrong! */
                    /*do { *t++= ' '; } while( (++i) % tabsize );*/
                    do { *t++= ' '; i++; } while( (i % tabsize) );
                  }
                  else i++, *t++= *s;
                }
              }

              while(*s=='\n')
                *t++= *s++;
            }
            *t= '\0';

            free(fs->data.text);
            fs->data.text= x;
          }
          else err= __LINE__;
        }
      }
    }
  }

  return err;
}

/****** libfun/newsec ********************************************************
*
*   NAME
*       newsec -- Add a new heading to the current function description
*
*   SYNOPSIS
*       error= newsec(title);
*
*       int newsec(char *title);
*
*   FUNCTION
*       This function adds a new section heading `title' to the current
*       function description which began with the last newfun() call.
*       The following calls of addtext() will add the text to this new
*       section.
*
*   INPUTS
*       title         - The section heading keyword
*
*   RESULT
*       error         - either 0 on success or !=0 in case of an error
*
*   SEE ALSO
*       newfun(), addtext()
*
******************************************************************************
*
*/

static funsec_t *funsec_new(char *title)
{
  funsec_t *fs= (funsec_t *)malloc( sizeof(funsec_t) );

  if(fs)
  {
    fs->name= (title) ? strdup(title) : (char *)0;

    if(title && !fs->name)
    {
      free(fs);
      fs= (funsec_t *)0;
    }

    else
    {
      fs->succ =
      fs->pred = (funsec_t *)0;

      fs->data.text = (char *)0;
      fs->data.len  =
      fs->data.size = 0;
    }
  }

  return fs;
}

/**/

int newsec(char *title)
{
  int err= 0;

  if(thisfun)
  {
    funsec_t *new= funsec_new(title);

    if(new)
    {
      funsec_t *fs= funsec_tail(thisfun->sec);

      if(fs)
      {
        fs->succ= new;
        new->pred= fs;
      }
      else thisfun->sec= new;
    }
    else err= 2;

    thissec= new;
  }
  else err= 1;

  return err;
}

/****** libfun/addtext *******************************************************
*
*   NAME
*       addtext -- Add a new line of text to the current function description
*
*   SYNOPSIS
*       error= addtext(text);
*
*       int funindent(char *text);
*
*   FUNCTION
*       This function dynamically adds a new line of text to the current
*       section in the current function description.  There does not need
*       to be a trailing newline `\n' at the end of `text'.
*
*   INPUTS
*       text          - The string to add to the description
*
*   RESULT
*       error         - either 0 on success or !=0 in case of an error
*
*   NOTES
*       Body text can only be added to a section.  There must be at least
*       one newsec() following newfun() before this function can add body
*       text lines.
*
*   SEE ALSO
*       newsec(), funfree()
*
******************************************************************************
*
*/

int addtext(char *text)
{
  int err= 0;

  if(thissec)
  {
    if(text)
    {
      /* get the minimum needed size of the text block */
      size_t len= thissec->data.len + 1 + strlen(text) + 1;  /* '\n' + '\0' */

      /* see if we have enough room for '\n' and text */
      if(len >= thissec->data.size)
      {
        /* resize the text buffer to the needed size + SECMINSIZE chars */
        char *old= thissec->data.text;
        char *new= (char *)my_realloc(old, (SECMINSIZE + len) * sizeof(char) );

        if(new)
        {
          thissec->data.text = new;
          thissec->data.size = SECMINSIZE + len;

          if(!old)
            *new= '\0';
        }
        else /* !new */
        {
          free(thissec->data.text);
          thissec->data.text = (char *)0;
          thissec->data.len  =
          thissec->data.size = 0;
          err= 1;
        }
      }

      /* append the new `text' portion to the section text */
      if(thissec->data.text)
      {
        if(*thissec->data.text)
          strcat(thissec->data.text,"\n");

        if(*text)
          strcat(thissec->data.text,text);

        thissec->data.len= strlen(thissec->data.text);
      }

    }
    /* else: no error */
  }
  else err= -1;

  return err;
}


/****** libfun/getfun ********************************************************
*
*   NAME
*       getfun -- Get a function by name
*
*   SYNOPSIS
*       fun= getfun(name)
*
*       char *getfun(char *name);
*
*   FUNCTION
*       This function searches the list of function descriptions for
*       a function `name' and returns a pointer `fun' to the name of the
*       function or a (char *)0 if there is no such function `name'.
*
*   INPUTS
*       name          - The name of the function to look for
*
*   RESULT
*       fun           - The real name of the function `name' or (char *)0
*                       if no function `name' was found.
*
*   SEE ALSO
*       newfun(), getsec()
*
******************************************************************************
*
*/

char *getfun(char *name)
{
  libfun_t *lf= (libfun_t *)0;

  if(name)
  {
    for(lf= libfun_head(funlist); (lf && strcmp(lf->name, name)); lf= lf->succ)
      ;
  }
  else lf= thisfun;

  return lf ? lf->name : (char *)0;
}


/****** libfun/getsec ********************************************************
*
*   NAME
*       getsec -- Get a section text via section heading
*
*   SYNOPSIS
*       text= getsec(heading)
*
*       char *getsec(char *heading);
*
*   FUNCTION
*       This function searches for the section `heading' in the function 
*       description of the current function.  If this section exists then
*       the corresponding section text is returned.
*
*   INPUTS
*       heading       - The section title keyword to look for
*
*   RESULT
*       text          - The section text of that section or (char *)0 if
*                       there is no such section in the description of the
*                       current function.
*
*   SEE ALSO
*       newsec(), getfun()
*
******************************************************************************
*
*/

char *getsec(char *name)
{
  funsec_t *fs= (funsec_t *)0;

  if(name)
  {
    if(thisfun)
    {
      for(fs= funsec_head(thisfun->sec); (fs && strcmp(fs->name, name)); fs= fs->succ)
        ;
    }
  }
  else fs= thissec;

  return fs ? fs->data.text : (char *)0;
}


/****** libfun/islib **********************************************************
*
*   NAME
*       islib -- Find a library in the list of functions
*
*   SYNOPSIS
*       gotit= islib(name)
*
*       int islib(char *name);
*
*   FUNCTION
*       Scan the list of functions for a function which begins with `name' 
*       and has a `/' immediately following `name'.
*
*   INPUTS
*       name           - The name of the library (without a trailing `/')
*
*   RESULT
*       gotit          - 1 if there is at least one function `name/...'
*                        0 otherwise.
*
******************************************************************************
*
*/

int islib(char *name)
{
  libfun_t *lf= (libfun_t *)0;

  if(name)
  {
    size_t l= strlen(name);
		       
    for(lf= libfun_head(funlist); (lf && strncmp(lf->name, name, l)); lf= lf->succ)
      ;
    if(lf && lf->name && (lf->name)[l]!='/')
      lf= (libfun_t *)0;
  }

  return lf ? 1:0;
}


/****** libfun/stepfun *******************************************************
*
*   NAME
*       stepfun -- Move the current function pointer
*
*   SYNOPSIS
*       fun= stepfun(trigger);
*
*       char *stepfun(int trigger);
*
*   FUNCTION
*       This function moves the current function pointer according to the
*       `trigger' value.
*
*   INPUTS
*       trigger       - < 0 moves back `-trigger' functions
*                       = 0 moves to the first function
*                       > 0 moves forward `trigger' functions
*
*   RESULT
*       fun           - The name of the function or (char *)0
*
*   SEE ALSO
*       stepsec()
*
******************************************************************************
*
*/

char *stepfun(int trigger)
{
  char *result= (char *)0;

  if(trigger > 0)
  {
    for(; thisfun && (trigger > 0); trigger--)
      thisfun= thisfun->succ;
  }
  else if(trigger < 0)
  {
    for(; thisfun && (trigger < 0); trigger++)
      thisfun= thisfun->pred;
  }
  else /* trigger == 0 */
    thisfun= libfun_head(funlist);

  if(thisfun)
  {
    thissec= funsec_head(thisfun->sec);
    result= thisfun->name;
  }
  else thissec= (funsec_t *)0;

  return result;
}


/****** libfun/stepsec *******************************************************
*
*   NAME
*       stepsec -- Move the current section pointer
*
*   SYNOPSIS
*       sec= stepsec(trigger);
*
*       char *stepsec(int trigger);
*
*   FUNCTION
*       This function moves the current section pointer according to the
*       `trigger' value.
*
*   INPUTS
*       trigger       - < 0 moves back `-trigger' sections
*                       = 0 moves to the first section
*                       > 0 moves forward `trigger' sections
*
*   RESULT
*       sec           - The name of the section or (char *)0
*
*   SEE ALSO
*       stepfun()
*
******************************************************************************
*
*/

char *stepsec(int trigger)
{
  char *result= (char *)0;

  if(trigger > 0)
  {
    for(; thissec && (trigger > 0); trigger--)
      thissec= thissec->succ;
  }
  else if(trigger < 0)
  {
    for(; thissec && (trigger < 0); trigger++)
      thissec= thissec->pred;
  }
  else /* trigger == 0 */
  {
    if(thisfun)
      thissec= funsec_head(thisfun->sec);

    else
      thissec= (funsec_t *)0;
  }

  if(thissec)
    result= thissec->name;

  return result;
}


/****** libfun/funfree *******************************************************
*
*   NAME
*       funfree -- Dispose all function descriptions
*
*   SYNOPSIS
*       funfree();
*
*       void funfree(void);
*
*   FUNCTION
*       This function disposes all function descriptions including all their
*       sections.
*
*   INPUTS
*       none
*
*   RESULT
*       none
*
*   SEE ALSO
*       newfun(), newsec(), addtext()
*
******************************************************************************
*
*/

static funsec_t *funsec_dispose(funsec_t *fs)
{
  if(fs)
  {
    fs->succ= funsec_dispose(fs->succ);

    if(fs->name)
      free(fs->name);

    if(fs->data.text)
      free(fs->data.text);
  }

  return (funsec_t *)0;
}

static libfun_t *libfun_dispose(libfun_t *lf)
{
  if(lf)
  {
    lf->succ= libfun_dispose(lf->succ);

    if(lf->name)
      free(lf->name);

    lf->sec= funsec_dispose( funsec_head(lf->sec) );

    free(lf);
  }

  return (libfun_t *)0;
}

void funfree(void)
{
  funlist= thisfun= libfun_dispose( libfun_head(funlist) );
  thissec= (funsec_t *)0;
}
