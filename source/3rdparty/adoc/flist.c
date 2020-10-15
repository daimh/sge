/*                                                               -*- C -*-
 *  FLIST.C
 *
 *  (c)Copyright 1994 by Tobias Ferber,  All Rights Reserved
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

/* $VER: $Id: flist.c,v 1.1 2001-07-18 11:05:53 root Exp $ */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "flist.h"

/* My favorite macro */
#define NIL(type) (type)0

#ifdef __GNUC__
/* suggested parentheses around assignment used as truth value */
#define if(assignment)    if( (assignment) )
#endif /* __GNUC__ */

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif /* !MAXPATHLEN */

/****** flist/--background-- *************************************************
*
*	FLIST ist eine auf Dateinamen spezialisierte FIFO Implementierung.
*	Der Haupt-Anwendungsbereich d"urfte wohl im Bereich des Command-Line
*	Parsings liegen, dem Durchwandern der Argumente (Optionen) eines
*	Programms.
*
*	Meist liegen im argv[] Array die Optionen und die Dateinamen in
*	gemischter Form vor.  Da ein Umsortieren von argv[] kritisch ist
*	empfiehlt es sich die Liste der Argumente von links nach rechts zu
*	durchwandern und die Dateinamen in einer separaten Liste zu sammeln.
*	Genau hier setzt FLIST an.
*
******************************************************************************
*
*/

typedef struct fnode {  /* filename node */
  struct fnode *next;   /* next node (command line from left to right) */
  char *filename;       /* the input filename */
  char *hostname;       /* where `filename' appeared, NIL == command-line */
  long hostline;        /* line number in which `filename' appeared */
} fnode_t;

static fnode_t *flist= NIL(fnode_t *),  /* very first node of the list */
               *fcurr= NIL(fnode_t *),  /* head of the remaining sub-list */
               *ftail= NIL(fnode_t *);  /* very last node of the list */

/*
 *  >>> PRIVATE <<<
 */

/****** flist/fnode_new ******************************************************
*
*   NAME
*	fnode_new -- fnode constructor (static)
*
*   SYNOPSIS
*	fn= fnode_new()
*
*	static fnode_t *fnode_new();
*
*   FUNCTION
*
*   SEE ALSO
*	fnode_dispose()
*
******************************************************************************
*
*  Ich habe aus den ****i* xxx/yyy **** Kommentaren der `static functions'
*  normale Kommentare gemacht, damit ich nicht zwei docfiles erstellen mu\3...
*/

static fnode_t *fnode_new(void)
{
  fnode_t *fn;

  DD(bug_enter("fnode_new"));

  fn= (fnode_t *)malloc( sizeof(struct fnode) );

  if(fn)
  {
    fn->next     = NIL(fnode_t *);
    fn->filename =
    fn->hostname = NIL(char *);
    fn->hostline = 0;

  }

  DD(bug_leave("fnode_new (%s)", fn ? "successful":"failed"));

  return fn;
}

/****** flist/fnode_dispose **************************************************
*
*   NAME
*	fnode_dispose -- fnode destructor (static)
*
*   SYNOPSIS
*	nil= fnode_dispose(fn)
*
*	static fnode_t *fnode_dispose(fnode_t *);
*
*   FUNCTION
*
*   SEE ALSO
*	fnode_new()
*
******************************************************************************
*
*/

static fnode_t *fnode_dispose(fnode_t *fn)
{
  DD(bug_enter("fnode_dispose"));

  if(fn)
  {
    if(fn->filename)
    {
      free(fn->filename);
      fn->filename= NIL(char *);
    }

    fn->hostname= NIL(char *);
    free(fn);
  }

  DD(bug_leave("fnode_dispose"));

  return NIL(fnode_t *);
}




/****** flist/flist_addnode **************************************************
*
*   NAME
*	flist_addnode -- F"ugt eine fnode in die flist ein (static)
*
*   SYNOPSIS
*	flist_addnode(fn)
*
*	static void flist_addnode(fnode_t *);
*
*   FUNCTION
*	Diese Funktion h"angt die gegebene fnode `fn' an die Liste der fnode
*	Strukturen an und aktualisiert `flist', `fcurr' und `ftail'.
*
*
*   SEE ALSO
*	flist_addfile(), flist_from_file()
*
******************************************************************************
*
*/

static void flist_addnode(fnode_t *fn)
{
  DD(bug_enter("flist_addnode"));

  if(fn)
  {
    if(!flist)  flist= fn;
    if(!fcurr)  fcurr= fn;

    if(ftail)
      ftail->next= fn;

    ftail= fn;
  }

  DD(bug_leave("flist_addnode"));
}


/****** flist/fnode_from_file ************************************************
*
*   NAME
*	fnode_from_file -- Erzeugt eine fnode aus dem n"achsten Dateinamen
*
*   SYNOPSIS
*	fn= fnode_from_file(fp);
*
*	fnode_t *fnode_from_file(FILE *);
*
*   FUNCTION
*	Diese Funktion legt eine neue fnode Struktur an und liest deren
*	Auspr"agung aus dem Stream `fp' ein.  Dabei werden die Felder
*	`filename' und `hostline' gesetzt, der hostname jedoch nicht.
*	Ein Aufruf dieser Funktion mit NIL(FILE *) als Argument f"uhrt dazu,
*	da\3 der line Counter auf 1 zur"uckgesetzt wird.
*
*   INPUTS
*	fp	      - Der Stream aus dem der n"achste Dateiname gelesen
*			werden soll
*
*   RESULT
*	fn	      - Ein Zeiger auf die erstellte fnode Struktur
*
*   SEE ALSO
*	flist_from_file()
*
******************************************************************************
*
*/

static fnode_t *fnode_from_file(FILE *fp)
{
  static long line= 1;
  fnode_t *fn= NIL(fnode_t *);

  typedef enum { outer_mode,
                 word_mode,
                 string_mode,
                 return_mode,
                 error_mode
               } smode_t;

  smode_t smode= outer_mode;

  DD(bug_enter("fnode_from_file"));

  if(fp) /* -> neue fnode anlegen und Auspr"agung einlesen */
  {
    if( fn= fnode_new() )
    {
      char *fname= (char *)malloc( MAXPATHLEN * sizeof(char) );

      if( fname )
      {
        int c, n=0;
        fn->filename= fname;

        while( smode != return_mode && smode != error_mode && !feof(fp) )
        {
          c= fgetc(fp);

          if(feof(fp) && c!=EOF)
            c= EOF;

          switch(c)
          {
            case ' ': case '\t':
              switch(smode)
              {
                case word_mode:
                  fname[n++]= '\0';
                  smode= return_mode;
                  break;

                case string_mode:
                  fname[n++]= c;
                  break;

                case outer_mode:
                case return_mode:
                case error_mode:
                  break;
              }
              break;

            case '\n': case '\r':
              switch(smode)
              {
                case word_mode:
                  fname[n++]= '\0';
                  smode= return_mode;
                  break;

                case string_mode:
                  sprintf(fname,"%ld: unterminated string at EOL; missing quotes `\"'",line);
                  smode= error_mode;
                  break;

                case outer_mode:
                case return_mode:
                case error_mode:
                  break;
              }
              line++;

              { int d= fgetc(fp);
                if(!( (c=='\n' && d=='\r') || (c=='\r' && d=='\n') ))
                  ungetc(d,fp);
              }
              break;

            case '\"':
              switch(smode)
              {
                case outer_mode:
                  smode= string_mode;
                  fn->hostline= line;
                  break;

                case string_mode:
                  fname[n++]= '\0';
                  smode= return_mode;
                  break;

                case word_mode:
                  fname[n++]= '\0';
                  ungetc(c,fp);
                  smode= return_mode;
                  break;

                case return_mode:
                case error_mode:
                  break;
              }
              break;

            case EOF:
              switch(smode)
              {
                case word_mode:
                  if( feof(fp) )
                  {
                    fname[n++]= '\0';
                    smode= return_mode;
                  }
                  else fname[n++]= c;
                  break;

                case string_mode:
                  if( feof(fp) )
                  {
                    sprintf(fname,"%ld: unterminated string at EOF; missing quotes `\"'",line);
                    smode= error_mode;
                  }
                  else fname[n++]= c;
                  break;

                case outer_mode:
                case return_mode:
                case error_mode:
                  break;
              }
              break;

            default:
              switch(smode)
              {
                case outer_mode:
                  smode= word_mode;
                  fn->hostline= line;
                  /* fall through */

                case word_mode:
                case string_mode:
                  fname[n++]= c;
                  break;

                case return_mode:
                case error_mode:
                  break;
              }
              break;
          }

          if(n >= MAXPATHLEN)
          {
            sprintf(fname,"%ld: line too long",line);
            smode= error_mode;
          }

        } /* wend */

        if(smode != error_mode)
          fname[n]= '\0';
        else
          fn->hostline= 0;
      }
      else /* !fname */
        fn= fnode_dispose(fn);
    }
  }
  else /* !fp || !fname */
    line= 1;

  DD(bug_leave("fnode_from_file"));

  return fn;
}


/*
 *  >>> PUBLIC <<<
 */


/****** flist/flist_dispose **************************************************
*
*   NAME
*	flist_dispose -- Gibt die Liste der Dateinamen frei
*
*   SYNOPSIS
*	flist_dispose()
*
*	void flist_dispose(void);
*
*   FUNCTION
*
*   SEE ALSO
*	flist_addfile(), flist_from_file()
*
******************************************************************************
*
*/

void flist_dispose(void)
{
  D(bug_enter("flist_dispose"));

  while(flist)
  {
    fnode_t *t= flist;
    flist= flist->next;

    (void)fnode_dispose(t);
  }

  fcurr=
  ftail= NIL(fnode_t *);

  D(bug_leave("flist_dispose"));
}


/****** flist/flist_addfile **************************************************
*
*   NAME
*	flist_addfile -- F"ugt einen Dateinamen in die Fileliste ein
*
*   SYNOPSIS
*	error = flist_addfile(filename)
*
*	int flist_addfile(char *);
*
*   FUNCTION
*	Diese Funktion h"angt einen Dateinamen (wie er z.B. von der command
*	line kommt) an das Ende der Liste der Dateinamen an.
*
*   INPUTS
*	filename      - Der Name der Datei
*
*   RESULT
*	error	      - 0, wenn alles geklappt hat
*			1, wenn nicht genug Speicher verf"ugbar war
*
*   SEE ALSO
*	flist_dispose()
*
******************************************************************************
*
*/

int flist_addfile(char *fname)
{
  fnode_t *fn= NIL(fnode_t *);

  D(bug_enter("flist_addfile \"%s\"",fname));

  if(fname)
  {
    if( fn= fnode_new() )
    {
      fn->hostname = NIL(char *);
      fn->hostline = 0;

      fn->filename = strdup(fname);

      if(fn->filename)
        flist_addnode(fn);

      else fn= fnode_dispose(fn);
    }
  }

  D(bug_leave("flist_addfile (%s)", fn ? "successful":"failed"));

  return fn ? 0:1;
}


/****** flist/flist_from_file ************************************************
*
*   NAME
*	flist_from_file -- Baut eine Liste aus den Dateinamen in einem File
*
*   SYNOPSIS
*	error= flist_from_file(filename);
*
*	int flist_from_file(char *);
*
*   FUNCTION
*	Diese Funktion h"angt alle in der angegebenen Datei aufgef"uhrten
*	Dateinamen an die Fileliste an.
*
*   INPUTS
*	filename      - Der Name der Datei mit den Filenamen
*
*   RESULT
*	error	      - 0, wenn alles geklappt hat
*			1, im Fehlerfall
*
*   SEE ALSO
*	flist_dispose()
*
******************************************************************************
*
*/

int flist_from_file(char *host)
{
  int rc= 0;
  FILE *fp;

  D(bug_enter("flist_from_file \"%s\"",host));

  if( (fp= fopen(host,"r")) )
  {
    fnode_t *fn;

    /* reset the line counter */
    fnode_from_file( NIL(FILE *) );

    while( !rc && !feof(fp) && !ferror(fp) )
    {
      if( fn= fnode_from_file(fp) )
      {
        if(fn->hostline > 0)
        {
          if(fn->filename && *fn->filename)
          {
            fn->hostname= host;
            flist_addnode(fn);
          }
        }
        else if(fn->filename && *fn->filename)
        {
          /* fn->filename contains an error message */
          fprintf(stderr,"%s: %s\n",host,fn->filename);
          rc= 1;
        }
        else fn= fnode_dispose(fn); /* EOF */
      }
      else /* !fn */
      {
        fprintf(stderr,"%s: %ld: ran out of memory!\n", host, fn->hostline);
        rc= 2;
      }
    }
  }
  else perror(host);

  D(bug_leave("flist_from_file (%s)", rc ? "failed":"successful"));

  return rc;
}

/**/

char *flist_getname(void)
{
  D(bug("flist_getname()"));
  return fcurr ? fcurr->filename : NIL(char *);
}


char *flist_gethost(void)
{
  D(bug("flist_gethost()"));
  return fcurr ? fcurr->hostname : NIL(char *);
}


long flist_gethostline(void)
{
  D(bug("flist_gethostline()"));
  return fcurr ? fcurr->hostline : 0L;
}

int flist_nextfile(void)
{
  D(bug("flist_nextfile()"));

  if(fcurr)
    fcurr= fcurr->next;

  return fcurr ? 1:0;
}


#if defined(DEBUG) || defined(TEST)

void flist_print(FILE *fp)
{
  fnode_t *fn= flist;

  D(bug_enter("flist_print"));

  if(fn)
  {
    int n;
    fprintf(fp,"list of input files:\n");

    for(n=0; fn; n++, fn= fn->next)
      fprintf(fp,"\t%s:%ld: \"%s\"\n",
        (fn->hostname ? fn->hostname
                      : "command-line"), fn->hostline, fn->filename);
  }
  else fprintf(fp,"list of input files is empty\n");

  D(bug_leave("flist_print"));
}
#endif /* DEBUG || TEST */


#ifdef TEST


int main(int argc, char **argv)
{
  int rc= 0;
  char *whoami= argv[0];

  while(--argc > 0 && !rc)
  {
    char *arg= *++argv;
    if(*arg == '@')
    {
      if(arg[1]) ++arg;
      else arg= (--argc > 0) ? *(++argv) : (char *)0L;

      if(arg && *arg)
      {
        if( rc= flist_from_file(arg) )
          fprintf(stderr,"%s: reading filenames from `%s' failed\n",whoami,arg);
      }
      else
      {
        fprintf(stderr,"%s: missing filename after `%s'\n",whoami,*argv);
        rc= 1;
      }
    }
    else /* !@... */
    {
      if( rc= flist_addfile(arg) )
        fprintf(stderr,"%s: out of memory... aaaiiiiiieeeeeeeee!\n",whoami);
    }
  }

  if(!rc)
    flist_print(stdout);

  flist_dispose();

  if(rc) fprintf(stderr,"%s: [%s] *** Error %d\n"
                        "%s terminated abnormally (error %d)\n", whoami, *argv, rc, whoami, rc);

  return rc;
}

#endif /* TEST */
