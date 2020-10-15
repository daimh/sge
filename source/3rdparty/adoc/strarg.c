/*
 *  STRARG.C
 *
 *  (c)Copyright 1991 by Tobias Ferber,  All Rights Reserved
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

/* $VER: $Id: strarg.c,v 1.1 2001-07-18 11:05:53 root Exp $ */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/*
 *  strarg() sucht den angebenenen String `key' in der Liste der auf ihn
 *  folgenden Argumente.  Diese Liste muß mit einem "" oder einem (char *)0
 *  enden.  Es wird die Nummer des ersten Arguments zurückgegeben, das
 *  lexikographisch gleich `key' ist.
 *  Das Ergebnis ist 0, wenn `key' nicht in dieser Liste vorkommt.
 */

int strarg(char *key, ...)
{
  int result = 0;
  int gotit  = 0;

  va_list argp;
  va_start(argp,key);

  if(key && *key)
  {
    for(result= gotit= 0; !gotit; result++)
    {
      char *arg= va_arg(argp, char *);

      if( arg )
      {
        if( !strcmp(arg,key) )
          gotit= 1;

        else if( !(*arg) )
          break;
      }
      else break;
    }
  }

  va_end(argp);
  return gotit ? result : 0;
}


#ifdef TEST
#include <stdio.h>

int main(int argc, char *argv[])
{
  if(argc > 1)
    printf( "%d\n", strarg(argv[1], "tobi", "gnu", "foo", "bar", "gnubbel", "42", "blubb", "") );

  return 0;
}

#endif /*TEST*/
