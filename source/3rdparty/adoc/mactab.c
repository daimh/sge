/*                                                               -*- C -*-
 *  MACTAB.C
 *
 *  (c)Copyright 1995 by Tobias Ferber,  All Rights Reserved
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 1 of the License,
 *  or (at your option) any later version.
 *
 *  This file is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $VER: $Id: mactab.c,v 1.1 2001-07-18 11:05:53 root Exp $ */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "mactab.h"

/* repair buggy realloc() implementation */
#define my_realloc(ptr,size)  ( (ptr) ? realloc(ptr,size) : malloc(size) )

/* #of macro entries added when expanding dynamically */
#define MACRESIZE  10

/* bytesize of a macro table for `n' macros */
#define MACTABSIZE(n)  (size_t)( 2*(n) * sizeof(char *) + 2 * sizeof(char *))

typedef struct macnode {
  struct macnode *succ, *pred;
  int handle;           /* a unique macnode handle */
  int reserved, used;   /* #of macros allocated (reserved) and used in mactab */
  char **mactab;        /* the mactro table for strexpand() */
} macnode_t;


/* globals += */
static int new_handle= 1;
static macnode_t *maclist= (macnode_t *)0;

static macnode_t *macnode_head(macnode_t *this) { return (this && this->pred) ? macnode_head(this->pred) : this; }
static macnode_t *macnode_tail(macnode_t *this) { return (this && this->succ) ? macnode_tail(this->succ) : this; }

/* expand the `macnode.mactab' table for `num_macros' macros */
static int macnode_expand(macnode_t *this, int num_macros)
{
  int result= 0;

  if(this)
  {
    char **new= (char **)my_realloc( (void *)(this->mactab), MACTABSIZE(this->reserved + num_macros) );

    if(new)
    {
      this->mactab= new;
      result= (this->reserved += num_macros);
    }
  }

  return result;
}


/* allocate a new `macnode_t' structure with a `mactab' holding `num_macros' macros */
static macnode_t *macnode_new(int num_macros)
{
  macnode_t *new= (macnode_t *)malloc( sizeof(macnode_t) );

  if(new)
  {
    new->succ     =
    new->pred     = (macnode_t *)0;

    new->reserved =
    new->used     = 0;

    new->mactab   = (char **)0;

    /* allocate the macro table */
    if( macnode_expand(new,num_macros) )
    {
      /* set the last mactro to NIL */
      new->mactab[0] =
      new->mactab[1] = (char *)0;

      /* give this macnode a handle */
      new->handle= new_handle++;
    }
    else /* !new->mactab */
    {
      free(new);
      new= (macnode_t *)0;
    }
  }

  return new;
}


/* remove `this' from the `maclist' -- return `this' */
static macnode_t *macnode_remove(macnode_t *this)
{
  if(this)
  {
    if(this->pred)
      this->pred->succ= this->succ;

    else
      maclist= this->succ;

    if(this->succ)
      this->succ->pred= this->pred;

    this->pred =
    this->succ = (macnode_t *)0;
  }

  return this;
}


/* add a macnode_t list `this' to the global `maclist' */
static void macnode_add(macnode_t *this)
{
  if(this)
  {
    macnode_tail(this)->succ= maclist;
    maclist= macnode_head(this);
  }
}


/* remove `this' from the `maclist' and dispose it */
static macnode_t *macnode_dispose(macnode_t *this)
{
  if(this)
  {
    macnode_remove(this);

    if(this->mactab)
    {
      int n;

      for(n=0; n < 2*this->used; n++)
      {
        if( (this->mactab)[n] )
          free( (this->mactab)[n] );
      }

      free(this->mactab);
    }
    free(this);
  }
  return (macnode_t *)0;
}


/* find the macnode_t in the `maclist' via handle */
static macnode_t *macnode_find(int handle)
{
  macnode_t *this= maclist;

  while( this && (this->handle != handle) )
    this= this->succ;

  return this;
}


/* get the pointer to a macro `lhs' in `this->mactab' or the pointer to a new entry */
static char **macnode_getmac(macnode_t *this, char *lhs)
{
  char **tab= (char **)0;

  if(this && this->mactab && lhs && *lhs)
  {
    int n;

    for(n=0, tab= this->mactab; n < 2*this->used; n+=2, tab= &tab[2])
    {
      if( *tab && !strcmp(*tab,lhs) )
        break;
    }
  }

  return tab;
}



/* modify a macro `lhs' w/ value `rhs' in `this' macnode_t's `mactab'.
   The `rhs' of an existing macro `lhs' is replaced with the given `rhs',
   an `rhs' == (char *)0 can be used to remove `lhs' from `this->mactab'. */

static int macnode_setmac(macnode_t *this, char *lhs, char *rhs)
{
  int err= 0;

  if(this && lhs && *lhs)
  {
    char **ptr= macnode_getmac(this,lhs);

    if(ptr)
    {
      if(*ptr) /* we found the macro `lhs' at the position `ptr' */
      {
        /* free the old `rhs' value */
        if(ptr[1])
          free(ptr[1]);

        if(rhs) /* set the new `rhs' value */
        {
          ptr[1]= strdup(rhs);

          if(!ptr[1])
            err= 3;
        }

        else /* remove this macro */
        {
          if(ptr[0])
            free(ptr[0]);

          while(ptr[2])
          {
            ptr[0]= ptr[2];
            ptr[1]= ptr[3];

            ptr= &ptr[2];
          }

          ptr[0]= ptr[1]= (char *)0;

          this->used--;
        }
      }

      else if(rhs) /* `lhs' is a new macro */
      {
        if(this->used + 1 > this->reserved)
        {
          /* resize the macro table (add MACRESIZE new macro entries) */
          if( macnode_expand(this, MACRESIZE) == 0 )
            err= 4;
        }

        if(err == 0)
        {
          char *l= strdup(lhs);
          char *r= strdup(rhs);

          if(l && r)
          {
            ptr= &(this->mactab)[2*this->used];

            ptr[0]= l;
            ptr[1]= r;

            ptr[2]=
            ptr[3]= (char *)0;

            this->used++;
          }
          else /* strdup() failed */
          {
            if(l) free(l);
            if(r) free(r);

            err= 3;
          }
        }
      }
      /* otherwise this was a no-op (:< */
    }
    else err= 2;
  }
  else err= 1;

  return err;
}

/*
**  PUBLIC
*/


/****** mactab/mactab ********************************************************
*
*   NAME
*       mactab -- Get the macro table associated with a handle
*
*   SYNOPSIS
*       table= mactab( handle );
*
*       char **mactab( int handle );
*
*   FUNCTION
*       This function returns the macro table with the given handle `handle'.
*       The returned macro table can be directly passed to strexpand().  This
*       function will not fail if `handle' is a legal mactab handle.
*
*   INPUTS
*       handle        - The handle of the macro table
*
*   RESULT
*       table         - A macro table for strexpand()
*
*   EXAMPLE
*       {
*         \* create a new macro table *\
*         int my_handle = mactab_new( 3 );
*
*         if(my_handle != 0)
*         {
*           \* initialize macros for a "ASCII->Texinfo" conversion *\
*           int err= mactab_add( my_handle,   "@",  "@@"
*                                             "{",  "@{",
*                                             "}",  "@}",  (char *)0 );
*           if(err == 0)
*           {
*             \* convert our `ascii_string' into Texinfo *\
*             char *texi_string= strexpad( ascii_string, mactab(my_handle) );
*
*             if(texi_string)
*             {
*               \* print the converted string to stdout and dispose it *\
*               puts(texi_string);
*               free(texi_string);
*             }
*             else out_of_memory();
*           }
*           else out_of_memory();
*
*           \* free our macro table *\
*           my_handle= mactab_dispose(my_handle);
*         }
*         else out_of_memory();
*       }
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       mactab_new(), mactab_add(), strexpand()
*
******************************************************************************
*
*  get the macro table `mactab' of the macnode_t in `maclist' with handle `handle'
*/

char **mactab(int handle)
{
  macnode_t *this= macnode_find(handle);
  return this ? this->mactab : (char **)0;
}


/****** mactab/mactab_new ****************************************************
*
*   NAME
*       mactab_new -- Allocate a new macro table
*
*   SYNOPSIS
*       handle= mactab_new( num_macros );
*
*       int mactab_new( int num_macros );
*
*   FUNCTION
*       This function allocates a new macro table which offers enough room
*       to hold `num_macros' macros plus one NULL entry.  The value of
*       `num_macros' should be > 0, however if more macros are added, then
*       the macro table will be expanded dynamically.
*
*   INPUTS
*       num_macros    - The number of macros reserved for this macro table.
*                       (should be > 0)
*
*   RESULT
*       handle        - The handle of the allocated macro table or
*                       0 in case of an error.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       mactab_dispose(), mactab(), mactab_add(), strexpand()
*
******************************************************************************
*
*  allocate a new macnode_t and reserve `num_macros' macros in the mactab
*/

int mactab_new(int num_macros)
{
  macnode_t *this= macnode_new(num_macros);

  if(this)
    macnode_add(this);

  return this ? this->handle : 0;
}


/****** mactab/mactab_dispose ************************************************
*
*   NAME
*       mactab_dispose -- Free one (or all) macro tables
*
*   SYNOPSIS
*       null_handle= mactab_dispose( handle );
*
*       int mactab_dispose( int handle );
*
*   FUNCTION
*       This function removes the macro table associated with `handle'
*       and returns all memory allocated for it to the system free pool.
*       If a negative handle is passed to this function then *ALL*
*       macro tables will be disposed.
*
*   INPUTS
*       handle        - The handle of the macro table to delete or
*                       `-1' to delete all macro tables.
*
*   RESULT
*       null_handle   - a handle which is never associated with a
*                       macro table.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       mactab_new(), mactab_remove()
*
******************************************************************************
*
*  dispose the macnode_t with handle `handle' or all macnode_t's if handle < 0
*/

int mactab_dispose(int handle)
{
  if(handle > 0)
  {
    macnode_t *this= macnode_find(handle);

    if(this)
      this= macnode_dispose(this);
  }
  else if(handle < 0)
  {
    while(maclist)
    {
      macnode_t *this= maclist;
      maclist= maclist->succ;
      macnode_dispose(this);
    }
  }

  return 0;
}

/****** mactab/mactab_remove *************************************************
*
*   NAME
*       mactab_remove -- Remove one or more macros from a macro table
*
*   SYNOPSIS
*       error= mactab_remove( handle, macro1, macro2, ..., (char *)0 );
*
*       int mactab_remove( int handle, ... );
*
*   FUNCTION
*       This function removes all given macros from the macro table associated
*       with the handle `handle'.  The list of macro names must be terminated
*       with a (char *)0.
*
*   INPUTS
*       handle        - The handle of the macro table
*       ...           - A list of macro names with a trailing (char *)0
*
*   RESULT
*       error         - != 0 in case of an error,  0 otherwise
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       mactab_get(), mactab_add(), mactab_dispose()
*
******************************************************************************
*
*  delete the given macros from the `mactab' of the macnode with handle `handle'
*/

int mactab_remove(int handle, ...)
{
  int err= 0;

  macnode_t *this= macnode_find(handle);

  if(this)
  {
    char *lhs;

    va_list argp;
    va_start(argp,handle);

    do {
      lhs= va_arg(argp, char *);

      if(lhs && *lhs)
      {
        /* set the `rhs' of each macro `lhs' to NIL */
        err= macnode_setmac(this, lhs, (char *)0);
      }
    } while(lhs && *lhs && !err);

    va_end(argp);
  }
  else err= 1;

  return err;
}


/****** mactab/mactab_add ****************************************************
*
*   NAME
*       mactab_add -- Add one or more macros to a macro table
*
*   SYNOPSIS
*       error= mactab_add( handle, macro1, value1, ..., (char *)0 );
*
*       int mactab_add( int handle, ... );
*
*   FUNCTION
*       This function adds the given macros to the macro table associated with
*       given handle `handle'.  A (char *)0 terminates the list of macro names
*       and macro values.  A macro value (char *)0 can be used to remove this
*       macro from the list of macros in this table and is equivalent to a
*       mactab_remove() call for this macro name.
*
*   INPUTS
*       handle        - The handle of the macro table
*       ...           - A list of macro names and macro values with a
*                       trailing (char *)0
*
*   RESULT
*       error         - != 0 in case of an error,  0 otherwise
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       mactab_remove(), mactab_new(), mactab_get(), mactab(), strexpand()
*
******************************************************************************
*
*  add some macros to the `mactab' of the macnode_t with handle `handle'
*/

int mactab_add(int handle, ...)
{
  int err= 0;

  macnode_t *this= macnode_find(handle);

  if(this)
  {
    char *lhs, *rhs;

    va_list argp;
    va_start(argp,handle);

    do {
      lhs= va_arg(argp, char *);

      if(lhs && *lhs)
      {
        rhs= va_arg(argp, char *);
        err= macnode_setmac(this,lhs,rhs);
      }
    } while(lhs && *lhs && !err);

    va_end(argp);
  }
  else err= 1;

  return err;
}


/****** mactab/mactab_get ****************************************************
*
*   NAME
*       mactab_get -- Get the value of a macro
*
*   SYNOPSIS
*       value= mactab_get( handle, name );
*
*       char *mactab_get( int handle, char *name );
*
*   FUNCTION
*       This function returns the value of the macro `name' out of the table
*       associated with the given handle `handle'.
*
*   INPUTS
*       handle        - The handle of the macro table
*       name          - The name of the macro
*
*   RESULT
*       value         - The value of the macro `name' or 
*                       (char *)0 if `name' was not found
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       mactab_new(), mactab_add(), mactab()
*
******************************************************************************
*
*  get the macro table `mactab' of the macnode_t in `maclist' with handle `handle'
*/

char *mactab_get(int handle, char *lhs)
{
  char *rhs= (char *)0;

  macnode_t *this= macnode_find(handle);

  if(this)
  {
    char **ptr= macnode_getmac(this,lhs);

    if(ptr && *ptr)
      rhs= ptr[1];
  }

  return rhs;
}


#ifdef DEBUG
#include <stdio.h>

void mactab_debug(FILE *fp)
{
  macnode_t *this;

  for(this= maclist; this; this= this->succ)
  {
    char **tab= this->mactab;

    fprintf(fp,"handle %d:  (reserved: %d, used: %d)\n",this->handle, this->reserved, this->used);

    if(tab)
    {
      int n;

      for(n=0; n<this->used; n++)
      {
        fprintf(fp,"\t\"%s\"\t\"%s\"\n", tab[0] ? tab[0] : "(nil)", tab[1] ? tab[1] : "(nil)");
        tab= &tab[2];
      }
    }

    fprintf(fp,"\n");
  }
}

#endif /* DEBUG */
