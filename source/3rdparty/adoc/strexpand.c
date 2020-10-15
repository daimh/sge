/*                                                               -*- C -*-
 *  STREXPAND.C
 *
 *  (c)Copyright 1990 by Tobias Ferber,  All Rights Reserved
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

/* $VER: $Id: strexpand.c,v 1.1 2001-07-18 11:05:53 root Exp $ */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct rule {
  struct rule *rules[256];
  char *rhs;
} rule_t;

/*
 *  Dieses Array enth"alt an der Position rules[x].rhs den String in den `x'
 *  "ubersetzt werden soll und/oder einen Zeiger rules[x].rules auf die Liste,
 *  die in (rules[x].rules)[y] die "Ubersetzungen f"ur `xy' enthalten...
 */

static rule_t *rules[256];

/*
*/

static rule_t *rule_new(void)
{
  rule_t *r= (rule_t *)malloc( sizeof(rule_t) );

  if(r)
  {
    int c;

    for(c=0; c<256; c++)
      (r->rules)[c]= (rule_t *)0;

    r->rhs = (char *)0;
  }

  return r;
}

/*
*/

static rule_t *rule_dispose(rule_t *r)
{
  if(r)
  {
    int c;

    for(c=0; c<256; c++)
      (r->rules)[c]= rule_dispose( r->rules[c] );

    r->rhs= (char *)0;

    free(r);
  }

  return (rule_t *)0;
}

/*
*/

static int rule_add(char *lhs, char *rhs)
{
  int err= 0;

  if(lhs && *lhs)
  {
    int c;
    rule_t **R;

    for(R=&rules[0], c= (int)(unsigned char)*lhs; c && (err==0); c= (int)(unsigned char)*++lhs)
    {
      if( !R[c] )
        R[c]= rule_new();

      if(R[c])
      {
        if(lhs[1])
          R= R[c]->rules;

        else
          R[c]->rhs= (rhs ? rhs : "");  /* Fri Feb 24 18:20:39 1995 */
      }
      else err= 2;
    }
  }
  else err= 1;

  return err;
}

/*
*/

static char *rule_find(char **lhs_)
{
  char *lhs= *lhs_;
  char *rhs= (char *)0;

  rule_t **R= &rules[0];

  int done;

  for(done=0; R[(int)(unsigned char)(*lhs)] && !done; ++lhs)
  {
    rule_t *r= R[ (int)(unsigned char)(*lhs) ];

    if( r->rules && (r->rules)[ (int)(unsigned char)(lhs[1]) ] )
      R= r->rules;

    else
    {
      rhs= r->rhs;
      done= 1;
    }
  }

  if(rhs)
    *lhs_= lhs;

  return rhs;
}

/*
*/

static int strexpand_init(char **table)
{
  int err=0;
  int c, t;

  /* initialize the rules[] array */
  for(c=0; c<256; c++)
    rules[c]= (rule_t *)0;

  /* add the given rules */

  if(table)
  {
    for(t=0; table[t] && (err==0); t+=2)
      err= rule_add(table[t], table[t+1]);
  }

  return err;
}

/*
*/

static void strexpand_exit(void)
{
  int c;

  for(c=0; c<256; c++)
  {
    if(rules[c])
      rules[c]= rule_dispose(rules[c]);
  }
}


/****** strexpand/strexpand **************************************************
*
*   NAME
*	strexpand -- Expand macros in a string dynamically
*
*   SYNOPSIS
*	str= strexpand(fmt, tab)
*
*	char *strexpand( char *, char ** );
*
*   FUNCTION
*	This function expands all occurences of tab[n] (n even) in `fmt' to
*	tab[n+1] and returns a dynamically allocated string which has to be
*	disposed via free(str) by the user.
*
*	In other words: given translation table `tab' holds substitution-
*	or translation rules with the source string (or left-hand-side)
*	on even places and the destination string (or right-hand-side) on
*	the odd places.  The translation is performed on given string `fmt'
*	in the following way:
*
*			tab[0] -> tab[1]
*			tab[2] -> tab[3]
*			tab[4] -> tab[5]  ....
*
*	I.e.: all occurences of tab[0] in `fmt' will be replaced by tab[1],
*				tab[2]                              tab[3],
*				tab[4]                              tab[5],
*	      etc.
*
*   INPUTS
*	fmt	      - The format string holding 0 or more of the macros
*
*	tab	      - A table of strings with the source string (on even
*			indices) and the destination string.
*			This table must end with a (char *)0 as the last
*			source string.
*
*   RESULT
*	str	      - A dynamically allocated copy of `fmt' with all 
*			occurences of t[n] (n even) replaced by t[n+1].
*
*   EXAMPLE
*
*	char *table[] = { "ab"  , "1234",
*	                  "abc" , "xyz",
*	                  "ef"  , "foobar",  0,0 };
*
*	compiling the translation table via strexpand_init(table) yields the
*	following graph:
*
*	          +---+---+---+---+---+---+
*	rules[] = | a | b | c | d | e | f | ....
*	          +---+---+---+---+---+---+
*	           /\               /\
*	          /                   \
*	          +---+---+            +---+---+---+---+---+---+
*	          | a | b | ....       | a | b | c | d | e | f | ....
*	          +---+---+            +---+---+---+---+---+---+
*	               /\                                    /\
*	              /  \                                     \
*	             /    +-- "1234"                            +-- "foobar"
*	            /
*	           /
*	          +---+---+---+---+---+---+
*	          | a | b | c | d | e | f | ....
*	          +---+---+---+---+---+---+
*	                   /\
*	                     \
*	                      +-- "xyz"
*
*	strexpand("abcdefgab!",table); returns "xyzdfoobarg1234!".
*
*   SEE ALSO
*
******************************************************************************
*
*
*/

char *strexpand(char *fmt, char **tab)
{
  char *buf= (char *)0;

  if(fmt && *fmt)
  {
    if( strexpand_init(tab) == 0 )
    {
      size_t len= strlen(fmt) + 1;  /* size of output buffer */

      if( (buf= (char *)malloc(len*sizeof(char))) )
      {
        unsigned int pos= 0; /* char position in buf */

/**/
        while(buf && *fmt)
        {
          char *rhs= rule_find( &fmt );

          if(rhs)
          {
            while(*rhs)
            {
              buf[pos++]= *rhs++;

              if(pos >= len)
              {
                char *old= buf;

                len += 1024;
                buf= (char *)realloc(buf,len*sizeof(char));

                if(!buf)
                  free(old);
              }
            }
          }
          else /* !rhs */
          {
            buf[pos++]= *fmt++;

            if(pos >= len)
            {
              char *old= buf;

              len += 1024;
              buf= (char *)realloc(buf,len*sizeof(char) );

              if(!buf)
                free(old);
            }
          }
        }
/**/
        if(buf)
          buf[pos]= '\0';

/*printf("strexpand(): pos=%d, len=%d\n",pos,len);*/
      }
    }
    strexpand_exit();
  }

  return buf;
}



#ifdef TEST

#include <stdio.h>

char *table[] = { "ab"  , "1234",
                  "abc" , "xyz",
                  "ef"  , "foobar",  0,0 };

int main(int argc, char **argv)
{
  while(--argc > 0)
  {
    char *s= strexpand(*++argv, table);

    if(s)
    {
      puts(s);
      free(s);
    }
    else printf("strexpand(\"%s\") failed.\n",*argv);
  }

  return 0;
}

#endif /*TEST*/
