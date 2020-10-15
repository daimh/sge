/* 
 * Motif Tools Library, Version 3.1
 * $Id: FindFile.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: FindFile.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * Portions of this file are derived from the Xt source code.
 * See the file COPYRIGHT for the MIT and Digital copyrights.
 */

#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h>  /* for struct stat and stat() */
#include <X11/Xos.h>   /* for R_OK constant */
#include <Xmt/XmtP.h>
#include <Xmt/Util.h>
#include <Xmt/AppResP.h>

/* The following procedure is modified from the X11R5 source code. */
#ifndef VMS
#include <pwd.h>
#endif

#ifndef SYSV386
extern struct passwd *getpwuid(), *getpwnam();
#endif

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#include <unistd.h>
#else
extern char *getenv();
#endif

#if NeedFunctionPrototypes
String XmtGetHomeDir(void)
#else
String XmtGetHomeDir()
#endif
{
    static char *home = NULL;
#ifndef VMS
    struct passwd *pw;
#ifndef X_NOT_POSIX
    uid_t uid;
    extern uid_t getuid();
#else
    int uid;
    extern int getuid();
#endif
#endif /* not VMS */
    
    /* only do this once and cache the result */
    if (home == NULL) {
#ifndef VMS	
	if (!(home = getenv("HOME"))) {
	    if ((home = getenv("USER"))) pw = getpwnam(home);
	    else {
		uid = getuid();
		pw = getpwuid(uid);
	    }
	    if (pw) home = pw->pw_dir;
	    else {
		home = "";
	    }
	}
#else /* VMS */
	if (!(home = getenv("HOME"))) home = getenv("sys$login");
#endif /* VMS */
	
	if (home) home = XtNewString(home);
    }
    return home;
}

#ifndef VMS

#ifndef NDEBUG
static Boolean DebugPredicate(path)
    String path;
{
    struct stat status;

    printf("\t%s\n", path);
    return (access(path, R_OK) == 0 &&		/* exists and is readable */
	    stat(path, &status) == 0 &&		/* get the status */
#ifndef X_NOT_POSIX
	    S_ISDIR(status.st_mode) == 0);	/* not a directory */
#else
	    (status.st_mode & S_IFDIR) == 0);	/* not a directory */
#endif /* X_NOT_POSIX else */
}
#endif

/*
 * Standard substitutions:
 *   %R: the rootdir argument  (configDir app-resource by default)
 *   %H: the user's homedir
 *   %A: application classname
 *   %a: application name
 *   %v: visual type.  One of: "color", "gray", "monochrome".
 *   %d: screen depth in bitplanes.
 *   %z: Screen siZe: "small", "medium", "large".  Based on screen reZolution.
 *   %T: typename  
 *   %N: objname   (application classname by default)
 *   %S: suffix arg.
 *   %C: customization
 *   %L: language string
 *   %l: language part
 *   %t: territory part
 *   %c: codeset part
 *   %D: default Xt search path (X11R6 only)
 *
 * If a path is not specified, then the filePath app-resource is used,
 * or if that is not defined then the default path.  Whatever the
 * path, this function may make up to 3 separate searches: for a user-file,
 * for an app file, and for a system file.
 *
 * if SearchUserPath is True, then $XUSERFILESEARCHPATH is searched first,
 * or if it is not defined, then the value of $XAPPLRESDIR is substituted
 * for %R in the default path and that path is searched first.
 * Then $HOME is substituted for %R in the above and that path searched.
 *
 * if SearchAppPath, or where == 0, 
 * if the rootdir argument is non-NULL, it is substituted for %R,
 * and that path searched.  If it is NULL, then the directory specified by
 * the configDir app-resource is used in its place.  The default value
 * for this resource is the system dir: /usr/lib/X11
 *
 * if XmtSearchSysPath is True, then XtResolvePathname is called without a
 * path which causes it to search in the default system places.
 * There's a problem:  I treat suffixes differently than the Xt
 * default path.  So if a suffix is specified, and no file is found,
 * try again without the suffix.
 */

#if NeedFunctionPrototypes
String XmtFindFile(Widget w, StringConst type,
		   StringConst objname, StringConst suffix,
		   StringConst rootdir, StringConst path,
		   int where)
#else
String XmtFindFile(w, type, objname, suffix, rootdir, path, where)
Widget w;
StringConst type;
StringConst objname;
StringConst suffix;
StringConst rootdir;
StringConst path;
int where;
#endif
{
    static Boolean first_time = True;
    static String home_dir;
    static XtFilePredicate predicate;
    static SubstitutionRec subs[7];
    Display *dpy;
    Screen *screen;
    Visual *visual;
    XmtAppResources *app_resources = XmtGetApplicationResources(w);
    String visual_sub;
    String size_sub;
    char depth_sub[4];
    int screen_width;
    String filename = NULL;
#ifdef X11R5
    XrmDatabase old_db;
#endif 

    while(!XtIsWidget(w)) w = XtParent(w);
    screen = XtScreen(w);
    dpy = DisplayOfScreen(screen);
    visual = XmtGetVisual(w);

    /* handle absolute and relative object names */
    if (objname &&
	((objname[0] == '/') ||
	 ((objname[0] == '.') && (objname[1] == '/')) ||
	 ((objname[0] == '.') && (objname[1] == '.') && (objname[2] == '/'))))
	return XtNewString(objname);
    
    /* if where is XmtSearchPathOnly, make sure there is a path */
    if (where == XmtSearchPathOnly)
	if (path == NULL) return NULL;
    
    /*
     * do some one-time initialization:
     *   Get home_dir.
     *   Get XUSERFILESEARCHPATH; overide user_config_path if set.
     *   Get XAPPLRESDIR; set user_config_dir to it or to homedir
     */
    if (first_time) {
	String userpath, userdir;

	first_time = False;
	home_dir = XmtGetHomeDir();
	userpath = getenv("XUSERFILESEARCHPATH");
	if (userpath)
	    app_resources->user_config_path = XtNewString(userpath);
	userdir = getenv("XAPPLRESDIR");
	if (userdir)
	    app_resources->user_config_dir = XtNewString(userdir);
	else
	    app_resources->user_config_dir = home_dir;

	subs[0].match = 'R'; 
	subs[1].match = 'H'; 
	subs[2].match = 'A'; 
	subs[3].match = 'a'; 
	subs[4].match = 'v'; 
	subs[5].match = 'd'; 
	subs[6].match = 'z'; 
	
#ifndef NDEBUG
	if (getenv("XMTDEBUGFINDFILE"))
	    predicate = DebugPredicate;
#endif /* not NDEBUG */	
    }


    /* if no object name specified, use application class name */
    if (objname == NULL)
	objname = app_resources->application_class;
    
#ifndef NDEBUG
    if (predicate)
	printf("XmtFindFile: looking for object '%s' of type '%s'...\n",
	       objname, type);
#endif /* not NDEBUG */	

    if (visual->map_entries == 2) visual_sub = "monochrome";
    else if ((visual->class == StaticGray) ||
	     (visual->class == GrayScale)) visual_sub = "gray";
    else visual_sub = "color";
    
    sprintf(depth_sub, "%d", w->core.depth);

    screen_width = DisplayWidth(dpy, XScreenNumberOfScreen(screen));
    if (screen_width < 750) size_sub = "small";
    else if (screen_width > 1150) size_sub = "large";
    else size_sub = "medium";

    /* set up substitutions */
    /* we'll initialize the first substitution later */
    subs[1].substitution = home_dir;
    subs[2].substitution = app_resources->application_class;
    subs[3].substitution = app_resources->application_name;
    subs[4].substitution = visual_sub;
    subs[5].substitution = depth_sub;
    subs[6].substitution = size_sub;
    
#ifdef X11R5
    /*
     * In R5 and later, the customization resource should come from the
     * screen database, but XtResolvePathname only reads the db of the
     * default screen.  So we've got to explicitly set the right db.
     */
    old_db = XrmGetDatabase(dpy);
    XrmSetDatabase(dpy, XtScreenDatabase(XtScreenOfObject(w)));
#endif
		   
    /* search the user paths */
    if (where & XmtSearchUserPath) {
	/* root directory is $XAPPLRESDIR or $HOME */
	subs[0].substitution = app_resources->user_config_dir;
	filename = XtResolvePathname(dpy, type, objname, suffix,
				     app_resources->user_config_path,
				     subs, XtNumber(subs), predicate);
    }

    /*
     * set up the root directory substitution for the rest of the searches.
     * root directory is the rootdir argument, or 
     * the value of the configDir app resource.
     * the default value for this resource is the system directory.
     */
    subs[0].substitution = (rootdir)?(String)rootdir:app_resources->config_dir;

    /* search the supplied path, if non-NULL */
    if (!filename && path) {
	filename = XtResolvePathname(dpy, type, objname, suffix, path,
				     subs, XtNumber(subs), predicate);
    }

    /* search the application paths, if requested */
    if (!filename && (where & XmtSearchAppPath)) {
	filename = XtResolvePathname(dpy, type, objname, suffix,
				     app_resources->config_path,
				     subs, XtNumber(subs), predicate);
    }

    /* search the system paths */
    if (!filename && (where & XmtSearchSysPath)) {
	/*
	 * we define "system paths" to be where XtResolvePathname searches
	 * if we don't specify a path.  ie: $XFILESEARCHPATH or the default
	 * path under /usr/lib/X11.  Our special substitutions are of no
	 * use in this case. Note that this case does not distinguish between
	 * application names and object names.
	 */
	filename = XtResolvePathname(dpy, type, objname, suffix, NULL,
				     NULL, 0, predicate);
	/*
	 * if we didn't find anything, and there is a type and a suffix
	 * then try again, without the suffix.  This is sort of a kludge
	 * required because programmers are expected to pass suffixes like
	 * ".ad" and ".xbm" to this function, but files are generally not
	 * installed with those suffixes under /usr/lib/X11.
	 */
	if (!filename && type && type[0] && suffix && suffix[0])
	    filename = XtResolvePathname(dpy, type, objname, NULL, NULL,
					 NULL, 0, predicate);
    }

#ifdef X11R5
    /*
     * Restore the db, if needed
     */
    XrmSetDatabase(dpy, old_db);
#endif
		   
#ifndef NDEBUG
    if (predicate) {
	if (filename)
	    printf("XmtFindFile: found '%s'.\n", filename);
	else
	    printf("XmtFileFile: no file found.\n");
    }
#endif    

    return filename;
}

#else /* VMS */

#include <string.h>
#include <stdio.h>
#include <descrip.h>
#include <lnmdef.h>

static String result_path;

typedef struct file__comp {
   struct file__comp **next;
   int len;
   char *str;
   } file_comp;

static Boolean VmsTestFile(char *test);
static char *test_path(file_comp *path);
static file_comp *breakdown(char *test);
static char *assemble(file_comp *path);
static void get_logical(char *logname, char ***equiv, int *n_equiv);
static int check_logical(char *logname);

/*
** XmtFindFile isn't all that different for VMS. The main difference is in the
** way XtResolvePathname is handled. By default on VMS it's a no-op, as it
** supplies no default path, and the file test predicate always returns true.
** We always supply a path, and we supply our own file test predicate, which is
** where all the serious work is done. Since the strings formed by
** XtResovePathname aren't legal VMS file specs, when VmsTestFile finds a file,
** it stores the actual name of it in the static variable result_path so we can
** retrieve it after XtResolvePathname returns. See below for the workings of
** that routine.
**
** We handle a couple of special cases for VMS. When searching system
** directories, if the file type is bitmap, we look in DECW$BITMAPS, and when
** it is help, we look in SYS$HELP. Everything else is looked for in the same
** place as application default files: DECW$SYSTEM_DEFAULTS. This isn't as bad
** as it seems, as that logical is a search list that includes sys$library. If
** a system manager is inclined to put files in standard VMS directories,
** that's probably where he'd put them.
**
** See appshell.c for VMS specific changes to the default path.
**
** Standard substitutions:
**   %R: the rootdir argument  (decw$user_defaults for user searches,
**                              configDir app-resource for specified path and
**				application searches, and decw$system_defaults,
**				decw$bitmaps, or sys$help for system searches)
**   %H: the user's homedir
**   %A: application classname
**   %a: application name
**   %T: typename  
**   %N: objname   (application classname by default)
**   %S: suffix arg.
**   %C: customization
**   %L: language string
**   %l: language part
**   %t: territory part
**   %c: codeset part
**
** Note that this function may make up to 4 separate searches: for a user-file,
** a file with the specified path, for an app file, and for a system file.
**
** if SearchUserPath is True, then DECW$USER_DEFAULTS is searched first.
**
** Then, if a path was supplied, that path is searched using the specified root
** directory, or the configDir app resource if no root directory was specified.
**
** if SearchAppPath, or where == 0, if the rootdir argument is non-NULL, it is
** substituted for %R, otherwise the value of the configDir resource (which
** defaults to DECW$SYSTEM_DEFAULTS) is substituted for %R.  Then the default
** path, or the path specified by the configPath resource, is searched.  The
** default value for configPath is found in appshell.c.
**
** if XmtSearchSysPath is True, we search the system directories using a
** special path that looks only in the default system directory, and looks for
** the object name with and without the suffix. The default system directory
** for bitmaps (type == "bitmap") is DECW$BITMAPS. The default system directory
** for help files (type == "help") is SYS$HELP. The default system directory
** for everything else is DECW$SYSTEM_DEFAULTS, which, by default, searches
** several DECwindows directories, followed by sys$library.
**
** NOTE: We handle a special case for use by mockup that avoids initializing or
** using application resources so that this routine can be called during
** application initialization without automatically initializing anything. This
** is a gross hack, made necessary by the fact that VMS DECwindows/Motif doesn't
** give us any way to force a particular file to be used for the app-defaults
** file, which is precisely what mockup needs to do.
**
** If where is equal to XmtSearchEverywhere+1, we are in "mockup mode". We
** reset where to XmtSearchPathOnly, we do NOT initialize the app_resources
** variable, and we don't execute any code that might reference it. This is
** because XmtGetApplicationResources has some first time code that will
** intialize the app resources and we don't want that to happen till we play
** some games with the resource database (see mockup.c). We also don't execute
** XmtFindFile's own first_time code, for similar reasons. Note that the
** supplied path (which must exist) can not include any substitutions but the
** standard ones supported by XtResolvePathname and %R (the user's current
** default directory).
*/

#if NeedFunctionPrototypes
String XmtFindFile(Widget w, StringConst type,
		   StringConst objname, StringConst suffix,
		   StringConst rootdir, StringConst path,
		   int where)
#else
String XmtFindFile(w, type, objname, suffix, rootdir, path, where)
Widget w;
StringConst type;
StringConst objname;
StringConst suffix;
StringConst rootdir;
StringConst path;
int where;
#endif
{
    XmtAppResources *app_resources;
    static Boolean first_time = True;
    static String home_dir;
    SubstitutionRec subs[4];
    Display *dpy = XtDisplayOfObject(w);
    String filename = NULL;
    static XrmQuark bitmapQ, helpQ;
    XrmQuark typeQ;
    int mockup=0;

    /* Check for mockup mode. */
    if(where == (XmtSearchEverywhere)+1){
        mockup = 1;
	where = XmtSearchPathOnly;
	subs[0].match = 'R';
        subs[0].substitution = (String)rootdir;
    } else
        app_resources = XmtGetApplicationResources(w);

    /* handle absolute object names */
    if (objname && (strchr(objname, '[') || strchr(objname, ':') ||
        check_logical(objname)))
	return XtNewString(objname);
    
    /* if where is XmtSearchPathOnly, make sure there is a path */
    if (where == XmtSearchPathOnly)
	if (path == NULL) return NULL;
    
    result_path = NULL;

    if(!mockup){

	/*
	 * do some one-time initialization:
	 *   Get home_dir.
	 *   Get XUSERFILESEARCHPATH; overide user_config_path if set.
	 *   Get XAPPLRESDIR; set user_config_dir to it or to homedir
	 */
	if (first_time) {
	    first_time = False;
	    home_dir = XmtGetHomeDir();
	    app_resources->user_config_dir = "decw$user_defaults:";
	    bitmapQ = XrmStringToQuark("bitmap");
	    helpQ = XrmStringToQuark("help");
	}


	/* if no object name specified, use application class name */
	if (objname == NULL)
	    objname = app_resources->application_class;
	
	/* set up substitutions */
	subs[0].match = 'R'; /* we'll initialize this substitution later */
	subs[1].match = 'H'; subs[1].substitution = home_dir;
	subs[2].match = 'A'; subs[2].substitution=app_resources->application_class;
	subs[3].match = 'a'; subs[3].substitution=app_resources->application_name;
    }    
		   
    /* search the user paths */
    if (where & XmtSearchUserPath) {
	/* root directory is $XAPPLRESDIR or $HOME */
	subs[0].substitution = app_resources->user_config_dir;
	filename = XtResolvePathname(dpy, type, objname, suffix,
				     app_resources->user_config_path,
				     subs, XtNumber(subs), VmsTestFile);
    }

    /*
     * set up the root directory substitution for the rest of the searches.
     * root directory is the rootdir argument, or 
     * the value of the configDir app resource.
     * the default value for this resource is the system directory.
     */
    if(!mockup)subs[0].substitution =
	(rootdir)?(String)rootdir:app_resources->config_dir;

    /* search the supplied path, if non-NULL */
    if (!filename &&path) {
	filename = XtResolvePathname(dpy, type, objname, suffix, path,
	    subs, mockup ? 1 : XtNumber(subs), VmsTestFile);
    }

    /* search the application paths, if requested */
    if (!filename && (where & XmtSearchAppPath)) {
	filename = XtResolvePathname(dpy, type, objname, suffix,
				     app_resources->config_path,
				     subs, XtNumber(subs), VmsTestFile);
    }

    /* search the system paths */
    if (!filename && (where & XmtSearchSysPath)) {
        typeQ = XrmStringToQuark(type);
        if(typeQ == bitmapQ)subs[0].substitution = "decw$bitmaps:";
        else if(typeQ == helpQ)subs[0].substitution = "sys$help:";
        else subs[0].substitution = "decw$system_defaults:";
	/*
	 * we define "system paths" to be where XtResolvePathname searches
	 * if we don't specify a path.  ie: $XFILESEARCHPATH or the default
	 * path under /usr/lib/X11.  Our special substitutions are of no
	 * use in this case. Note that this case does not distinguish between
	 * application names and object names.
	 */
	filename = XtResolvePathname(dpy, type, objname, suffix,
            "%R%N%S:%R%N:", subs, 1, VmsTestFile);
    }

    if(filename)XtFree(filename);
    return result_path;
}

static Boolean VmsTestFile(char *test){

   file_comp *path;

   /*
   ** If a non-empty string is specified, break it down to components and test
   ** if the file exists. Otherwise, set result_path to NULL and return FALSE.
   */
   if(test && *test){
      path = breakdown(test);
      result_path = test_path(path);
      }
   else result_path = NULL;
   return result_path ? TRUE : FALSE;
   }

static file_comp *breakdown(char *test){

   file_comp *path = NULL, *cur;
   char *p, *copy, *q, *last_dot;

   /*
   ** Separate the filespec into a set of components. Delimiters are discarded.
   ** Note that this will break down things that bear no resemblence in form to
   ** VMS filespecs, and that multiple delimiters are treated as one delimiter,
   ** and leading delimiters are ignored. Thus
   ** /decw$user_defaults/[stuff]:file.type will breakdown just fine. Thus you
   ** don't have to worry about getting punctuation right in the paths, just as
   ** long as there is _some_ punctuation at the appropriate points.
   */
   copy = XtNewString(test);
   last_dot = strrchr(copy, '.');
   if(last_dot && !strpbrk(last_dot, ":[]/"))*last_dot = '~';
   q = copy;
   while(p = strtok(q, ":[]/.")){
      if(!path){
         path = XtMalloc(sizeof(file_comp));
	 cur = path;
	 q = NULL;
         }
      else {
         cur->next = XtMalloc(sizeof(file_comp));
	 cur = cur->next;
         }
      cur->str = XtNewString(p);
      cur->len = strlen(cur->str);
      }
   if(path)cur->next = NULL;
   last_dot = strchr(cur->str, '~');
   if(last_dot)*last_dot = '.';
   XtFree(copy);
   return path;
   }

static char *assemble(file_comp *path){

   int chars = 0, comps = 0, i;
   file_comp *p;
   char *ret;

   /*
   ** Assemble a filespec from the components in path. If there is only 1
   ** component, simply return it. If there are 2, return comp_1:comp_2. If
   ** there are 3, return comp_1:[comp_2]comp_3. If there are more, we add
   ** subdirectories, like: comp_1:[comp_2.comp_3]comp_4.
   */
   for(p = path; p; p = p->next){
      comps++;
      chars += p->len;
      }
   ret = XtMalloc(chars+comps+2);
   strcpy(ret, path->str);
   if(comps > 1)strcat(ret, ":");
   if(comps > 2)strcat(ret, "["); 
   p = path->next;
   for(i = 1; i < comps-2; i++, p = p->next){
      strcat(ret, p->str);
      strcat(ret, ".");
      }
   if(comps > 2){
      strcat(ret, p->str);
      strcat(ret, "]");
      p = p->next;
      }
   if(comps > 1)strcat(ret, p->str);
   return ret;
   }

static char *test_path(file_comp *path){

   char *ret = NULL, *test, **equiv;
   file_comp *log, *p, *q;
   int i, n_equiv;

   /*
   ** Test if the file exists and is readable. Process differently depending on
   ** whether the first component of the filespec is a logical name.
   */
   get_logical(path->str, &equiv, &n_equiv);
   if(n_equiv){
      /*
      ** The first component is a logical name. Loop through each equivalence
      ** string in order. For each one, break down the logical name and append
      ** all file spec components except the first, thus creating a new file
      ** components list that has the equivalence string substituted for the
      ** logical name. Then call ourselves recursively to test that path
      ** instead. Recursion ends when we run out of logical names to translate
      ** (we'll hit the else clause below).  Each equivalence name is tried in
      ** turn as long as we haven't found a file yet.
      */
      for(i = 0; !ret && i < n_equiv; i++){
         log = breakdown(equiv[i]);
	 for(p = log; p->next; p = p->next);
	 p->next = path->next;
	 ret = test_path(log);
	 p = log;
	 do {
            q = p->next;
	    XtFree(p);
	    p = q;
            }
	 while(p != path->next);
         }
      for(i = 0; i < n_equiv; i++)XtFree(equiv[i]);
      XtFree(equiv);
      }
   else {
      /*
      ** No logicals, assemble a filespec and see if the file is there.
      */
      test = assemble(path);
      if(access(test, 4) == 0)ret = test;
      else XtFree(test);
      }
   return ret;
   }

static int check_logical(char *logname){

   int n_equiv;

   get_logical(logname, NULL, &n_equiv);
   return n_equiv != 0;
   }

static void get_logical(char *logname, char ***equiv, int *n_equiv){

   typedef struct _itmlist {
      short blen, code;
      void *baddr, *rladdr;
   } itmlist;
   
   $DESCRIPTOR(table, "LNM$FILE_DEV");
   $DESCRIPTOR(lnam, "");
   int n, i, stat;
   short len;
   char str[256];
   itmlist max_index[] = {
      {sizeof(n), LNM$_MAX_INDEX, &n, NULL},
      {0, 0, 0, 0}
      };
   itmlist value[] = {
      {sizeof(i), LNM$_INDEX, &i, NULL},
      {255, LNM$_STRING, str, &len},
      {0, 0, 0, 0}
      };

   /*
   ** This routine gets all the equivalence names of the logical name logname.
   */

   /*
   ** Get the number of equivalence names.
   */
   lnam.dsc$w_length = strlen(logname);
   lnam.dsc$a_pointer = logname;
   stat = sys$trnlnm(&LNM$M_CASE_BLIND, &table, &lnam, 0, max_index);
   n++;
   if(stat != 1 || n < 1){
      /*
      ** Not a logical or no equivalence names (Huh?)
      */
      *n_equiv = 0;
      if(equiv)*equiv = NULL;
      }
   else {
      /*
      ** It is a logical and has at least one equivalence name.
      ** Store the number and see if the caller wants the values.
      */
      *n_equiv = n;
      if(equiv){
         /*
	 ** He does. Set up equiv and loop through the equivalence names.
	 */
	 *equiv = XtMalloc(*n_equiv*sizeof(char *));
	 for(i = 0; i < n; i++){
	    str[0] = '\0';
	    stat = sys$trnlnm(&LNM$M_CASE_BLIND, &table, &lnam, 0, value);
	    str[len] = '\0';
	    (*equiv)[i] = XtNewString(str);
	    }
         }
      }
   }

#endif /* VMS */
