/* $Id: IconListT.c,v 1.1 2001-07-18 11:06:03 root Exp $ */
/*
 * Copyright 1996 John L. Cwikla
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * Wolfram Research, Inc not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.    John L. Cwikla and Wolfram Research, Inc make no
 * representations about the suitability of this software for any
 * purpose. It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and Wolfram Research, Inc disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * Wolfram Research, Inc be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 *  John L. Cwikla
 *  X Programmer
 *  Wolfram Research Inc.
 *
 *  cwikla@wri.com
*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/ShellP.h>

#include "IconList.h"

#include "guifbff.h"

#include <stdio.h>

#define APPNAME "IconListTest"
#define APPCLASS "IconListTest"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

static void QuitIt(_w, _nil, _event)
Widget _w;
void *_nil;
XEvent *_event;
{
	int n;
	Arg warg[2];

	if (_event->type == ButtonPress)
		if (((XButtonEvent *)_event)->button == 3)
		{
			printf("Have a nice day. --JLC\n");
			exit(1);
		}
}

main(argc, argv)
int argc;
char *argv[];
{
	char *dir;
	char buffer[25];
	Widget iconList, toplevel;
	XtAppContext app;
	Display *theDisplay;
	int theScreenNumber;
	Arg warg[3];
	int n, i;
	XColor xcolor;
	Pixmap folderPixmap, pagePixmap;
	unsigned int folderWidth, folderHeight, pageWidth, pageHeight;
	int xhot, yhot;
	IconListElement *elements, *thisElement;
	const char **dirList, **fileList;
	int dirTotal, fileTotal;
	int dirJ, fileJ;

	XtToolkitInitialize();
	app = XtCreateApplicationContext();

	theDisplay = XtOpenDisplay (app, NULL, APPNAME, APPCLASS, 
		NULL, 0, &argc, argv);

	if (!theDisplay)
	{
		printf("%s: can't open display, exiting...", APPNAME);
		exit (0);
	}

	theScreenNumber = DefaultScreen(theDisplay);

	toplevel = XtAppCreateShell (APPNAME, APPCLASS,
		applicationShellWidgetClass, theDisplay, NULL, 0);

	XReadBitmapFile(theDisplay, RootWindow(theDisplay, theScreenNumber),
		"folder.xbm", &folderWidth, &folderHeight, &folderPixmap, &xhot, &yhot);

	XReadBitmapFile(theDisplay, RootWindow(theDisplay, theScreenNumber),
		"page.xbm", &pageWidth, &pageHeight, &pagePixmap, &xhot, &yhot);

	if (argc > 1)
		dir = argv[1];
	else
		dir = ".";

	dirList = GUIFBFindFiles(dir, (argc > 2) ? argv[2] : NULL, FALSE, TRUE, FALSE, &dirTotal);
	fileList = GUIFBFindFiles(dir, (argc > 2) ? argv[2] : NULL, TRUE, FALSE, FALSE, &fileTotal);

	if (dirTotal + fileTotal)
		elements = (IconListElement *)XtMalloc(sizeof(IconListElement) * (dirTotal+fileTotal));
	else
		elements = NULL;

	dirJ = 0;
	fileJ = 0;
	for(i=0;i<(dirTotal+fileTotal);i++)
	{
		thisElement = elements+i;
		if (i<dirTotal)
		{
			thisElement->pixmap = folderPixmap;
			thisElement->mask = None;
			thisElement->pixWidth = folderWidth;
			thisElement->pixHeight = folderHeight;
			thisElement->sensitive = TRUE;
			thisElement->string = dirList[dirJ++];
		}
		else
		{
			thisElement->pixmap = pagePixmap;
			thisElement->mask = pagePixmap;
			thisElement->pixWidth = pageWidth;
			thisElement->pixHeight = pageHeight;
			thisElement->sensitive = TRUE;
			thisElement->string = fileList[fileJ++];
		}
	}

	n = 0;
	XtSetArg(warg[n], XmNiconWidth, MAX(folderWidth, pageWidth)); n++;
	XtSetArg(warg[n], XmNiconHeight, MAX(folderHeight, pageHeight)); n++;
	XtSetArg(warg[n], XmNusingBitmaps, TRUE); n++;
	iconList = XmCreateScrolledIconList(toplevel, "IconList", warg, n);

/* XtCreateManagedWidget("IconList", xmIconListWidgetClass, toplevel, warg, n); */

	XmIconListSetItems(iconList, elements, (dirTotal+fileTotal));

	XtRealizeWidget(toplevel);

	XtAppMainLoop(app);
}
