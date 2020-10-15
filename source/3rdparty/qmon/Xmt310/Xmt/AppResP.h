/* 
 * Motif Tools Library, Version 3.1
 * $Id: AppResP.h,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: AppResP.h,v $
 * Revision 1.1  2001-07-18 11:06:01  root
 * Initial revision
 *
 * Revision 1.1  2001/06/12 15:00:21  andre
 * AA-2001-06-12-0: replaced Xmt212 by Xmt310
 *                  (http://sourceforge.net/projects/motiftools) with
 *                  our xmt212 patches applied
 *
 *
 */

#ifndef _XmtAppResP_h
#define _XmtAppResP_h

#include <Xmt/AppRes.h>
#include <Xmt/Hash.h>
#include <Xmt/Color.h>
#include <Xmt/DialogsP.h>

typedef struct {
    /* root directory and various paths for configuration files */
    String config_dir;
    String user_config_dir;  /* not a resource, but a good place to store it */
    String config_path;
    String user_config_path;
    String resource_file_path;
    String pixmap_file_path;
    String bitmap_file_path;
    String help_file_path;
    String application_name;
    String application_class;
    XmtHashTable loaded_files_table;
    String help_file;
    Cursor help_cursor;
    Pixmap help_pixmap;
    Boolean free_help_pixmap;
    XrmDatabase help_database;
    Boolean free_help_database;
    XmtHashTable screen_table;
    Cursor busy_cursor;
    Pixel foreground;
    Pixel background;
    Boolean reverse_video;
    unsigned background_hue;
    unsigned background_saturation;
    unsigned background_lightness;
    unsigned foreground_hue;
    unsigned foreground_saturation;
    unsigned foreground_lightness;
    String context_help_file;
    XmtHashTable help_node_table;
    Boolean help_file_loaded;
    XmtColorTable colortable;
    XmtColorTable default_colortable;
    Cursor cursor;
    Pixel cursor_foreground;
    Pixel cursor_background;
    Pixel help_cursor_foreground;
    Pixel help_cursor_background;
    Pixel busy_cursor_foreground;
    Pixel busy_cursor_background;
} XmtAppResources;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern XmtAppResources *XmtGetApplicationResources(Widget);
#else
extern XmtAppResources *XmtGetApplicationResources();
#endif
_XFUNCPROTOEND    

#endif
