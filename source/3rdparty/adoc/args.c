/*                                                               -*- C -*-
 *  ARGS.C
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

/* $VER: $Id: args.c,v 1.1 2001-07-18 11:05:53 root Exp $ */

#include <string.h>
#include <stdio.h>

static char *howtouse[]= {

  "-",              "--stdin",                  "",            "read source code from stdin",
  "-0",             "--no-output",              "",            "do not write any output (same as `-n')",
  "-1",             "--autodoc",                "",            "create autodoc output (default)",
  "-2",             "--texinfo",                "",            "create texinfo output",
  "-2",             "--texi",                   "",            "",
  "-b",             "--texi-funtable",          "",            "create a @table for each function",
  "-B",             "--body-environment",       "<string>",    "put body text into a @<string> environment",

/*
  "-Bexample",      "--example",                "",            "put body text into an @example environment",
  "-Bsmallexample", "--small-example",          "",            "put body text into a @smallexample environment",
  "-Bsmallexample", "--smallexample",           "",            "",
  "-Bdisplay",      "--display",                "",            "put body text into a @display environment",
*/

  "-c",             "--flip-slashes",           "",            "convert \\* to /* and *\\ to */",
  "-c",             "--convert",                "",            "",

#ifdef DEBUG
  "-d",             "--debug",                  "<number>",    "set debug level to <number>",
#endif

  "-D<macro>",      "",                         "<value>",     "define a texinfo header macro",
  "-DPROJECT",      "--project",                "<string>",    "define a value for the PROJECT macro",
  "-DEDITION",      "--edition",                "<string>",    "define a value for the EDITION macro",
  "-DREVISION",     "--revision",               "<string>",    "define a value for the REVISION macro",
  "-DCOPYRIGHT",    "--copyright",              "<string>",    "define a value for the COPYRIGHT macro",
  "-DAUTHOR",       "--author",                 "<string>",    "define a value for the AUTHOR macro",
  "-E",             "--error-file",             "<filename>",  "redirect error messages (default is stderr)",
  "-f",             "--no-form-feeds",          "",            "no form feeds (^L) or @page between entries",
  "-f",             "--no-ff",                  "",            "",
  "-ff",            "--form-feeds",             "",            "begin a new page for each function",
  "-g",             "--no-grouping",            "",            "no creation of @group ... @end group",
  "-g",             "--no-groups",              "",            "",
  "-gs",            "--group-sections",         "",            "put keyword sections into a @group",
  "-H",             "--texi-header-file",       "<filename>",  "read texinfo header from file <filename>",
  "-h",             "--help",                   "",            "print this short usage information and exit",
  "-i",             "--internal",               "",            "process only internal docs (same as `-yi')",
  "-I",             "--table-of-contents",      "",            "output table of contents before entries",
  "-I",             "--toc",                    "",            "",
  "-j",             "--reindent-bodytext",      "<number>",    "change the indentation of bodytext lines",
  "-j",             "--reindent",               "",            "",
  "-l",             "--line-length",            "<number>",    "set autodoc page width to <number> columns",
  "-l",             "--page-width",             "",            "",
  "-l",             "--width",                  "",            "",
  "-l",             "--columns",                "",            "",
  "-l",             "--cols",                   "",            "",
  "-M<macro>",      "",                         "<value>",     "define a body text macro",
  "-n",             "--dry-run",                "",            "do not write any output (same as `-0')",
  "-n",             "--dryrun",                 "",            "",
  "-o",             "--output-file",            "<filename>",  "set output filename (default is stdout)",
  "-p",             "--preserve-order",         "",            "don't sort autodoc entries",
  "-p",             "--no-sort",                "",            "",
  "-pp",            "--sort-entries",           "",            "sort autodoc entries (default)",
  "-q",             "--quiet",                  "",            "don't print information while processing",
  "-q",             "--silent",                 "",            "",
  "-q",             "--brief",                  "",            "",
  "-t",             "--tabs-to-spaces",         "",            "turn tabs to spaces (same as `-T8')",
  "-T",             "--tab-size",               "<number>",    "set tab step to <number> columns",
  "-T",             "--tabsize",                "",            "",
  "-U",             "",                         "<macro>",     "undefine a macro defined via `-D' or `-M'",
/*
  "-u",             "--unindent-bodytext",      "",            "remove indentation of body text lines",
  "-u",             "--unindent",               "",            "",
*/
  "-uu",            "--preserve-indent",        "",            "do not modify the body text indentation",
  "-uu",            "--preserve-indentation",   "",            "",
  "-v",             "--version",                "",            "print author and version information",
  "-W",             "--no-warnings",            "",            "don't print any warnings",
  "-Wkeywords",     "--check-keywords",         "",            "check section keywords while reading",
  "-Wabsence",      "--check-presence",         "",            "look for missing section keywords",
  "-Wall",          "--pedantic",               "",            "complain about almost everything",
  "-x",             "--no-references",          "",            "print the SEE ALSO section as it is",
  "-xoff",          "--no-xrefs",               "",            "",
  "-xon",           "--parse-references",       "",            "try to create @xref{}s from SEE ALSO",
  "-xitemize",      "--itemize-references",     "",            "put @xref{}s into an @itemize list",
  "-y",             "--yank-type",              "<string>",    "process only comments of type <string>",
  "-y",             "--input-type",             "",            "",
  "-y",             "--comment-type",           "",            "",
  "-Y",             "--indented-comments",      "",            "allow an indentation of autodoc comments",
  "-Y",             "--allow-indentation",      "",            "",
  "-z",             "--no-texi-header",         "",            "ommit writing texinfo header and title page",
  "-Z",             "--no-texi-index",          "",            "ommit function index and table of contents",

#if 0

  /*
   *  The following options are ignored for compatibility
   *  with Bill Koester's original version `autodoc' which
   *  is part of C=ommodore's Native Developer Kit (NDK).
   */

  "-C",             "--slash-asterisk",         "",            "process autodocs starting with /*",
  "-s",             "--semicolon",              "",            "process autodocs starting with a semicolon",
  "-a",             "--asterisk",               "",            "process autodocs starting with an asterisk",
  "-F",             "--tempfile",               "<filename>",  "ignored for compatibility",
  "-r",             "--troff",                  "",            "ignored for compatibility",
  "-w",             "--no-word-wrap",           "",            "ignored for compatibility",

#endif /*0*/

  (char *)0, (char *)0, (char *)0, (char *)0
};

char *
convert_args(char *s)
{
  char **t= howtouse;

  while(*t) {
    if(!strcmp(s,t[1]))
      break;
    t= &t[4];
  }

  return (*t) ? *t : s;
}

void
display_args(int which)
{
  char **t= howtouse;

  while(*t) {
    if(t[3] && *t[3]) {

      if(which & 1)
      {
        printf("%-12s ",t[0]);

        if(which & 2)
          printf("%s ",t[1][0] ? "or":"  ");
      }

      if(which & 2)
        printf("%-20s ",t[1][0] ? t[1] : ((which & 1) ? "": t[0]) );

      if(which & 3)
        printf("%-13s %s\n",t[2],t[3]);
    }
    t= &t[4];
  }
}
