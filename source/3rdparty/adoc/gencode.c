/*                                                               -*- C -*-
 *  GENCODE.C
 *
 *  (c)Copyright 1995 by Tobias Ferber,  All Rights Reserved.
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

/* $VER: $Id: gencode.c,v 1.3 2002-03-12 12:44:44 joga Exp $ */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "libfun.h"
#include "adoc.h"

/* texinfo sepeator for library and function name (used in @node and @xref) */

/* makeinfo has a problem when using "." or ":" or "/" here... */
#define CHAPSEC " "

/* convert tabs in `s' to spaces with a tab step of `ts' */

static void fexpand(FILE *fp, int ts, char *s)
{
  int column= 0;

  while(*s)
  {
    switch(*s)
    {
      case '\n':
        fputc(*s,fp);
        column= 0;
        break;

      case '\t':
        /*do { fputc(' ',fp); } while( (++column) % ts);*/
        do { fputc(' ',fp); ++column; } while(column % ts);
        break;

     default:
       fputc(*s,fp);
       ++column;
       break;
    }
    ++s;
  }
}


/*
**  AUTODOCS
*/

int gen_autodoc_toc(FILE *fp)
{
  char *fun;

  fprintf(fp,"TABLE OF CONTENTS\n\n");

  for(fun= stepfun(0); fun; fun= stepfun(1))
    fprintf(fp,"%s\n",fun);

  return 0;
}


int gen_autodoc(FILE *fp, int cols, int tabsize, int flags, char **body_macros)
{
  int err= 0;

  char *fun, *sec, *text;

  for(fun= stepfun(0); fun && (err==0); fun= stepfun(1))
  {
    if(flags & ADOC_FORM_FEEDS)
      fputc('\f',fp);

    fprintf(fp,"%s%*s\n\n",fun,(int)(cols-strlen(fun)),fun);

    for(sec= stepsec(0); sec && (err==0); sec= stepsec(1))
    {
      /* indent the section heading with 3 spaces */
      if(*sec)
        fprintf(fp,"   %s\n",sec);

      if( (text= getsec(sec)) )
      {
        if(text && *text)
        {
          char *x= strexpand(text, body_macros);

          if(x)
          {
            if(tabsize > 0)
              fexpand(fp,tabsize,x);

            else
              fputs(x,fp);

            free(x);
          }
          else err= __LINE__;
        }

        fputc('\n',fp);
      }
    }
  }

  return err;
}


/*
**  TEXINFO
*/


/* split `fun' into `chapter' and `section' and `subsection'
   the string returned by this function has to be disposed via free() by the caller */

static char *chapsec(char *fun, char **chapter, char **section, char **subsection)
{
  char *c= strdup(fun);
  char *s= (char *)0;
  char *ss= (char *)0;

  if(c)
  {
    if( (s= strchr(c,'/')) ) {
      *s++= '\0';
      if( (ss = strchr(s, '/')) ) {
         *ss++= '\0';
      }
    }  
  }

  *chapter= c;
  *section= s;
  *subsection = ss;

  return c;
}

/*
 *  The implementation of references in Texinfo are perfectly silly :'(
 *  Instead of offering one simple reference macro, Texinfo comes up
 *  with context-sensitive @ref{}, @xref{}, @pref{}, ... and prints
 *  a `See ...' or `See also ...' right in front of it...
 */

static int see_also(FILE *fp, char *refs, int flags)
{
  int err= 0;
  char *buf= strdup(refs);

#define isref(c) ( ('A'<=(c) && (c)<='Z') || \
                   ('a'<=(c) && (c)<='z') || \
                   ('0'<=(c) && (c)<='9') || ((c)=='_') || ((c)=='-') || ((c)=='/') || ((c)=='.') )

  if(buf)
  {
    /* make `s' the working pointer in `buf' */
    char *s= buf;

    /*
     *  former revisions of ADOC used to indent the parsed references
     *  by the same amount of white space as the first.  Actually we
     *  do not indent them amymore, so out it goes....
     */

    /* indentation string of the first reference */
#if 0
    char *indent= (char *)0;
#endif

    int num_refs= 0;

    while( *s && (err==0) )
    {
      char *l, *r;

      /* move `l' and `r' to the left and right end of a reference in `s' */

      for(l=s; *l && !isref(*l); l++) ;
      for(r=l; *r &&  isref(*r); r++) ;

      /* terminate the reference string with a '\0' */
      if(*r) *r++= '\0';

      /* save the indentation of the first reference */
#if 0
      if(num_refs == 0)
      {
        if( (indent= strdup(s)) )
        {
          char *t= indent;

          while(*t==' ' || *t=='\t')
            ++t;

          *t= '\0';
        }
        else err= __LINE__;
      }
#endif

      /* move `s' behind the reference */
      s= r;

      if( *l && (err==0) )
      {
        /* look for a function `l' and initialize `fun' to it's name */
        char *fun= (char *)0;

/*fprintf(stderr,"--> @ref{%s} ?\n",l);*/

        if( getfun(l) )
          fun= strdup(l);

        else if( !strchr(l,'/') )
        {
	  if( islib(l) )
	    fun= strdup(l);

	  else
	  {
            /*
             *  Okay, we tried it the easy way but perhaps this is a reference
             *  into the library without the library name in front of it.
             *  Let's try appending `l' to the the current library name...
             */

            char *f= getfun( (char *)0 );

            if(f)
            {
              char *x, *xl, *xr, *xss;

              if( (x= chapsec(f, &xl, &xr, &xss)) )
              {
                size_t len= strlen(xl) + 1 + strlen(l) + 1;
                char *y= (char *)malloc( len * sizeof(char) );

                if(y)
                {
                  sprintf(y,"%s/%s",xl,l);
 
/*fprintf(stderr,"--> @ref{%s} ?\n",y);*/

                  if( getfun(y) )
                    fun= strdup(y);

                  free(y);
                }
                else err= __LINE__;

                free(x);
              }
              else err= __LINE__;
            }
            else /* no current function? */
              err= __LINE__;
	  }
        }

        if(err == 0)
        {
          /* print the reference */

          if( fun )
          {
            char *cs, *chapter, *section, *subsection;

            if( (cs= chapsec(fun, &chapter, &section, &subsection)) )
            {
              if(flags & TEXI_ITEMIZE_REFERENCES)
              {
                if(num_refs==0)
                  fprintf(fp,"@itemize\n");
      if(subsection) {
         fprintf(fp, "@item\n@ref{%s%s%s%s%s}.\n", chapter,CHAPSEC,section,CHAPSEC,subsection); 
      } else {   
		   if(section) {
            fprintf(fp,"@item\n@ref{%s%s%s}.\n",chapter,CHAPSEC,section);
         } else {
            fprintf(fp,"@item\n@ref{%s}.\n",chapter);
         }   
      }
      }
              else /* not itemized */
              {
                if( (num_refs==0) && (flags & TEXI_GROUP_SECTIONS) )
                  fprintf(fp,"@group\n");

		fprintf(fp,"%s",(num_refs==0) ? "@*@xref" : "@ref");

      if(subsection) {
		   fprintf(fp,"{%s%s%s%s%s},\n",chapter,CHAPSEC,section,CHAPSEC,subsection);
      } else {   
		   if(section) {
            fprintf(fp,"{%s%s%s},\n",chapter,CHAPSEC,section);
         } else {
            fprintf(fp,"{%s},\n",chapter);
         }   
      }
              }

              free(cs);
            }
            else err= __LINE__;

            free(fun);
          }
          else /* !fun */
          {
            if(flags & TEXI_ITEMIZE_REFERENCES)
            {
              if(num_refs==0)
                fprintf(fp,"@itemize\n");

              fprintf(fp,"@item\nSee @file{%s}\n",l);
            }
            else /* not itemized */
            {
              if( (num_refs==0) && (flags & TEXI_GROUP_SECTIONS) )
                fprintf(fp,"@group\n");

              fprintf(fp,"%s{%s},\n",((num_refs==0) ? "@*See @file":"@file"),l);
            }
          }

          /* now at least one reference is printed */
          ++num_refs;
        }
      }
    }

    if(num_refs > 0)
    {
      if(flags & TEXI_ITEMIZE_REFERENCES)
        fprintf(fp,"@end itemize\n");

      else /* not itemized */
      {
        if(flags & TEXI_GROUP_SECTIONS)
          fprintf(fp,"@end group\n");

        fprintf(fp,"for more information.\n");
      }
    }

#if 0
    if(indent)
      free(indent);
#endif

    free(buf);
  }
  else err= __LINE__;

#undef isref

  return err;
}


int gen_texinfo_header(FILE *fp, char *fname, char **header_macros)
{
  int err= 0;

  char *default_header=
    "\\input texinfo  @c -*-texinfo-*-\n"
    "@comment %%**start of header\n"
    "@setfilename PROJECT.guide\n"
    "@settitle Autodocs for @code{PROJECT}\n"
    "@paragraphindent 0\n"
    "@iftex\n"
    "@afourpaper\n"
    "@finalout\n"
    "@setchapternewpage on\n"  /* odd */
    "@end iftex\n"
    "@comment %%**end of header\n\n"
    "@ifinfo\n"
    "@node Top\n"
    "@top\n"
    "This document describes @code{PROJECT} version REVISION.\n\n"
    "@noindent Copyright @copyright{} COPYRIGHT\n"
    "@end ifinfo\n\n"
    
    "@titlepage\n"
    "@title PROJECT\n"
    "@subtitle Documentation taken from source code\n"
    "@subtitle Edition EDITION for Version REVISION\n"
    "@subtitle @today\n"
    "@author AUTHOR\n\n"
    "@page\n"
    "@vskip 0pt plus 1filll\n"
    "Copyright @copyright{} COPYRIGHT\n"
    "@end titlepage\n"
    "@headings double\n\n";

  if(fname && *fname)
  {
    FILE *fh= fopen(fname,"r");

    if(fh)
    {
      char *header= (char *)0;
      size_t header_size= 0;

      /* compute the size of the header file */

#ifdef BUGGY_FTELL

      do {

        (void)fgetc(fh);

        if(!feof(fh))
          ++header_size;

      } while(!feof(fh) || ferror(fh))

#else /* ftell() works fine */

      if( fseek(fh,0L,2L) >= 0) /* 2 == OFFSET_END */
        header_size= ftell(fh);

      else
        err= __LINE__;

#endif /* BUGGY_FTELL */

      if(!ferror(fh) && header_size > 0)
      {
        if(fseek(fh,-header_size,1L) < 0) /* 1 == OFFSET_CURRENT */
          err= __LINE__;
      }
      else err= __LINE__;


      /* load the header */

      if(err == 0)
      {
        if( (header= (char *)malloc( (header_size + 1) * sizeof(char) )) )
        {
          fread(header, sizeof(char), header_size, fh);
          header[header_size]= '\0';

          if( ferror(fh) )
          {
            free(header);
            header= (char *)0;
            err= __LINE__;
          }
        }
        else err= __LINE__;
      }

      if(err == 0)
      {
        char *x= strexpand(header, header_macros);

        if(x)
        {
          fputs(x,fp);
          free(x);
        }
        else /* out of memory */
          err= __LINE__;
      }

      fclose(fh);
    }
  }

  else /* !fname */
  {
    char *x= strexpand(default_header, header_macros);

    if(x)
    {
      fputs(x,fp);
      free(x);
    }
    else /* out of memory */
      err= __LINE__;
  }

  return err;
}



int gen_texinfo(FILE *fp, int tabsize, int flags, char *body_environment, char **body_macros)
{
  int err= 0;
  char *fun, *sec;

  /* main menu:
     create a @menu with an entry for each library.
     an additional entry will be created for the function index */

  if(err == 0)
  {
    char *last_chapter = (char *)0;  /* last printed chapter */
    char *last_section = (char *)0;  /* last printed section */

    fprintf(fp,"@ifnotinfo\n@node Top\n@end ifnotinfo\n@menu\n");

    for(fun= stepfun(0); fun && (err==0); fun= stepfun(1))
    {
      char *cs, *chapter, *section, *subsection;

      if( (cs= chapsec(fun, &chapter, &section, &subsection)) )
      {
/*         fprintf(stderr, "chapter = %s\n", chapter ? chapter : "null"); */
/*         fprintf(stderr, "last_chapter = %s\n", last_chapter ? last_chapter : "null"); */
/*         fprintf(stderr, "section = %s\n", section ? section : "null"); */
/*         fprintf(stderr, "subsection = %s\n\n", subsection ? subsection : "null"); */
        
        /* new chapter */
        if( !last_chapter || strcmp(last_chapter, chapter) )
        {
           fprintf(fp,"* %s::\n", chapter);

           if(last_chapter) 
              free(last_chapter);
           last_chapter = strdup(chapter);

           if(last_section) {
              free(last_section);
              last_section = 0;
           }
        } 
        
        /* new section */
        if ( section && ( !last_section || strcmp(last_section, section) ) ) {
           fprintf(fp, "* %s %s::\n", chapter, section);

           if(last_section) 
              free(last_section);
           last_section = strdup(section);
        }

        free(cs);
      }
      else /* out of memory */
        err= __LINE__;
    }

    if( !(flags & TEXI_NO_INDEX) )
      fprintf(fp, "\n"
                  "* Function Index::\n");

    fprintf(fp,"@end menu\n\n");

    if(last_section) {
      free(last_section);
    }

    if(last_chapter) {
      free(last_chapter);
    }
  }

/* fprintf(stderr, "done with menu\n\n"); fflush(fp); */

#if 0

  /* create one single (flat) menu for functions mapped to
     @chapters.  This code is obsolete since we introduced
     the hierarchical layout with libraries mapped to
     @chapters and functions mapped to @sections. */

  if(err == 0)
  {
    fprintf(fp,"@menu\n");

    for(fun= stepfun(0); fun && (err==0); fun= stepfun(1))
    {
      char *cs, *chapter, *section;

      if( (cs= chapsec(fun, &chapter, &section)) )
      {
        fprintf(fp,"* %s %s::\n", chapter, section);
        free(cs);
      }
      else /* out of memory */
        err= __LINE__;
    }

    if( !(flags & TEXI_NO_INDEX) )
      fprintf(fp, "\n"
                  "* Function Index::\n");

    fprintf(fp,"@end menu\n\n");
  }

#endif

  /* chapters & sections */

  if(err == 0)
  {
    char *last_chapter = (char *)0;
    char *last_section = (char *)0;

    for(fun= stepfun(0); fun && (err==0); fun= stepfun(1))
    {
      char *cs, *chapter, *section, *subsection;

      if( (cs= chapsec(fun, &chapter, &section, &subsection)) )
      {
        int newpage = 0;
        char *function = (char *)0;
        if(subsection) {
           function = subsection;
        } else {
           if(section) {
              function = section;
           } else {
              function = chapter;
           }
        }

           
/*         fprintf(stderr, "chapter = %s\n", chapter ? chapter : "null"); */
/*         fprintf(stderr, "last_chapter = %s\n", last_chapter ? last_chapter : "null"); */
/*         fprintf(stderr, "section = %s\n", section ? section : "null"); */
/*         fprintf(stderr, "subsection = %s\n", subsection ? subsection : "null"); */
/*         fprintf(stderr, "function = %s\n\n", function); */

        if( !last_chapter || strcmp(last_chapter, chapter) )
        {
          newpage = 1;
          /*
           *  Start a new chapter
           */

          fprintf(fp, "\n"
                      "@node %s\n"
                      "@chapter %s\n\n"
                      ,chapter
                      ,chapter
          );

          /* print a sub menu for the current chapter */

/*          pushfun();
          {
            int done;
            char *sub_fun;

            fprintf(fp,"@menu\n");
            for(sub_fun= fun, done= 0; sub_fun && (err==0) && !done; sub_fun= stepfun(1))
            {
              char *sub_cs, *sub_chapter, *new_section, *new_subsection;

              if( (sub_cs= chapsec(sub_fun, &sub_chapter, &new_section, &new_subsection)) )
              {
                if( (done= strcmp(sub_chapter, chapter)) == 0 )
                  fprintf(fp,"* %s %s::\n", sub_chapter, new_section);

                free(sub_cs);
              }
              else err= __LINE__;
            }
            fprintf(fp,"@end menu\n\n");
          }
          popfun();
*/
          if(last_chapter)
             free(last_chapter);
          last_chapter = strdup(chapter);

          if(last_section)
             free(last_section);
          last_section = 0;   
        }

        if(!newpage) /* we didn't start a new page for the chapter */
        {
          if(flags & TEXI_FUNCTION_NEWPAGE)
            fprintf(fp,"@page\n");
        }

        if(section && ( !last_section || strcmp(last_section, section) ) )
        {
          /*
           *  Start a new section
           */

          fprintf(fp, "\n"
                      "@node %s%s%s\n"
                      "@section %s\n\n"
                      ,chapter, CHAPSEC, section
                      ,section
          );

          /* print a sub menu for the current chapter */

/*          pushfun();
          {
            int done;
            char *sub_fun;

            fprintf(fp,"@menu\n");
            for(sub_fun= fun, done= 0; sub_fun && (err==0) && !done; sub_fun= stepfun(1))
            {
              char *sub_cs, *new_section, *sub_section, *new_subsection;

              if( (sub_cs= chapsec(sub_fun, &new_section, &sub_section, &new_subsection)) )
              {
                if( (done= strcmp(new_section, section)) == 0 )
                  fprintf(fp,"* %s %s::\n", new_section, new_subsection);

                free(sub_cs);
              }
              else err= __LINE__;
            }
            fprintf(fp,"@end menu\n\n");
          }
          popfun();
*/
          if(last_section)
             free(last_section);
          last_section = strdup(section);
        }


        /*
         *  function on 3rd hierarchy
         */

        if(subsection) {
           fprintf(fp, "@node %s%s%s%s%s\n"
                       "@subsection %s\n\n"
                       ,chapter,CHAPSEC,section,CHAPSEC,subsection
                       ,subsection
           );            
        }
   
          fprintf(fp, "\n"
                      "@findex %s\n"
                      "\n"
                      ,function
          );

        if(flags & TEXI_TABLE_FUNCTIONS)
          fprintf(fp,"@table @b\n");

        for(sec= stepsec(0); sec && (err == 0); sec= stepsec(1))
        {
          char *text= getsec(sec);

          if(text && *text)
          {
            if(flags & TEXI_TABLE_FUNCTIONS)
              fprintf(fp,"@item %s\n",sec);
            else
             fprintf(fp,"@b{%s}\n",sec);  /* was @strong{} or @code{} */
             /*fprintf(fp,"@noindent\n@b{%s}\n",sec);*/


            if( (flags & TEXI_PARSE_REFERENCES) && (strcmp(sec,"SEE ALSO") == 0) )
              err= see_also(fp, text, flags);

            else /* ! "SEE ALSO" */
            {
              char *x= strexpand(text,body_macros);

              if(x)
              {
                fprintf(fp,"@noindent\n"
                           "@%s\n",body_environment);

                if(flags & TEXI_GROUP_SECTIONS)
                  fprintf(fp,"@group\n");

                if(tabsize > 0)
                  fexpand(fp,tabsize,x);
                else
                  fputs(x,fp);

                if(flags & TEXI_GROUP_SECTIONS)
                  fprintf(fp,"@end group\n");

                fprintf(fp,"@end %s\n"
                           "@refill\n",body_environment);

                free(x);
              }
              else /* out of memory */
                err= __LINE__;
            }
          }
          /* else (!text) -> no error */

          fprintf(fp,"\n");
        }

        if(flags & TEXI_TABLE_FUNCTIONS)
          fprintf(fp,"@end table\n");

         free(cs);
      }
      else /* out of memory */
        err= __LINE__;
    }

    if(last_chapter)
       free(last_chapter);

    if(last_section)
       free(last_section);
  }


  /* index */

  if(err == 0)
  {
    if( !(flags & TEXI_NO_INDEX) )
      fprintf(fp, "@node Function Index\n"
                  "@unnumbered Function Index\n"
                  "@printindex fn\n"
                  "\n"
                  "@page\n"
                  "@contents\n"
                  "@bye\n");
  }

  return err;
}
