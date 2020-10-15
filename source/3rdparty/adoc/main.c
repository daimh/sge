/*                                                               -*- C -*-
 *  MAIN.C
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

/* $VER: $Id: main.c,v 1.1 2001-07-18 11:05:53 root Exp $ */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <stdio.h>

#include "adoc.h"
#include "flist.h"
#include "libfun.h"
#include "mactab.h"
#include "version.h"
#include "debug.h"

/*@*/

/* local variables */
static char *whoami;
static FILE *ferr;

/* function to export `ferr' to the scanner */
FILE *get_ferr(void) { return ferr; }

/**/

static void echo(const char *fmt, ...)
{
  va_list argp;
  va_start(argp,fmt);

  fprintf(stderr,"%s: ",whoami);
  vfprintf(stderr,(char *)fmt,argp);
  fprintf(stderr,"\n");
  fflush(stderr);

  va_end(argp);
}


/**/

int main(int argc, char *argv[])
{
  int err= 0; /* return code */

  char *outfile    = (char *)0;      /* --output-file       */
  char *errfile    = (char *)0;      /* --error-file        */
  char *headerfile = (char *)0;      /* --texi-header-file  */
  char *yank_type  = "*";            /* --yank-type         */
  char *body_env   = "smallexample"; /* -B<environment>     */

  int page_width         = 80;       /* --page-width        */
  int tabsize            = 8;        /* --tab-size          */
  int tabs_to_spaces     = 0;        /* --tabs-to-spaces    */
  int output_type        = 1;        /* --output-type       */
  int table_of_contents  = 0;        /* --table-of-contents */
  int sort_entries       = 1;        /* --preserve-order    */
  int texi_flags         = TEXI_CREATE_HEADER | TEXI_PARSE_REFERENCES | TEXI_ITEMIZE_REFERENCES;
  int adoc_flags         = ADOC_FORM_FEEDS;
  int warn_mask          = WARN_NORMAL;
  int scanner_flags      = 0;        /* --indented-comments */
                                     /* --unindent-bodytext */
  int minimum_indentation = -1;      /* --reindent-bodytext */

  /* handles for the macro tables */

  int texi_macros = 0;
  int body_macros = 0;

#ifdef _DCC /* Dice */
  expand_args(argc,argv, &argc,&argv);
#endif /* _DCC */

  /* filenames on MS-DOG systems look very ugly: all uppercase and
   * backslashes.  Perform some cosmetics */

#ifdef __MSDOS__
  whoami= "adoc";

#else
  whoami= argv[0];

#endif /*__MSDOS__*/


  /* set the debugging defaults */
  D(bug_init(0,stdout));

  /* initialize the default error stream */
  ferr= stderr;

  if(err == 0)
  {
    /* prepare the texinfo macro table */
    texi_macros= mactab_new( 4+10 );

    if(!texi_macros)
      err= 1;
  }

  if(err == 0)
  {
    /* prepare the body-text macro table */
    body_macros= mactab_new( 2 );

    if(!body_macros)
      err= 2;
  }

  if(err)
    echo("error %d creating macro tables -- not enough memory?", err);

  else

  /* BEGIN scanning command line arguments */

  while( (--argc > 0) && (err <= 0) )
  {
    char *arg= *++argv;

#ifdef DEBUG
    if(argc > 1)  { D(bug("examining command line argument `%s' ( `%s', ... ) [%d]", argv[0], argv[1], argc-1)); }
    else          { D(bug("examining command line argument `%s' ( ) [%d]", argv[0], argc-1)); }
#endif /* DEBUG */

    if(*arg=='-')
    {
      /* remember the original command-line option string */
      char *opt= arg;

      if(arg[1]=='-')
        arg= convert_args(*argv);

      switch(*++arg)
      {

/*-0*/  case '0':
          output_type= 0;
          break;

/*-1*/  case '1':
          output_type= 1;
          break;

/*-2*/  case '2':
          output_type= 2;
          tabs_to_spaces= 1;
          minimum_indentation= 0;
          break;

/*-b*/  case 'b':
          texi_flags |= TEXI_TABLE_FUNCTIONS;
          break;

/*-B*/  case 'B':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            body_env= arg;
          }
          else
          {
            echo("missing texinfo body text environment after %s option",opt);
            err= 1;
          }
          break;

/*-c*/  case 'c':
          err= mactab_add(body_macros, "\\*", "/*", "*\\", "*/", (char *)0);

          if(err)
            echo("error %d adding comment convertion macros",err);
          break;

/*-d*/  case 'd':

#ifdef DEBUG

          if(arg[1]) { D(bug_level= atoi( &(arg[1]) )); }
          else       { D(bug_level= 1); }

#else /* !DEBUG */
          echo("not compiled w/ -DDEBUG.  No debug information available -- Sorry");
          /* no error */

#endif /* DEBUG */

          break;


/*-D*/  case 'D':
          if(arg[1] && --argc > 0)
          {
            char *lhs= &arg[1];
            char *rhs= *(++argv);

            err= mactab_add(texi_macros, lhs, rhs, (char *)0);

            if(err)
              echo("error adding texinfo macro `%s' = `%s'", lhs, rhs);
          }
          else
          {
            echo("missing macro %s after `%s' option",(arg[1] ? "value":"name"),opt);
            err= 1;
          }
          break;

/*-E*/  case 'E':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            if(errfile)
            {
              echo("warning: option `%s' has already been seen", opt);
              D(bug("%s \"%s\" superseeds -E \"%s\"", opt, arg, errfile));
            }

            /*errfile= strcmp(arg,"-") ? arg : (char *)0;*/
            errfile= arg;
          }
          else /* !(arg && *arg) */
          {
            echo("missing filename after `%s' option", opt);
            err= 1;
          }
          break;

/*-f*/  case 'f':
          if(arg[1])
          {
            while(*++arg) switch(*arg)
            {
              case 'f':
                adoc_flags |= ADOC_FORM_FEEDS;
                texi_flags |= TEXI_FUNCTION_NEWPAGE;
                break;

              default:
                echo("unknown paging option: `%s'",opt);
                break;
            }
          }
          else /* !arg[1] */
          {
            adoc_flags &= ~ADOC_FORM_FEEDS;
            texi_flags &= ~TEXI_FUNCTION_NEWPAGE;
          }
          break;

/*-g*/  case 'g':
          if(arg[1])
          {
            while(*++arg) switch(*arg)
            {
              case 's':
                texi_flags |= TEXI_GROUP_SECTIONS;
                break;

              default:
                echo("unknown grouping option: `%s'",opt);
                err= 1;
                break;
            }
          }
          else texi_flags &= ~TEXI_GROUP_SECTIONS;
          break;

/*-H*/  case 'H':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            if(headerfile)
            {
              echo("warning: option `%s' has already been seen", opt);
              D(bug("%s \"%s\" superseeds -H \"%s\"", opt, arg, headerfile));
            }

            headerfile= arg;
          }
          else /* !(arg && *arg) */
          {
            echo("missing texinfo header filename after `%s' option", opt);
            err= 1;
          }
          break;

/*-h*/  case 'h':
          printf("usage: %s [options] [-o outfile] [@ listfile] [infiles...]\n\n", whoami);
          display_args( arg[1] ? atoi(&arg[1]) : 3 );
          err= -1;    /* negative means exit w/o error */
          break;

/*-I*/  case 'I':
          table_of_contents= 1;
          break;

/*-i*/  case 'i':
          yank_type= "i";
          break;

/*-j*/  case 'j':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            if( (minimum_indentation= atoi(arg)) < 0 )
            {
              echo("illegal indentation: %d  (must be >= 0)", minimum_indentation);
              err= 1;
            }
          }
          else /* !(arg && *arg) */
          {
            echo("missing indentation after `%s' option", opt);
            err= 1;
          }
          break;

/*-l*/  case 'l':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            page_width= atoi(arg);

            if(page_width < 1)
            {
              echo("illegal page width: `%s'  (must be > 0)", arg);
              err= 1;
            }
          }
          else /* !(arg && *arg) */
          {
            echo("missing page width after `%s' option", opt);
            err= 1;
          }
          break;

/*-M*/  case 'M':
          if(arg[1] && --argc > 0)
          {
            char *lhs= &arg[1];
            char *rhs= *(++argv);

            err= mactab_add(body_macros, lhs, rhs, (char *)0);

            if(err)
              echo("error adding body macro `%s' -> `%s'", lhs, rhs);
          }
          else
          {
            echo("missing macro %s after `%s' option",(arg[1] ? "value":"name"),opt);
            err= 1;
          }
          break;

/*-n*/  case 'n':
          output_type= 0;
          break;

/*-o*/  case 'o':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            if(outfile)
              echo("warning: option `%s' has already been seen", opt);

            outfile= arg;
          }
          else /* !(arg && *arg) */
          {
            echo("missing filename after `%s' option", opt);
            err= 1;
          }
          break;

/*-p*/  case 'p':
          sort_entries= arg[1] ? 1:0;
          break;

/*-q*/  case 'q':
          break;

/*-T*/  case 'T':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            tabs_to_spaces= 1;
            tabsize= atoi(arg);

            if(tabsize < 1)
            {
              echo("illegal tab step: `%d'  (must be >= 1)", tabsize);
              err= 1;
            }
          }
          else /* !(arg && *arg) */
          {
            echo("missing tab size after `%s' option", opt);
            err= 1;
          }
          break;

/*-t*/  case 't':
          tabs_to_spaces= arg[1] ? atoi(&arg[1]) : 1;
          break;

/*-U*/  case 'U':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0;

          if(arg && *arg)
          {
            mactab_remove(texi_macros, arg, (char *)0);
            mactab_remove(body_macros, arg, (char *)0);
          }

          else /* !(arg && *arg) */
          {
            echo("missing macro after `%s' option", opt);
            err= 1;
          }
          break;

/*-u*/  case 'u':
          if(arg[1])
          {
            scanner_flags &= ~SCANNER_UNINDENT_BODYTEXT;
            minimum_indentation= -1;
          }
          else scanner_flags |= SCANNER_UNINDENT_BODYTEXT;
          break;

/*-v*/  case 'v':
          printf("ADOC Version " VERSION " (compiled " __DATE__ ", " __TIME__ ")\n"
                 "(c)Copyright 1995 by Tobias Ferber,  All Rights Reserved\n" );
          err= -1;
          break;

/*-W*/  case 'W':
          if(arg[1])
          {
            ++arg;

            if( isdigit(*arg) )
              warn_mask |= atoi(arg);

            else switch( strarg(arg, "none",       /* 1 */
                                     "arnings",    /* 2 */
                                     "keywords",   /* 3 */
                                     "absence",    /* 4 */
                                     "untitled",   /* 5 */
                                     "all", "") )  /* 6 */
            {
              case 1:   warn_mask  = WARN_NONE;                 break;
              case 2:   warn_mask |= WARN_NORMAL;               break;
              case 3:   warn_mask |= WARN_UNKNOWN_KEYWORDS;     break;
              case 4:   warn_mask |= WARN_MISSING_KEYWORDS;     break;
              case 5:   warn_mask |= WARN_UNTITLED_SECTION;     break;
              case 6:   warn_mask |= WARN_ALL;                  break;

              default:
                echo("unknown warning method: `%s'",opt);
                err= 1;
                break;
            }
          }
          else warn_mask= WARN_NONE;
          break;

/*-x*/  case 'x':
          if(arg[1])
          {
            switch( strarg(++arg, "off",        /* 1 */
                                  "on",         /* 2 */
                                  "itemize",    /* 3 */
                                  "", "") )     /* 4 */
            {
              case 1:   texi_flags &= ~TEXI_PARSE_REFERENCES;
                        texi_flags &= ~TEXI_ITEMIZE_REFERENCES;   break;

              case 2:   texi_flags |=  TEXI_PARSE_REFERENCES;
                        texi_flags &= ~TEXI_ITEMIZE_REFERENCES;   break;

              case 3:   texi_flags |=  TEXI_PARSE_REFERENCES;
                        texi_flags |=  TEXI_ITEMIZE_REFERENCES;   break;

              default:
                echo("unknown reference handlig option: `%s'",opt);
                err= 1;
                break;
            }
          }
          else texi_flags &= ~(TEXI_PARSE_REFERENCES | TEXI_ITEMIZE_REFERENCES);
          break;


/*-Y*/  case 'Y':
          if(arg[1]) scanner_flags &= ~SCANNER_ALLOW_INDENTED_COMMENTS;
          else       scanner_flags |=  SCANNER_ALLOW_INDENTED_COMMENTS;
          break;

/*-y*/  case 'y':
          if(arg[1]) ++arg;
          else arg= (--argc > 0) ? *(++argv) : (char *)0L;

          if(arg && *arg)
            yank_type= arg;

          else /* !(arg && *arg) */
          {
            echo("missing comment type string after `%s' option", opt);
            err= 1;
          }
          break;

/*-z*/  case 'z':
          texi_flags &= ~TEXI_CREATE_HEADER;
          break;

/*-Z*/  case 'Z':
          if(arg[1]) texi_flags &= ~TEXI_NO_INDEX;
          else       texi_flags |=  TEXI_NO_INDEX;
          break;

          /*
           *  The following options are ignored for compatibility
           *  with Bill Koester's original version `autodoc' which
           *  is part of C=ommodore's Native Developer Kit (NDK).
           */

/*-C*/  case 'C':
/*-F*/  case 'F':
/*-s*/  case 's':
/*-a*/  case 'a':
/*-r*/  case 'r':
/*-w*/  case 'w':
          echo("warning: option `%s' ignored for compatibility", opt);
          break;

/*- */  case '\0':
          if( (err= flist_addfile("")) )
             echo("out of memory... hmmmmmmmmmpf!");
           break;

/*??*/  default:
          echo("unrecognized option `%s'", opt);
          err= 1;
          break;
      }
    }
    else if(*arg=='@')
    {
      if(arg[1]) ++arg;
      else arg= (--argc > 0) ? *(++argv) : (char *)0L;

      if(arg && *arg)
      {
        if( (err= flist_from_file(arg)) )
          echo("out of memory... aaarrrrrrgggggghh!");
      }
      else /* !(arg && *arg) */
      {
        echo("missing filename after `%s'", *argv);
        err= 1;
      }
    }
    else /* *arg != '@' */
    {
      if(arg && *arg)
      {
        if( (err= flist_addfile(arg)) )
          echo("out of memory... aaaiiiiiieeeeeeeee!");
      }
      else echo("internal problem parsing command line arguments: arg is empty");
    }
  }
  /* END scanning command line arguments */
  D(bug("command line argument parsing done"));

  if(err == 0)
  {
    /* prepare the error stream */

    if(errfile && *errfile)
    {
      D(bug("opening error stream `%s'",errfile));

      if( !(ferr= fopen(errfile,"w")) )
      {
        echo("could not write error messages to `%s'",errfile);
        err= __LINE__;
      }
    }
    /*else ferr is initialized to stderr */

    /* if no filename is given then read from stdin */

    if( !flist_getname() )
      flist_addfile("");


    /* read the input files (the scanner takes them from the flist queue) */

    D(bug("reading autodocs of type `%s'", yank_type));

    if(err == 0)
      err= read_source(yank_type, warn_mask, scanner_flags);

    if(err < 0)
      err= -err;  /* I/O error */

    D(bug("disposing file list"));
    flist_dispose();

    /*
     */

    if( (err == 0) && (minimum_indentation >= 0) )
    {
      if( (err= funindent(minimum_indentation, tabsize)) )
        echo("error %d reworking body text indentation -- not enough memory?",err);

      /* funindent() already performed that conversion */
      else tabs_to_spaces= 0;
    }


    if( (err == 0) && (output_type > 0) )
    {
      FILE *fout;

      /* prepare the output file */

      if(outfile && *outfile)
      {
        D(bug("opening output stream `%s'",outfile));

        if(!(fout= fopen(outfile,"w")) )
        {
          echo("could not write to `%s'",outfile);
          err= __LINE__;
        }
      }
      else fout= stdout;


      if( fout && (err==0) )
      {
        if(sort_entries)
        {
          D(bug("sorting entries"));
          funsort();
        }

        switch(output_type)
        {
          case 1: /* --autodoc */

            if(table_of_contents)
            {
              D(bug("writing table of contents"));
              err= gen_autodoc_toc(fout);
            }
            if(err == 0)
            {
              D(bug("writing autodocs"));
              err= gen_autodoc( fout, page_width, tabs_to_spaces ? tabsize : 0, adoc_flags, mactab(body_macros) );
            }
            break;

          case 2: /* --texinfo */
            if(texi_flags & TEXI_CREATE_HEADER)
            {
              D(bug("creating texinfo header"));
              err= gen_texinfo_header( fout, headerfile, mactab(texi_macros) );
            }

            if(err == 0)
            {
              D(bug("adding texinfo body macros"));
              err= mactab_add( body_macros,  "@",        "@@",
                                             "{",        "@{",
                                             "}",        "@}",
                                          /* "...",      "@dots{}", */
                                          /* "TeX",      "@TeX{}",  */
                                             "e.g. ",    "e.g.@: ",
                                             "E.g. ",    "E.g.@: ",
                                             "i.e. ",    "i.e.@: ",
                                             "I.e. ",    "I.e.@: ",   (char *)0 );
            }

            if(err == 0)
            {
              D(bug("creating texinfo output"));
              err+= gen_texinfo( fout, tabs_to_spaces ? tabsize : 0, texi_flags, body_env, mactab(body_macros) );
            }

            if(err)
              echo("error creating texinfo output");
            break;

          default: /* --dry-run */
            break;
        }
      }

      if(fout && (fout != stdout))
        fclose(fout);
    }

    D(bug("disposing libfun entries"));
    funfree();
  }

#ifdef DEBUG
  mactab_debug(bug_stream);
#endif

  D(bug("disposing macro tables"));
  mactab_dispose(body_macros);
  mactab_dispose(texi_macros);

  /*
  */

  if(err > 0)
  {
    echo("[%s] *** Error %d", (outfile && *outfile) ? outfile : "stdout", err);
    fprintf(ferr,"%s terminated abnormally (error %d)\n", whoami, err);
  }

  D(bug("closing I/O streams"));

  if( ferr && (ferr != stderr) && (ferr != stdout) )
    fclose(ferr);

  D(bug("exiting adoc returning %d (%s)", (err>0) ? 1:0, (err>0) ? "error":"success" ));
  D(bug_exit());

#ifdef DEBUG

  if(bug_stream && (bug_stream != stdout))
    fclose(bug_stream);

#endif /*DEBUG*/

  return (err > 0) ? 1:0;
}
