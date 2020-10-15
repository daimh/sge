/* 
 * Motif Tools Library, Version 3.1
 * $Id: Xbm.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Xbm.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <Xmt/Xmt.h>
#include <Xmt/Xbm.h>

/*
 * The Xbm file parser in this file is based heavily on code in 
 * the Xmu library.  See XCOPYRIGHT for copyright information.
 * $XConsortium: RdBitF.c,v 1.8 91/03/09 16:27:55 rws Exp $
 * Based on an optimized version provided by Jim Becker, Auguest 5, 1988.
 */

#define MAX_SIZE 255

/*
 * we need a stream type that can be either a string or a file
 */
typedef struct {
    _Xconst char *s;
    FILE *f;
} Stream;

/*
 *	read next hex value in the input stream, return -1 if EOF
 */
#if NeedFunctionPrototypes
static int NextInt (Stream *stream)
#else
static int NextInt (stream)
Stream *stream;
#endif
{
    int	ch;
    int	value = 0;
    int gotone = 0;
    int done = 0;
    int digit;
    
    /* loop, accumulate hex value until find delimiter  */
    /* skip any initial delimiters found in read stream */

    while (!done) {
	if (stream->f)
	    ch = getc(stream->f);
	else
	    ch = *stream->s++;
	
	if (!ch || ch == EOF) {
	    value	= -1;
	    done++;
	}
	else {
	    ch &= 0xff;
	    digit = -1;
	    switch(ch) {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		digit = ch - '0';
		break;
	    case 'a':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'e':
	    case 'f':
		digit = 10 + ch - 'a';
		break;
	    case 'A':
	    case 'B':
	    case 'C':
	    case 'D':
	    case 'E':
	    case 'F':
		digit = 10 + ch - 'A';
		break;
	    case ' ':
	    case ',':
	    case '}':
	    case '\n':
	    case '\t':
		if (gotone) done++;
		break;
	    }
	    if (digit != -1) {
		value = (value << 4) + digit;
		gotone++;
	    }
	}
    }
    return value;
}


#if NeedFunctionPrototypes
static char *NextLine(char *buf, int n, Stream *stream)
#else
static char *NextLine(buf, n, stream)
char *buf;
int n;
Stream *stream;
#endif
{
    int i;
    
    if (!stream->s || !*stream->s) return NULL;
    for(i=0; (i < n-1) && *stream->s && (*stream->s != '\n'); i++)
	buf[i] = *stream->s++;
    if ((i < n-1) && *stream->s == '\n')
	buf[i++] = *stream->s++;
    buf[i] = '\0';
    return buf;
}

/*
 * The data returned by the following routine is always in left-most byte
 * first and left-most bit first.  If it doesn't return BitmapSuccess then
 * its arguments won't have been touched.  This routine should look as much
 * like the Xlib routine XReadBitmapfile as possible.
 */
#if NeedFunctionPrototypes
static int ReadBitmapData (Stream *stream,
			   unsigned int *width, unsigned int *height,
			   unsigned char **datap, int *x_hot, int *y_hot)
#else
static int ReadBitmapData (stream, width, height, datap, x_hot, y_hot)
Stream *stream;
unsigned int *width;
unsigned int *height;
unsigned char **datap;
int *x_hot;
int *y_hot;
#endif
{
    unsigned char *data = NULL;		/* working variable */
    char line[MAX_SIZE];		/* input line from file */
    int size;				/* number of bytes of data */
    char name_and_type[MAX_SIZE];	/* an input line */
    char *type;				/* for parsing */
    int value;				/* from an input line */
    int version10p;			/* boolean, old format */
    int padding;			/* to handle alignment */
    int bytes_per_line;			/* per scanline of data */
    unsigned int ww = 0;		/* width */
    unsigned int hh = 0;		/* height */
    int hx = -1;			/* x hotspot */
    int hy = -1;			/* y hotspot */

#define	RETURN(code) { XtFree (data); return code; }
#define nextline(buf, n, stream)\
    ((stream->f)?(fgets(buf, n, stream->f)):(NextLine(buf, n, stream)))

    while (nextline(line, MAX_SIZE, stream)) {
	if (strlen(line) == MAX_SIZE-1) {
	    return BitmapFileInvalid;
	}
	if (sscanf(line,"#define %s %d",name_and_type,&value) == 2) {
	    if (!(type = strrchr(name_and_type, '_')))
	      type = name_and_type;
	    else
	      type++;

	    if (!strcmp("width", type))
	      ww = (unsigned int) value;
	    if (!strcmp("height", type))
	      hh = (unsigned int) value;
	    if (!strcmp("hot", type)) {
		if (type-- == name_and_type || type-- == name_and_type)
		  continue;
		if (!strcmp("x_hot", type))
		  hx = value;
		if (!strcmp("y_hot", type))
		  hy = value;
	    }
	    continue;
	}
    
	if (sscanf(line, "static short %s = {", name_and_type) == 1)
	  version10p = 1;
	else if (sscanf(line,"static unsigned char %s = {",name_and_type) == 1)
	  version10p = 0;
	else if (sscanf(line, "static char %s = {", name_and_type) == 1)
	  version10p = 0;
	else
	  continue;

	if (!(type = strrchr(name_and_type, '_')))
	  type = name_and_type;
	else
	  type++;

	if (strcmp("bits[]", type))
	  continue;
    
	if (!ww || !hh)
	  return BitmapFileInvalid;

	if ((ww % 16) && ((ww % 16) < 9) && version10p)
	  padding = 1;
	else
	  padding = 0;

	bytes_per_line = (ww+7)/8 + padding;

	size = bytes_per_line * hh;
	data = (unsigned char *) XtMalloc ((unsigned int) size);

	if (version10p) {
	    unsigned char *ptr;
	    int bytes;

	    for (bytes=0, ptr=data; bytes<size; (bytes += 2)) {
		if ((value = NextInt(stream)) < 0) {
		    XtFree((char *)data);
		    return BitmapFileInvalid;
		}
		*(ptr++) = value;
		if (!padding || ((bytes+2) % bytes_per_line))
		  *(ptr++) = value >> 8;
	    }
	} else {
	    unsigned char *ptr;
	    int bytes;

	    for (bytes=0, ptr=data; bytes<size; bytes++, ptr++) {
		if ((value = NextInt(stream)) < 0) {
		    XtFree((char *)data);
		    return BitmapFileInvalid;
		}
		*ptr=value;
	    }
	}
	break;
    }					/* end while */

    if (data == NULL) {
	return BitmapFileInvalid;
    }

    *datap = data;
    *width = ww;
    *height = hh;
    if (x_hot) *x_hot = hx;
    if (y_hot) *y_hot = hy;

    return BitmapSuccess;
}


#if NeedFunctionPrototypes
Boolean XmtParseXbmFile (StringConst filename, char **datap,
			 int *width, int *height, int *x_hot, int *y_hot)
#else
Boolean XmtParseXbmFile (filename, datap, width, height, x_hot, y_hot)
StringConst filename;
char **datap;
int *width;
int *height;
int *x_hot;
int *y_hot;
#endif
{
    Stream stream;
    int status;

    stream.s = NULL;
    if ((stream.f = fopen (filename, "r")) == NULL) {
	XmtWarningMsg("XmtParseXbmFile", "unreadable",
		      "can't open file '%s'",
		      filename);
	return False;
    }

    status = ReadBitmapData (&stream,
			     (unsigned int *)width, (unsigned int *)height,
			     (unsigned char **)datap, x_hot, y_hot);
    fclose (stream.f);

    if (status == BitmapSuccess) return True;
    else {
	XmtWarningMsg("XmtParseXbmFile", "invalid",
		      "file '%s' is not in valid Xbm format.",
		      filename);
	return False;
    }
}

#if NeedFunctionPrototypes
Boolean XmtParseXbmString (StringConst string, char **datap,
			   int *width, int *height, int *x_hot, int *y_hot)
#else
Boolean XmtParseXbmString (string, datap, width, height, x_hot, y_hot)
StringConst string;
char **datap;
int *width;
int *height;
int *x_hot;
int *y_hot;
#endif
{
    Stream stream;
    int status;

    stream.s = string;
    stream.f = NULL;

    status = ReadBitmapData (&stream,
			     (unsigned int *)width, (unsigned int *)height,
			     (unsigned char **)datap, x_hot, y_hot);

    if (status == BitmapSuccess) return True;
    else {
	XmtWarningMsg("XmtParseXbmString", "invalid",
		      "string is not in valid Xbm format:\n%s",
		      string);
	return False;
    }
}
