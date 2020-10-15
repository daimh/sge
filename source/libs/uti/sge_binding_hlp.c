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
 *   Portions of this code are Copyright 2011 Univa Inc.
 *   Copyright (C) 2011 Dave Love, University of Liverpool
 *
 ************************************************************************/

/*___INFO__MARK_END__*/

/* this code is used by shepherd */
#include <ctype.h>
#include <pthread.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_binding_hlp.h"
#include "uti/msg_utilib.h" 
#include "uti/sge_mtutil.h"
#include "uti/sge_stdlib.h"

static bool is_digit(const char* position, const char stopchar);

/****** sge_binding_hlp/parse_binding_parameter_string() ***********************
*  NAME
*     parse_binding_parameter_string() -- Parses binding parameter string. 
*
*  SYNOPSIS
*     bool parse_binding_parameter_string(const char* parameter, u_long32* 
*     type, dstring* strategy, int* amount, int* stepsize, int* firstsocket, 
*     int* firstcore, dstring* socketcorelist, dstring* error) 
*
*  FUNCTION
*     Parses binding parameter string and returns the values of the parameter.
*     Please check output values in dependency of the strategy string.
*
*  INPUTS
*     const char* parameter   - binding parameter string 
*
*  OUTPUT 
*     u_long32* type          - type of binding (pe = 0| env = 1|set = 2)
*     dstring* strategy       - binding strategy string
*     int* amount             - number of cores to bind to
*     int* stepsize           - step size between cores (or -1)
*     int* firstsocket        - first socket to use (or -1)
*     int* firstcore          - first core to use (on "first socket") (or -1)
*     dstring* socketcorelist - list of socket,core pairs with prefix explicit or NULL
*     dstring* error          - error as string in case of return false
*
*  RESULT
*     bool - true in case parsing was successful false in case of errors
*
*  NOTES
*     MT-NOTE: parse_binding_parameter_string() is MT safe 
*
*******************************************************************************/
bool parse_binding_parameter_string(const char* parameter, binding_type_t* type, 
      dstring* strategy, int* amount, int* stepsize, int* firstsocket, 
      int* firstcore, dstring* socketcorelist, dstring* error)
{
   bool retval = true;

   if (parameter == NULL) {
      /* fixme:  here and elsewhere below, use the message catalogue */
      sge_dstring_sprintf(error, "input parameter was NULL");
      return false;
   }
   
   /* check the type [pe|env|set] (set is default) */
   if (strstr(parameter, "pe ") != NULL) {
      *type = BINDING_TYPE_PE;
   } else if (strstr(parameter, "env ") != NULL) {
      *type = BINDING_TYPE_ENV;
   } else {
      *type = BINDING_TYPE_SET;   
   }

   if (strstr(parameter, "linear") != NULL) {

      *amount = binding_linear_parse_number(parameter);

      if (*amount  < 0) {
         /* couldn't parse number of cores */
         sge_dstring_sprintf(error, "couldn't parse amount (linear)");
         return false;
      }

      *firstsocket = binding_linear_parse_socket_offset(parameter);

      if (*firstsocket == -2) {
         sge_dstring_sprintf(error, "couldn't parse socket number (linear)");
         return false;     
      }

      if (*firstsocket < 0) {
         /* no socket number given, hence we don't scan for the core number */
         *firstcore = *firstsocket;
      } else { 
         *firstcore   = binding_linear_parse_core_offset(parameter);
         if (*firstcore < 0) {
            /* there was an error during parsing the core number */
            sge_dstring_sprintf(error, "couldn't parse core number (linear)");
            return false;
         }
      }

      if (*firstsocket < 0 && *firstcore < 0) {
         /* couldn't find start <socket,core> -> must be determined 
            automatically */
         sge_dstring_sprintf(strategy, "linear_automatic");

         /* this might be an error on shepherd side only */
         *firstsocket = -1;
         *firstcore = -1;
      } else {
         sge_dstring_sprintf(strategy, "linear");
      }
      
      /* set step size to dummy */ 
      *stepsize = -1;
      
   } else if (strstr(parameter, "striding") != NULL) {
      
      *amount = binding_striding_parse_number(parameter);
      
      if (*amount  < 0) {
         /* couldn't parse number of cores */
         sge_dstring_sprintf(error, "couldn't parse amount (striding)");
         return false;
      }
  
      *stepsize = binding_striding_parse_step_size(parameter);

      if (*stepsize < 0) {
         /* no given stepsize does lead to stepsize 0 which will 
            be extended on the execution host to <number of cores per socket>.
            When using a start socket,core the stepsize 0 has to be 
            included (binding striding:<amount>:0:<socket>,<core>.*/
         /* *stepsize = 0; */
         sge_dstring_sprintf(error, "couldn't parse stepsize (striding)");
         return false;
      }

      *firstsocket = binding_striding_parse_first_socket(parameter);

      if (*firstsocket == -2) {
         sge_dstring_sprintf(error, "couldn't parse socket number (striding)");
         return false;
      }

      if (*firstsocket == -1) {
         *firstcore = -1;
      } else {
         /* we need to parse the core number since a socket number is given */
         *firstcore = binding_striding_parse_first_core(parameter);
         if (*firstcore < 0) {
            sge_dstring_sprintf(error, "couldn't parse core number (striding)");
            return false;
         }
      }

      if (*firstsocket < 0 || *firstcore < 0) {
         sge_dstring_sprintf(strategy, "striding_automatic");   

         /* this might be an error on shepherd side only */
         *firstsocket = -1;
         *firstcore = -1;
      } else {
         sge_dstring_sprintf(strategy, "striding");   
      }

   } else if (strstr(parameter, "explicit") != NULL) {

      if (binding_explicit_has_correct_syntax(parameter, error) == false) {
         retval = false;   
      } else {
         if (socketcorelist == NULL) {
            sge_dstring_sprintf(error, "%s", MSG_SYNTAX_DSTRINGBUG);
            retval = false;  
         } else {
            char* pos = strstr(parameter, "explicit"); 
            sge_dstring_copy_string(socketcorelist, pos);
            sge_dstring_sprintf(strategy, "explicit");
            pos = NULL;
         }
      }

   } else {
      
      /* error: no valid strategy found */
      sge_dstring_sprintf(error, "couldn't parse binding parameter (no strategy found)"); 
      retval = false;
   }
   
  return retval; 
}


/* ---------------------------------------------------------------------------*/
/*                Beginning of generic parsing functions                      */ 
/* ---------------------------------------------------------------------------*/

/****** sge_binding_hlp/binding_linear_parse_number() **************************
*  NAME
*    binding_linear_parse_number() -- Parse the number of cores to occupy.
*
*  SYNOPSIS
*     int binding_linear_parse_number(const char* parameter)
*
*  FUNCTION
*    Parses a string in order to get the number of cores requested.
* 
*    The string has following format: "linear:<amount>[:<socket>,<core>]" 
*
*  INPUTS
*     const char* parameter - The first character of the string  
*
*  RESULT
*     int - if a value >= 0 then this reflects the number of cores
*           if a value < 0 then there was a parsing error
*
*  NOTES
*     MT-NOTE: binding_linear_parse_number() is MT safe
*
*******************************************************************************/
int binding_linear_parse_number(const char* parameter)
{
   int retval = -1;
   int nn = -1;

   /* expect string "linear" or "linear:<number>" or "linear:slots" or
      "linear:<number>:<firstsocket>,<firstcore>" */

   if (parameter != NULL && strstr(parameter, "linear") != NULL) {
      /* get number after linear: and before \0 or : */ 
      /* Use an "infinite" value to signify using the number of slots
	 to remain compatible, rather than changing types.  */
      if (sge_strtok(parameter, ":") == NULL)
         nn = BIND_INFINITY;
      else {
         /* get number after linear: and before \0 or : */
         char* n = sge_strtok(NULL, ":");
         if (!n || 0 == strcmp("slots", n))
            nn = BIND_INFINITY;
         else if (is_digit(n, ':'))
            nn = atoi(n);
      }
      /* check if we got a number */
      if (nn != -1) {
         /* check when something follows after an additional ":"
	    if it is also a digit (the socket number) */
	 char* socket = sge_strtok(NULL, ":");
	 if (socket == NULL || is_digit(socket, ',')) {
	    return nn;
	 }
      }
   }

   /* parsing error */
   return retval;
}

/****** sge_binding_hlp/bindingLinearParseSocketOffset() ***************************
*  NAME
*     bindingLinearParseSocketOffset() -- Get socket number of linear request. 
*
*  SYNOPSIS
*     int bindingLinearParseSocketOffset(const char* parameter) 
*
*  FUNCTION
*     In a request like "-binding linear:<amount>:<socketnr>,<corenr>" it 
*     parses the socket number. 
*
*  INPUTS
*     const char* parameter - pointer to the binding request 
*
*  RESULT
*     int - if a value >= 0 then this reflects the socket number
*           if a value < 0 then there was a parsing error or no socket number was given
*
*  NOTES
*     MT-NOTE: bindingLinearParseSocketOffset() is not MT safe 
*
*******************************************************************************/
int binding_linear_parse_socket_offset(const char* parameter)
{
   /* offset is like "linear:<N>:<socket>,<core>) */
   if (parameter != NULL && strstr(parameter, "linear") != NULL) {
      /* fetch linear */
      if (sge_strtok(parameter, ":") != NULL) {
         /* fetch first number (if any) */
         if (sge_strtok(NULL, ":") != NULL) {
            char* offset = sge_strtok(NULL, ",");
            if (offset != NULL) { 
               if (is_digit(offset, ',')) {
                  /* offset points to <socket> */
                  return atoi(offset);
               } else {
                  /* there is something but no a number */
                  return -2;
               }
            }
         }
      }
   }
   
   /* wasn't able to parse */
   return -1;
}

/****** sge_binding_hlp/bindingLinearParseCoreOffset() *****************************
*  NAME
*     bindingLinearParseCoreOffset() -- Get core number of linear request.
*
*  SYNOPSIS
*     int bindingLinearParseCoreOffset(const char* parameter) 
*
*  FUNCTION
*     In a request like "-binding linear:<amount>:<socketnr>,<corenr>" it 
*     parses the core number. 
*
*  INPUTS
*     const char* parameter - pointer to the binding request 
*
*  RESULT
*     int - if a value >= 0 then this reflects the core number 
*           if a value < 0 then there was a parsing error or no core number was given
*
*  NOTES
*     MT-NOTE: bindingLinearParseCoreOffset() is not MT safe 
*
*******************************************************************************/
int binding_linear_parse_core_offset(const char* parameter)
{
   /* offset is like "linear:<N>:<socket>,<core> (optional ":") */
   if (parameter != NULL && strstr(parameter, "linear") != NULL) {
      /* fetch linear */
      if (sge_strtok(parameter, ":") != NULL) {
         /* fetch first number (if any) */
         if (sge_strtok(NULL, ":") != NULL) {
            char* offset = sge_strtok(NULL, ",");
            if (offset != NULL &&
                  (offset = sge_strtok(NULL, ":")) != NULL) {
               /* check if something follows, what is not a number */
               if (!is_digit(offset, ' ')) {
                  /* core offset contains junk */
                  return -1;
               }
               /* offset points to <core> */
               return atoi(offset);
            }
         }
      }
   }
  
   /* wasn't able to parse */
   return -1;
}

/****** sge_binding_hlp/binding_parse_type() ***********************************
*  NAME
*     binding_parse_type() -- Parses binding type out of binding string. 
*
*  SYNOPSIS
*     binding_type_t binding_parse_type(const char* parameter) 
*
*  FUNCTION
*     The execution daemon communicates with the shepherd with the config 
*     file. This function parses the type of binding out of the specific 
*     binding string from the config file. In case of binding type "set" 
*     there is no special prefix in this string. In case of "environment" 
*     the "env_" prefix in within the string. In case of setting the 
*     rankfile the "pe_" prefix can be found in this string.
*
*  INPUTS
*     const char* parameter - The binding string from the config file. 
*
*  RESULT
*     binding_type_t - The type of binding. 
*
*  NOTES
*     MT-NOTE: binding_parse_type() is MT safe 
*
*******************************************************************************/
binding_type_t binding_parse_type(const char* parameter)
{
   binding_type_t type = BINDING_TYPE_SET;
   
   if (strstr(parameter, "env_") != NULL) {
      type = BINDING_TYPE_ENV;
   } else if (strstr(parameter, "pe_") != NULL) {
      type = BINDING_TYPE_PE;
   }

   return type;
}


/****** sge_binding_hlp/binding_explicit_has_correct_syntax() *********************
*  NAME
*     binding_explicit_has_correct_syntax() -- Check if parameter has correct syntax. 
*
*  SYNOPSIS
*     bool binding_explicit_has_correct_syntax(const char* parameter) 
*
*  FUNCTION
*     This function checks if the given string is a valid argument for the 
*     -binding parameter which provides a list of socket, cores which have 
*     to be selected explicitly.
* 
*     The accepted syntax is: "explicit:[1-9][0-9]*,[1-9][0-9]*(:[1-9][0-9]*,[1-9][0-9]*)*"
*
*     This is used from parse_qsub.c.
*
*  INPUTS
*     const char* parameter - A string with the parameter. 
*
*  RESULT
*     bool - True if the parameter has the expected syntax.
*
*  NOTES
*     MT-NOTE: binding_explicit_has_correct_syntax() is not MT safe 
*
*******************************************************************************/
bool binding_explicit_has_correct_syntax(const char* parameter, dstring* error) 
{
   int amount;

   /* check if the head is correct */
   if (strstr(parameter, "explicit:") == NULL) {
      sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_NOTFOUND);
      return false;
   }

   if (sge_strtok(parameter, ":") != NULL) {
      char* socket = NULL;
      char* core   = NULL;

      /* first socket,core is mandatory */ 
      if ((socket = sge_strtok(NULL, ",")) == NULL) {
         /* we have no first socket number */
         sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_NOPAIR);
         return false;
      }
      /* check if <socket> begins with a digit */
      if (!is_digit(socket, ',')) {
         sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_FIRSTSOCKNONUMBER);
         return false;
      }

      /* check for core */
      if ((core = sge_strtok(NULL, ":")) == NULL) {
         /* we have no first core number */
         sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_MISSINGFIRSTCORE);
         return false;
      }
      /* check if <core> begins with a digit */
      if (!is_digit(core, ':')) {
         sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_FIRSTCORENONUMBER);
         return false;
      }

      do {
         /* get socket number */ 
         if ((socket = sge_strtok(NULL, ",")) != NULL) {
            /* check if <socket> begins with a digit */
            if (!is_digit(socket, ',')) {
               sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_SOCKNONUMBER);
               return false;
            }

            /* we have a socket therefore we need a core number */
            if ((core = sge_strtok(NULL, ":")) == NULL) {
               /* no core found */
               sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_NOCOREFORSOCKET);
               return false;
            }
            
            /* check if <core> is a number */
            if (!is_digit(core, ':')) {
               sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_COREISNONUMBER);
               return false;
            }

         } /* end if <socket> */ 
      } while (socket != NULL);  /* we try to continue with the next socket if possible */ 

   } else {
      /* this should not be reachable because of the pre-check */
      return false;
   }

   /* check if there are <socket,core> pairs requested multiple times */
   amount = get_explicit_number(parameter, true);
  
   if (check_explicit_binding_string(parameter, amount, true) == false) {
      sge_dstring_sprintf(error, "%s", MSG_SYN_EXPLICIT_PAIRSNOTUNIQUE);
      return false;
   }

   return true;
}

/****** sge_binding_hlp/binding_striding_parse_first_core() ************************
*  NAME
*     binding_striding_parse_first_core() -- Parses core number from command line. 
*
*  SYNOPSIS
*     int binding_striding_parse_first_core(const char* parameter) 
*
*  FUNCTION
*     Parses the core number from command line in which to start binding 
*     in "striding" case. 
*
*     -binding striding:<amount>:<stepsize>:<socket>,<core>
*
*  INPUTS
*     const char* parameter - Pointer to first character of CL string. 
*
*  RESULT
*     int - -1 in case the string is corrupt or core number is not set
*           >= 0 in case the core number could parsed successfully.
*
*  NOTES
*     MT-NOTE: binding_striding_parse_first_core() is not MT safe 
*
*******************************************************************************/
int binding_striding_parse_first_core(const char* parameter)
{
   /* "striding:<amount>:<stepsize>:<socket>,<core>" */
   if (parameter != NULL && strstr(parameter, "striding") != NULL) {
      /* fetch "striding" */
      if (sge_strtok(parameter, ":") != NULL) {
         /* fetch <amount> */
         if (sge_strtok(NULL, ":") != NULL) {
            /* fetch <stepsize> */
            if (sge_strtok(NULL, ":") != NULL) {
               /* fetch first <socket> */
               if (sge_strtok(NULL, ",") != NULL) {
                  /* fetch first <core> */ 
                  char* first_core = NULL;
                  /* end usually with line end (":" in case of config file) */
                  if ((first_core = sge_strtok(NULL, ":")) != NULL 
                        && is_digit(first_core, ' ')) {
                     return atoi(first_core);
                  } 
               }
            }
         }
      }   
   }

   return -1;
}


/****** sge_binding_hlp/binding_striding_parse_first_socket() **********************
*  NAME
*     binding_striding_parse_first_socket() -- Parses the socket to begin binding on. 
*
*  SYNOPSIS
*     int binding_striding_parse_first_socket(const char* parameter) 
*
*  FUNCTION
*     Parses the "striding:" parameter string for the socket number.
*
*     The string is expected to have following syntax: 
*    
*           "striding:<amount>:<stepsize>[:<socket>,<core>]"
*
*  INPUTS
*     const char* parameter - Points to the string with the query. 
*
*  RESULT
*     int - Returns the socket number in case it could be parsed otherwise -1
*
*  NOTES
*     MT-NOTE: binding_striding_parse_first_socket() is not MT safe 
*
*******************************************************************************/
int binding_striding_parse_first_socket(const char* parameter)
{
   /* "striding:<amount>:<stepsize>:<socket>,<core>" */
   if (parameter != NULL && strstr(parameter, "striding") != NULL) {
      /* fetch "striding" */
      if (sge_strtok(parameter, ":") != NULL) {
         /* fetch amount*/
         if (sge_strtok(NULL, ":") != NULL) {
            /* fetch stepsize */
            if (sge_strtok(NULL, ":") != NULL) {
               /* fetch first socket */ 
               char* first_socket = NULL;
               if ((first_socket = sge_strtok(NULL, ",")) != NULL) { 
                  if (is_digit(first_socket, ',')) {
                     return atoi(first_socket);
                  } else {
                     /* socket number is given but is not a number */
                     return -2;
                  }
               } 
            }
         }
      }   
   }

   return -1;
}


/****** sge_binding_hlp/binding_striding_parse_number() ****************************
*  NAME
*     binding_striding_parse_number() -- Parses the number of cores to bind to.
*
*  SYNOPSIS
*     int binding_striding_parse_number(const char* parameter)
*
*  FUNCTION
*     Parses the number of cores to bind to out of "striding:" parameter string.
*
*     The string is expected to have following syntax: 
*    
*           "striding:<amount>:<stepsize>[:<socket>,<core>]"
*
*  INPUTS
*     const char* parameter - Points to the string with the query. 
*
*  RESULT
*     int - Returns the number of cores to bind to otherwise -1.
*
*  NOTES
*     MT-NOTE: binding_striding_parse_number() is not MT safe
*
*******************************************************************************/
int binding_striding_parse_number(const char* parameter)
{
   /* striding:<amount>:<step-size>:[starting-socket,starting-core] */

   if (parameter != NULL && strstr(parameter, "striding") != NULL) {
      
      /* fetch "striding:" */
      if (sge_strtok(parameter, ":") != NULL) {
         char* amount = NULL;

         if ((amount = sge_strtok(NULL, ":")) != NULL 
               && is_digit(amount, ':')) {
            /* check if step size if given */ 
            char* stepsize = sge_strtok(NULL, ":");
            if (is_digit(stepsize, ':')) {
               /* get the number from amount */
               return atoi(amount);
            }
         }
      }
   }

   /* couldn't parse it */
   return -1;
}

/****** sge_binding_hlp/binding_striding_parse_step_size() *************************
*  NAME
*     binding_striding_parse_step_size() -- Parses the step size out of the "striding" query. 
*
*  SYNOPSIS
*     int binding_striding_parse_step_size(const char* parameter) 
*
*  FUNCTION
*     Parses the step size for the core binding strategy "striding" out of the 
*     query.
* 
*     The query string is expected to have following syntax: 
*    
*           "striding:<amount>:<stepsize>[:<socket>,<core>]"
*
*  INPUTS
*     const char* parameter - Points to the string with the query. 
*
*  RESULT
*     int - Returns the step size or -1 when it could not been parsed. 
*
*  NOTES
*     MT-NOTE: binding_striding_parse_step_size() is not MT safe 
*
*******************************************************************************/
int binding_striding_parse_step_size(const char* parameter)
{
   /* striding:<amount>:<step-size>:  */ 
   if (parameter != NULL && strstr(parameter, "striding") != NULL) {
      /* fetch "striding:" */
      if (sge_strtok(parameter, ":") != NULL) {
         if (sge_strtok(NULL, ":") != NULL) {
            /* fetch step size */
            char* stepsize = NULL;
            if ((stepsize = sge_strtok(NULL, ":")) != NULL 
                  && is_digit(stepsize, ':')) {
                  /* the step size must be followed by " " or ":" or "\0" 
                     in order to avoid garbage like "striding:2:0,0" */
                  /* return step size */
                  return atoi(stepsize);
            }
         }
      }
   }
   
   /* in default case take each core */
   return -1;
}

/****** uti/binding_hlp/binding_get_topology_for_job() ********************
*  NAME
*     binding_get_topology_for_job() -- Returns topology string. 
*
*  SYNOPSIS
*     const char *
*     binding_get_topology_for_job(const char *binding_result)
*
*  FUNCTION
*     Returns the topology string of a host where the cores are 
*     marked with lowercase letters for those cores that were
*     "bound" to a certain job.
*     
*     It is assumed the 'binding_result' parameter that is
*     passed to this function was previously returned by
*     create_binding_strategy_string()
*
*  INPUTS
*     const char *binding_result - string returned by
*                         create_binding_strategy_string()
*
*  RESULT
*     const char * - topology string like "SCc"
*******************************************************************************/
const char *
binding_get_topology_for_job(const char* binding_result) {
   const char *topology_result = NULL;

   if (binding_result != NULL) {
      /* find test after last colon character including this character */
      topology_result = strrchr(binding_result, ':');

      /* skip colon character */
      if (topology_result != NULL) {
         topology_result++;
      }
   }
   return topology_result;
}

/****** sge_binding_hlp/topology_string_to_socket_core_lists() *****************
*  NAME
*     topology_string_to_socket_core_lists() -- Converts a topology into socket,core lists. 
*
*  SYNOPSIS
*     bool topology_string_to_socket_core_lists(const char* topology, int** 
*     sockets, int** cores, int* amount) 
*
*  FUNCTION
*     Converts a topology string into lists of cores and sockets which are marked 
*     as beeing used and returns them.
*
*  INPUTS
*     const char* topology - Pointer to a topology string.
*
*  OUTPUTS
*     int** sockets        - Pointer to the location of the socket array.
*     int** cores          - Pointer to the location of the core array. 
*     int* amount          - Length of the arrays.  
*
*  RESULT
*     bool - false when problems occured true otherwise
*
*  NOTES
*     MT-NOTE: topology_string_to_socket_core_lists() is MT safe 
*
*******************************************************************************/
bool
topology_string_to_socket_core_lists(const char* topology, int** sockets, 
                                     int** cores, int* amount) {
   bool retval = true;

   int current_socket = -1;
   int current_core   = -1;

   *amount = 0;
   
   if (topology == NULL || *sockets != NULL || *cores != NULL) {
      /* we expect to have clean input */
      retval = false;
   } else {
   
      while (*topology != '\0') {

         if (*topology == 'S' || *topology == 's') {
            current_socket++;
            current_core = -1;
         } else if (*topology == 'C') {
            /* this core is not in use */
            current_core++;
         } else if (*topology == 'c') {
            /* this core is in use hence we are collecting it */
            (*amount)++;
            current_core++;
            *sockets = sge_realloc(*sockets, (*amount) * sizeof(int),1 );
            *cores   = sge_realloc(*cores, (*amount) * sizeof(int), 1);
            (*sockets)[(*amount)-1] = current_socket;
            (*cores)[(*amount)-1]   = current_core;
         }

         topology++;
      }

   }

   return retval;
}

/****** sge_binding_hlp/get_explicit_number() **********************************
*  NAME
*     get_explicit_number() -- Counts the number of <socket,core> pairs.
*
*  SYNOPSIS
*     int get_explicit_number(const char* expl)
*
*  FUNCTION
*     Counts the number of <socket,core> pairs in the binding explicit request.
*
*  INPUTS
*     const char* expl - pointer to explicit binding request string
*     const bool with_explicit_prefix - does string start with "explicit:"?
*
*  RESULT
*     int - number of <socket,core> pairs in explicit binding request string
*
*  NOTES
*     MT-NOTE: get_explicit_number() is MT safe
*
*  SEE ALSO
*     sge_binding_hlp/check_explicit_binding_string()
*******************************************************************************/
int
get_explicit_number(const char* expl, const bool with_explicit_prefix) {

   int amount = 0;
   char* pair = NULL;
   struct saved_vars_s* context = NULL;

   if (expl == NULL) {
      return amount;
   }   

   pair = sge_strtok_r(expl, ":", &context);

   if (pair == NULL) {
      sge_free_saved_vars(context);
      return amount;
   }

   if (with_explicit_prefix == false) {
      /* it begins with a pair */
      amount++;
   }   
  
   while (sge_strtok_r(NULL, ":", &context) != NULL) {
      amount++;
   }

   sge_free_saved_vars(context);

   return amount;
}

/****** sge_binding_hlp/check_explicit_binding_string() ************************
*  NAME
*     check_explicit_binding_string() -- Checks binding string for duplicate pairs.
*
*  SYNOPSIS
*     bool check_explicit_binding_string(const char* expl, const int amount)
*
*  FUNCTION
*     Checks binding string for duplicate <socket,core> pairs. Works 
*     also when the first pair is the "explicit" string.
*
*  INPUTS
*     const char* expl - pointer to the explicit binding request
*     const int amount - expected number of pairs
*     const bool with_explicit_prefix - expl start with "excplicit:"
*
*  RESULT
*     bool - true if the explicit binding request is duplicate free
*
*  NOTES
*     MT-NOTE: check_explicit_binding_string() is MT safe
*
*  SEE ALSO
*     sge_binding_hlp/get_explicit_number()
*******************************************************************************/
bool
check_explicit_binding_string(const char* expl, const int amount, 
                              const bool with_explicit_prefix)
{
   bool success = true;
   struct saved_vars_s* context = NULL;
   
   /* pointer to the first position of all <socket,core> pairs */
   int pair_number = 0;
   char* pairs[amount];
   char* pair = NULL;
   
   if (expl == NULL || amount == 0) {
      return false;
   }

   /* skip "explicit:" */
   if (with_explicit_prefix == true) {
      pair = sge_strtok_r(expl, ":", &context);
      if (pair == NULL) {
         success = false;
      }   
   }

   /* get pointer to first pair */
   if (success == true) {
      if (with_explicit_prefix == true) {  
         pair = sge_strtok_r(NULL, ":", &context);
      } else {
         pair = sge_strtok_r(expl, ":", &context);
      }
      if (pair == NULL) {
         success = false;
      }   
   }

   /* store pointer to first <socket,core> pair */
   if (success == true) {
      pairs[pair_number] = pair;
      pair_number++;
   }

   /* split string in <socket,core> pairs and store them */
   while ((success == true) && (pair = sge_strtok_r(NULL, ":", &context)) != NULL) {
      if (pair_number > amount) {
         /* found more pairs than expected */
         success = false;
         break;
      }
      /* save string and check if it is unique */
      pairs[pair_number] = pair;
      pair_number++;
   }

   /* check if number of pairs did match */
   if (success == true && pair_number != amount) {
      success = false;
   }

   /* check if there is a duplicate <socket,core> pair */
   if (success == true) {
      int i,j;
      for (i = 0; i < amount && success == true; i++) {
         for (j = i+1; j < amount; j++) {
            if (strcmp(pairs[i], pairs[j]) == 0) {
               /* identical <socket,core> pair found -> illegal */
               success = false;
               break;
            }
         }
      }
   }

   sge_free_saved_vars(context);

   return success;
}

/****** sge_binding_hlp/is_digit() *********************************************
*  NAME
*     is_digit() -- Checks if char array consists of digits. 
*
*  SYNOPSIS
*     static bool is_digit(const char* position, const char stopchar) 
*
*  FUNCTION
*     Checks if the character array contains only digits till the end of the 
*     array or to a given stop character. If there is no digit found or 
*     the input pointer is a null pointer 'false' is returned. 
*     The input pointer have to point to the first digit of a number.
*
*  INPUTS
*     const char* position - pointer to the character array 
*     const char stopchar  - stop scanning at this parameter 
*
*  RESULT
*     static bool - true if a number is found false if not  
*
*  NOTES
*     MT-NOTE: is_digit() is MT safe 
*
*******************************************************************************/
static bool is_digit(const char* position, const char stopchar) {

   if (position == NULL || *position == '\0' || !isdigit(*position)) {
      return false;
   }

   position++;

   while (position != NULL && *position != '\0' && *position != stopchar) {
      if (!isdigit(*position)) {
         return false;
      }
      position++;
   }

   return true;
}
