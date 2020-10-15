/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
// culltrans.cpp
// sgee generator initially developed for qidl, now turned into
// a multi-purpose tool that translates cull definitions into
// a metastructure, which can then be used to create anything you
// want out of it, e.g. idl files, header files, java client stubs...

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include "culltrans_repository.h"
#include "culltrans.h"

#include "cull.h"
#include "sge_all_listsL.h"
#include "sge_boundaries.h"

#ifdef HAVE_STD
using namespace std;
#endif

// forward declaration
#ifdef HP11
extern "C" {
#endif
int yyparse(void);
#ifdef HP11
}
#endif

// global objects to store data in
map<string, List>               lists;
map<string, List>::iterator     active;
map<int, string>                constants;
int                             last_qidl_only = CCT_LOWERBOUND;
FILE*                           disthdr = NULL;
const char*                     yyin_name;
extern FILE*                    yyin;

// string arrays to convert multitype
const char *multiType2sgeType[] = {
   "",   // lEntT undefined
   "GE_sge_float",
   "GE_sge_double",
   "GE_sge_ulong",
   "GE_sge_long",
   "GE_sge_char",
   "GE_sge_bool"
   "GE_sge_int",
   "GE_sge_string",
   "",   // lListT undefined
   "GE_sge_object",
   "GE_sge_ref",
   "GE_sge_host"
};

const char *multiType2idlType[] = {
   "",   // lEntT undefined
   "sge_float",
   "sge_double",
   "sge_ulong",
   "sge_long",
   "sge_char",
   "boolean",
   "sge_int",
   "sge_string",
   "",   // lListT undefined
   "lObjectT",
   "lRefT",
   "lHostT"
};

const char *multiType2getType[] = {
   "",   // lEntT undefined
   "Float",
   "Double",
   "Ulong",
   "Long",
   "Char",
   "Bool",
   "Int",
   "String",
   "List",
   "Object",
   "Ref",
   "Host"
};

// checkArgs
// parses command line and returns:
// ARG_ERROR: for error
// ARG_EMPTY: when no args given
// ARG_NOXXX: when not to create XXX
// or a combination of these.
enum arguments {
   ARG_EMPTY = 0,
   ARG_ERROR = 1,
   ARG_IDL = 2,
   ARG_HDR = 4,
   ARG_DISTHDR = 8,
   ARG_IMPL = 16,
   ARG_ELEMSGEES = 32
};

static int checkArgs(char**& argv) {
   int ret = ARG_EMPTY;
   argv++;
   while(*argv) {
      if(!strcmp(*argv, "-idl"))
         ret |= ARG_IDL;
      else if(!strcmp(*argv, "-hdr"))
         ret |= ARG_HDR;
      else if(!strcmp(*argv, "-disthdr"))
         ret |= ARG_DISTHDR;
      else if(!strcmp(*argv, "-impl"))
         ret |= ARG_IMPL;
      else if(!strcmp(*argv, "-elemcodes"))
         ret |= ARG_ELEMSGEES;
      else if((*argv)[0] == '-') {
         ret |= ARG_ERROR;
         break;
      }
      else
         break;
      argv++;
   }
   return ret;
}

// checkFilesAndObjects
// parses the argument list for -of and -oo switches
static void checkFilesAndObjects(char**& argv, set<string>& files,
                          set<string>& objects) {
   set<string>* cur=&files;

   for(; *argv; argv++) {
      if(!strcmp(*argv, "-of"))
         cur = &files;
      else if(!strcmp(*argv, "-oo"))
         cur = &objects;
      else
         cur->insert(*argv);
   }
}
         

// usage
// print out usage message
static void usage() {
   cerr << "Usage: culltrans [-idl] [-hdr] [-impl] [-elemcodes] [-disthdr] input-files" << endl;
   cerr << "                 [-of files] [-oo objects]" << endl;
   cerr << endl;
   cerr << "-idl:        Triggers output of IDL files." << endl;
   cerr << "-hdr:        Triggers output of _implementation.h files." << endl;
   cerr << "-impl:       Triggers output of _implementation.cpp files." << endl;
   cerr << "-elemcodes:  Triggers output of elem_codes.idl file." << endl;
   cerr << "-disthdr:    Triggers output of .h.dist files." << endl;
   cerr << "-of:         Produces output only for objects that are defined in the" <<endl;
   cerr << "             following list of files." << endl;
   cerr << "-oo:         Produces output only for the given objects." << endl;
   cerr << "input-files: List of files that contain cull list definitions." << endl;
   cerr << "files:       List of files for whose objects output will be produced." << endl;
   cerr << "objects:     List of object names for which output will be produced." <<endl;
   cerr << endl;
   cerr << "If -of or -oo are not given, culltrans produces output for all files or objects." << endl;
   cerr << "If specified, BOTH -oo and -of requirements must be met for an object." << endl;
   cerr << "That is that no output will be produced for the following command line:" << endl;
   cout << "   culltrans -idl sge_calendarL.h sge_queue.h -of sge_calendar.h -oo Queue" << endl;
   cerr << "The order of the -oo and -of switches is not important unless they occur after" << endl;
   cerr << "the input file list. It is also valid to have multiple -of and -oo switches" << endl;
   cerr << "in one command line and to specify the same file or object more than once." <<endl;
   cerr << "or to specify files that don't exist." << endl;
   cerr << "-of and -oo have no effect on dist hdr output: .h.dist files are always" << endl;
   cerr << "produced if -disthdr is specified." << endl;
}

int main(int argc, char** argv) {
   // check arguments
   int args = checkArgs(argv);
   if(args & ARG_ERROR || argc == 1) {
      usage();
      return 1;
   }
   if(!*argv)
      return 0;

   lInit(nmv);

   // Creating dummy ST_Type list ( == string sequence )
   lists.insert(map<string, List>::value_type("ST_Type", List("ST_Type", "sge_string", NULL, NULL, false)));

   // for all arguments until -of or -oo do
   for(; *argv && strcmp(*argv, "-of") && strcmp(*argv, "-oo"); argv++) {
      char* disthdrname;

      // open the file
      yyin = fopen(*argv, "r");
      disthdr = NULL;
      if(!yyin) {
         cerr << "Could not open file " << *argv << endl;
         cerr << "Aborting..." << endl;
         return 1;
      }
      yyin_name = *argv;

      if(args & ARG_DISTHDR) {
         char* slash = strrchr(*argv, '/');
         disthdrname = new char[(slash?strlen(slash+1):strlen(*argv)) + strlen(".dist") + 1];
         strcpy(disthdrname, slash?slash+1:*argv);
         strcat(disthdrname, ".dist");
         disthdr = fopen(disthdrname, "w");
         if(!disthdr) {
            cerr << "Could not open file " << disthdrname << endl;
            cerr << "Aborting..." << endl;
            delete [] disthdrname;
            fclose(yyin);
            return 1;
         }
         delete [] disthdrname;
      }

      // parse the file
      cout << "Parsing file " << *argv << "..." << flush;

      while(!feof(yyin))
         yyparse();

      cout << "done" << endl;

      // cleanup
      fclose(yyin);
      if(disthdr)
         fclose(disthdr);
   }

   // now find out which objects to write
   set<string> files, objects;
   checkFilesAndObjects(argv, files, objects);

   // write IDL files
   if(args & ARG_IDL) {
      map<string, List>::iterator it;
      for(it=lists.begin(); it != lists.end(); ++it)
         if((files.empty() || files.find(it->second.file) != files.end()) &&
            (objects.empty() || objects.find(it->second.name) != objects.end()))
            if(!writeIDL(it)) {
               cerr << "Error writing IDL file for " << it->second.name << endl;
               cerr << "Aborting..." << endl;
               return 1;
            }
   }

   // write _implementation.h files
   if(args & ARG_HDR) {
      map<string, List>::iterator it;
      for(it=lists.begin(); it != lists.end(); ++it)
         if((files.empty() || files.find(it->second.file) != files.end()) &&
            (objects.empty() || objects.find(it->second.name) != objects.end()))
            if(!writeHdr(it)) {
               cerr << "Aborting..." << endl;
               return 1;
            }
   }

   // write _implementation.cpp files
   if(args & ARG_IMPL) {
      map<string, List>::iterator it;
      for(it=lists.begin(); it != lists.end(); ++it)
         if((files.empty() || files.find(it->second.file) != files.end()) &&
            (objects.empty() || objects.find(it->second.name) != objects.end()))
            if(!writeImplementation(it)) {
               cerr << "Aborting..." << endl;
               return 1;
            }
   }

   // write enum file
   if(args & ARG_ELEMSGEES) 
      if(!writeConsts()) {
         cerr << "Aborting..." << endl;
         return 1;
      }

   return 0;
}
