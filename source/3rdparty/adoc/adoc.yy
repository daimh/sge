/*
 *  ADOC.YY  -- Specification of the AUTODOC scanner
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

/* $VER: $Id: adoc.yy,v 1.2 2007-11-02 18:42:53 ernst Exp $ */

%{
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "adoc.h"
#include "libfun.h"
#include "flist.h"
#include "debug.h"

/* In former versions of flex, yywrap had been defined as a macro */

#ifdef yywrap
#undef yywrap
#endif

/*
** local variables
*/

/* scanner's error stream */
static FILE *ferr;

/* current input line (used for warnings and error messages) */
static int scanner_line= 1;

/* set to the line in this file if an error occurs */
static int scanner_error= 0;

/* name of the current input file */
static char *scanner_infile= (char *)0;

static int scanner_flags;

/* pedanticness of warning method */
static int warn_mask;

/* the last function which has been reported by funreport() */
static char *report_fun= (char *)0;

/* string containing allowed characters for the comment type */
static char *fun_type;

static int have_heading= 0;

/*
**  local functions
*/

static void funreport(const char *fmt, ...)
{
  va_list argp;
  va_start(argp,fmt);
  {
    char *fun= getfun( (char *)0 );

    if(fun) {
      if( !report_fun || strcmp(report_fun,fun) )
        fprintf(ferr,"%s: In function `%s':\n",scanner_infile,fun);
    }
    else { /* !fun */
      if(report_fun)
        fprintf(ferr,"%s: At top level:\n",scanner_infile);
    }
    report_fun= fun;
  }
  fprintf(ferr,"%s: %d: ",scanner_infile,scanner_line);
  vfprintf(ferr,(char *)fmt,argp);
  fprintf(ferr,"\n");
  fflush(ferr);

  va_end(argp);
}


/* Check the availability of section keywords in a function description.
   This procedure is called from the scanner at the end of a function */

static void funcheck(int line, ...)
{
  char *fun, *arg;

  va_list argp;
  va_start(argp,line);

  if( (fun= getfun((char *)0)) )
  {
    int missing= 0;

    for(arg= va_arg(argp, char *); arg && *arg; arg= va_arg(argp, char *))
    {
      if( !getsec(arg) )
      {
        if(missing)
          fprintf(ferr," `%s'",arg);

        else /* first missing keyword */
        {
          if( !report_fun || strcmp(report_fun,fun) )
          {
            fprintf(ferr,"%s: In function `%s':\n",scanner_infile,fun);
            report_fun= fun;
          }
          fprintf(ferr,"%s: %d: warning: missing keyword `%s'",scanner_infile,line,arg);
        }

        ++missing;
      }
    }

    if(missing)
      fprintf(ferr,"\n");
  }
  va_end(argp);
}

/* remove white spaces from the left and/or right end of a string `s' */

static char *strip(char *s, char *lr)
{
  char *l, *r, *t;

  if( strchr(lr,'l') )
  {
    /* find the first non-white char `l' in `s' */
    for(l=s; *l==' ' || *l=='\t'; l++) ;
  }
  else l=s;

  if( strchr(lr,'r') )
  {
    /* find the last non-white char `r' in `s' */
    for(t= r= l; *t; t++)
    {
      if(*t!=' ' && *t!='\t' && *t!='\r' && *t!='\n')
        r= t;
    }

    if(r[0])
    {
      if(*r==' ' || *r=='\t' || *r=='\r' || *r=='\n')
        *r= '\0';

      else r[1]= '\0';
    }
  }

  return l;
}

%}

BLACK        [^\n\r\f\t ]
IDENT        [a-zA-Z0-9\-_\.]+
AST          "*"
NL           (\r)|(\n)|(\r\n)|(\n\r)

/* the context has been moved to the rule section
   because of problems with former versions of lex */

FUN_BEGIN    [ \t]*{BLACK}{AST}{3}.{2}{AST}{1}" "
FUN_IDENT    {IDENT}("/"{IDENT})*
FUN_SECTION  [ \t]*{BLACK}[ \t]+[A-Z]{2}[A-Z ]*
FUN_TEXT     [ \t]*{BLACK}(([ \t].*)?)
FUN_END      [ \t]*{BLACK}{AST}{2}

/*
 *  eXclusive scanner modes:
 *
 *  INITIAL   skip source-code
 *  EXPECT    we just read a {FUN_BEGIN} and expect a {FUN_IDENT} now to enter <SKIP> mode
 *  SKIP      skip the rest of the line, then enter <FUN> mode
 *  FUN       collecting data in a comment section.  {FUN_END} switches back to <INITIAL> mode again
 */

%x EXPECT SKIP FUN

%%

<INITIAL>^{FUN_BEGIN}           { register char *s= yytext;

                                  if( (scanner_flags & SCANNER_ALLOW_INDENTED_COMMENTS) || (*s!=' ' && *s!='\t') )
                                  {
                                    while(*s==' ' || *s=='\t')
                                      ++s;

                                    if( strchr(fun_type,s[4]) || strchr(fun_type,s[5]) )
                                    {
                                      D(bug("line %ld: <%c%c>",scanner_line,s[4],s[5]));
                                      BEGIN(EXPECT);
                                    }
                                  }
                                  /* else: skip this indented comment */
                                }

<EXPECT>{FUN_IDENT}             { D(bug("BEGIN function `%s'",yytext));

                                  if( newfun((char *)yytext) )
                                  {
                                    funreport("error adding function `%s' -- not enough memory?",yytext);
                                    return scanner_error= __LINE__;
                                  }
                                  have_heading= 0;
                                  BEGIN(SKIP);
                                }

<EXPECT>.                       {                  BEGIN(INITIAL); }
<EXPECT>{NL}                    { ++scanner_line;  BEGIN(INITIAL); }

<SKIP>.*{NL}                    { yyless(yyleng-1); BEGIN(FUN); }


 /* There had been warnings because of a "dangerous trailing context"
    when using <FUN>^{FUN_SECTION}/{NL} and <FUN>^{FUN_TEXT}/{NL}.

    Note: we currently need the yyless() calls and put back the '\n' into
    the input to make sure the `^' in is recognized afterwards */


<FUN>^{FUN_SECTION}{NL}         { yyless(yyleng-1);

                                  if( (scanner_flags & SCANNER_ALLOW_INDENTED_COMMENTS) || (yytext[0]!=' ' && yytext[0]!='\t') )
                                  {
                                    register char *s, *t;

                                    for(s=yytext; *s==' ' || *s=='\t'; s++)
                                      ;

                                    /* make a temporary copy of yytext */
                                    if( (t=strdup(&s[1])) )
                                    {
                                      /* strip white chars on both sides of yytext */
                                      s= strip(t,"lr");
                                      D(bug("... section `%s'",s));

                                      if( warn_mask & WARN_UNKNOWN_KEYWORDS )
                                      {
                                        if( !strarg(s,"NAME",
                                                      "SYNOPSIS",
                                                      "FUNCTION",
                                                      "INPUTS",
                                                      "RESULT",
                                                      "EXAMPLE",
                                                      "NOTES",
                                                      "BUGS",
                                                      "SEE ALSO", "") )

                                          funreport("warning: unknown section keyword `%s'",s);
                                      }

                                      if( newsec(s) )
                                      {
                                        funreport("error adding section keyword `%s' -- not enough memory?",s);
                                        return scanner_error= __LINE__;
                                      }
                                      have_heading= 1;
                                      free(t);
                                    }
                                    else /* !t */
                                    {
                                      funreport("ran out of memory");
                                      return scanner_error= __LINE__;
                                    }
                                  }
                                  else
                                  {
                                    funreport("disallowed indentation before section heading");
                                    return scanner_error= __LINE__;
                                  }
                                }

<FUN>^{FUN_TEXT}{NL}            { yyless(yyleng-1);

                                  if( (scanner_flags & SCANNER_ALLOW_INDENTED_COMMENTS) || (yytext[0]!=' ' && yytext[0]!='\t') )
                                  {
                                    register char *s, *t;

                                    for(s=yytext; *s==' ' || *s=='\t'; s++)
                                      ;

                                    /* make a temporary copy of yytext and preserve
                                       the correct indentation of the body text */

                                    if( (s[1]=='\t') || (s[1]=='\0') || (s[1]=='\n') )
                                      t= strdup(&s[1]);

                                    else if( (t= strdup(s)) )
                                      t[0]= ' ';

                                    if(t)
                                    {
                                      /* remove (leading and) trailing white spaces - especially the {NL} */
                                      s= strip(t, (scanner_flags & SCANNER_UNINDENT_BODYTEXT) ? "lr":"r");

                                      if(!have_heading && *s)
                                      {
                                        if( newsec("") )
                                        {
                                          funreport("error creating untitled section -- not enough memory?");
                                          return scanner_error= __LINE__;
                                        }
                                        if( warn_mask & WARN_UNTITLED_SECTION )
                                          funreport("warning: untitled section");
                                        have_heading= 1;
                                      }

                                      if( addtext(s) > 0 )
                                      {
                                        funreport("error adding text line -- not enough memory?");
                                        return scanner_error= __LINE__;
                                      }

                                      free(t);
                                    }
                                    else /* !t */
                                    {
                                      funreport("ran out of memory");
                                      return scanner_error= __LINE__;
                                    }
                                  }
                                  else
                                  {
                                    funreport("disallowed indentation before section body");
                                    return scanner_error= __LINE__;
                                  }
                                }

<FUN>^{FUN_END}.*{NL}           { D(bug("END function"));

                                  if( warn_mask & WARN_MISSING_KEYWORDS )
                                  {
                                    funcheck( scanner_line, "NAME",
                                                            "SYNOPSIS",
                                                            "FUNCTION",
                                                            "INPUTS",
                                                            "RESULT",
                                                            "EXAMPLE",
                                                            "NOTES",
                                                            "BUGS",
                                                            "SEE ALSO", "");
                                  }
                                  yyless(yyleng-1);
                                  BEGIN(INITIAL);
                                }

<FUN>^[ \t]*{NL}                { if( warn_mask & WARN_BROKEN_COMMENTS )
                                    funreport("warning: broken autodoc comment");
                                  ++scanner_line;
                                }

<FUN>.                          { if( warn_mask & WARN_STRANGE_TEXT )
                                    funreport("warning: line does not match {FUN_TEXT}");
                                  BEGIN(SKIP);
                                }

<INITIAL,EXPECT,SKIP,FUN>{NL}   { scanner_line++; }
<INITIAL,EXPECT,SKIP,FUN>.      /* eat up everything else in <*> mode */

<EXPECT,SKIP,FUN><<EOF>>        { funreport("unterminated comment at the end of file");
                                  return scanner_error= __LINE__;
                                }

<INITIAL><<EOF>>                { D(bug("%d lines at <<EOF>>",scanner_line));
                                  yyterminate();
                                }

%%

/* This function opens the scanner stream for the file `fname' and
   initializes `scanner_infile' and `scanner_line'.  It is called 
   by read_source() and yywrap().  If USE_YYRESTART is defined then
   we use yyrestart(fp) to initialize `yyin' to the new FILE *.
   otherwise we assign `yyin' the new FILE * and call YY_NEW_FILE. */

static int init_scanner(char *fname)
{

#if !defined(USE_YYRESTART)
  static int num_calls= 0;
#endif

  int result= 0;

  if(fname)
  {
    if(*fname)
    {
      FILE *fp= fopen(fname,"r");

      if(fp)
      {
        scanner_infile= fname;

        if(yyin && (yyin !=stdin))
          fclose(yyin);

#if defined(USE_YYRESTART)
        yyrestart(fp);
#else
        yyin= fp;
        if(++num_calls > 1) YY_NEW_FILE;
#endif
      }
      else /* !fp */
      {
        fprintf(ferr,"scanner error: cannot read from `%s'\n",fname);
        scanner_error= __LINE__;
        result= 1;
      }
    }
    else /* !*fname */
    {
      scanner_infile= "stdin";

      if(yyin && (yyin !=stdin))
        fclose(yyin);

#if defined(USE_YYRESTART)
      yyrestart(stdin);
#else
      yyin= stdin;
      if(++num_calls > 1) YY_NEW_FILE;
#endif
    }
  }
  else /* !fname */
  {
    /* We used to set `scanner_infile' to (char *)0 here but since
     * the <<EOF>> rule which reports the unterminated comment is
     * executed after the yywrap() call, we need to save the
     * filename for this error message. */

    /*scanner_infile= (char *)0;*/

    result= 1;
  }

  /* reset the line counter */
  scanner_line= 1;

#ifdef DEBUG
  if(result == 0)
    D(bug("--> `%s'", scanner_infile));
  else
    D(bug("--> end of scanner input queue"));
#endif /*DEBUG*/

  return result;
}

/* In former versions of flex, yywrap had been defined as a macro */

#ifdef yywrap
#undef yywrap
#endif

/*
 *  yywrap() is called by yylex() each time the EOF is reached.
 *  It returns 0 if there is another file to be scanned or
 *             1 if all scanning is done.
 *  Our yywrap() will take the next file from the flist queue.
 */

int yywrap()
{
  int result= 1;

  if( !scanner_error )
  {
    flist_nextfile();
    result= init_scanner( flist_getname() );
  }

  return result;
}


int read_source(char *_fun_type, int _warn_mask, int flags)
{
  int err= 0;

  D(bug_enter("read_source"));

  /* copy parameters to the local variables */

  fun_type      = _fun_type;
  warn_mask     = _warn_mask;
  scanner_flags = flags;

  /* initialize the error stream */

  ferr= get_ferr();

  /* initialize the scanner */

  if( init_scanner( flist_getname() ) == 0 )
  {
    if(scanner_error == 0)
    {
      /* yylex() returns an integer != 0 if an error occured. */
      err= yylex();
    }
  }
  else
  {
    D(bug("ooops!?  filename queue is empty?"));
  }

  /* errors from yywrap() are returned as negative values */

  if(!err && scanner_error)
    err= -scanner_error;

  D(bug_leave("read_source (%s)",err ? "failed":"successful"));

  return err;
}
