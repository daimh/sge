#___INFO__MARK_BEGIN__
##########################################################################
#
#  The Contents of this file are made available subject to the terms of
#  the Sun Industry Standards Source License Version 1.2
#
#  Sun Microsystems Inc., March, 2001
#
#
#  Sun Industry Standards Source License Version 1.2
#  =================================================
#  The contents of this file are subject to the Sun Industry Standards
#  Source License Version 1.2 (the "License"); You may not use this file
#  except in compliance with the License. You may obtain a copy of the
#  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
#
#  Software provided under this License is provided on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#  See the License for the specific provisions governing your rights and
#  obligations concerning the Software.
#
#  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#  Copyright: 2001 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
##########################################################################
#___INFO__MARK_END__

global INFOTEXTIGNORE INFOTEXTCMD INPUTFILE INFOTEXTBINARY UPPER_ARCH OUTPUTFILE MESSAGE_OPT env
set INFOTEXTCMD "\$INFOTEXT"
set INFOTEXTIGNORE "INFOTEXT="
set INPUTFILE ""
set OUTPUTFILE "stdout"
set MESSAGE_OPT "-message"
set INFOTEXTBINARY "[ set env(BUILDARCH)]/infotext"

proc is_continued { line } {
   set linelength [ string length $line ]
   incr linelength -1
   if { [string last "\\" $line] == $linelength } {
      return true
   }
   return false
}

proc create_script { command } {
   set id [ open "test.sh" "w" ]
   puts $id "#!/bin/sh"
   catch {
      puts $id $command
   }
   close $id
   exec chmod 755 test.sh
   return "test.sh"
}

proc do_replace { i_string what replace_str {only_last 0} } { 

   set s_command $i_string
   while { 1 } {
      if { $only_last != 0 } {
         set where [string last $what $s_command]
      } else {
         set where [string first $what $s_command]
      }
      if { $where < 0 } {
         break;
      } 
      set slen [ string length $what]
      set s_command [ string replace $s_command $where [expr ($where + $slen - 1)] $replace_str ]
      if { $only_last != 0 } {
         return $s_command
      }
   }
  return $s_command
}

if { [ file exists $INFOTEXTBINARY ] != 1 } {
     puts "error: file \"$INFOTEXTBINARY\" not found!"
     exit 1
}

if { [info exists argc ] != 0 } {
   if { $argc == 0 } {
      puts "usage:"
      puts "infotext_msg_parse \[-P INFOTEXTCMD\] inputfile\n"
      puts "options:"
      puts "  -P INFOTEXTCMD   -  parse for INFOTEXTCMD in inputfile. This is"
      puts "                      the infotext call (default is \"INFOTEXT\""
      puts "  -F PO_FILE_NAME  -  generate po message file"
      puts "  -U               -  make special messages with _"
   } else {
      # start
      for { set i 0 } { $i < $argc } { incr i } {
         set arg [lindex $argv $i]
         switch -- $arg {
            "-P" {  
               # -P INFOTEXTCMD
               if { $i < [ expr ( $argc - 1 ) ] } {
                  incr i
                  set INFOTEXTCMD [lindex $argv $i]
               } else {
                  puts "error parsing option -P"
                  exit 1
               }
               continue
            }
            "-F" {  
               # -F PO_FILE_NAME 
               if { $i < [ expr ( $argc - 1 ) ] } {
                  incr i
                  set OUTPUTFILE [lindex $argv $i]
               } else {
                  puts "error parsing option -F"
                  exit 1
               }
               continue
            }
            "-U" {  
               # -U  
               set MESSAGE_OPT "-message-space"
               continue
            }


         }
         if { $i == [ expr ( $argc - 1 ) ] } {
            set INPUTFILE $arg
         }
      }
      if { $INPUTFILE == "" } {
         puts "no input file"
         exit 1
      }
      #
      # main start
      #
      puts "searching for \"$INFOTEXTCMD\" commands\nin file \"$INPUTFILE\" ..."

      # read in script file
      set script  [ open "$INPUTFILE" "r" ]
      set i 0
      while { [gets $script line] >= 0 } {
         set buffer($i) $line
         incr i 1
      }
      close $script
      if { [string compare $OUTPUTFILE "stdout" ] != 0 } {
         set output_file [ open $OUTPUTFILE "w" ]
      } else {
         set output_file "stdout"
      }
      set max_line [ expr $i - 1 ]

      set i 0
      set command ""
      while { $i <= $max_line } {
         set command ""
         set line $buffer($i)
         set line [ string trim $line  ]
         set where [string first $INFOTEXTCMD $line]
         if { $where >= 0 && [string first $INFOTEXTIGNORE $line] < 0 } {
            set slen [ string length $INFOTEXTCMD ]
            set hline [ string replace $line $where [expr ($where + $slen - 1)] "$INFOTEXTBINARY $MESSAGE_OPT"  ]
            append command "$hline"
            while { [is_continued $line] == "true" } {
               # remove last \ from command line
               set where [ string last "\\" $command ]
               set command [ string replace $command $where end "" ]
               set command [ string trim $command ]  
               incr i 1
               set line $buffer($i)
               set line [ string trim $line  ]
               append command " $line"
            }
            # here we have the complete lines with the command
            # first remove $VARIABLES with a default text
            set command [ do_replace $command "\\$" "_THIS_IS_A_PLACEHOLDER__" ]
            # handle -auto switch            
            if { [ string first "-auto" $command] >= 0 } {
                # get -auto parameter
                set where [string first "-auto" $command]
                set auto_para [ string replace $command 0 [ expr ( $where + 4 ) ] "" ]
                set auto_para [ string trim $auto_para ]
                set where [string first " " $auto_para]
                set auto_para [ string replace $auto_para $where end "" ]

                # now replace auto_para with "true"
                set where [string first $auto_para $command]
                set where_end [ expr ( [string length $auto_para] -1 ) + $where ]
                set command [ string replace $command $where $where_end "true"  ]
            } 


            # try if error occur
            set command [ do_replace $command "$" "_NOT_A_VARIABLE_ERROR" 0 ]
            set catch_return [ catch { eval exec [create_script $command] } output ]
            if { $catch_return != 0 } {
               # we had an error  
               set line_nr [expr ($i +1)]
               puts "syntax error! parsing line $line_nr"
               exit -1
            } 

            # remove \$
            set command [ do_replace $command "_THIS_IS_A_PLACEHOLDER__" "\\$" ]


            # now it is ok
            set catch_return [ catch { eval exec [ create_script $command] } output ]
            if { $catch_return != 0 } {
               puts "---------ERROR-------:"
               puts $command
               puts $output
               puts "\nis this a localization message call or something else? message ignored!!"
               # puts "press ENTER to continue with message parsing, the message is ignored!!!"
               # gets stdin test
            } else {
               # write output
               
               if { [string first "_NOT_A_VARIABLE_ERROR" $output ] >= 0 } { 
                   set line_nr [expr ($i +1)]
                   puts "---------ERROR-------:"
                   puts $command
                   puts $output
                   puts "line: $line_nr - _NOT_A_VARIABLE_ERROR can't be message text"
                   exit -1
               }
               set line_nr [expr ($i +1)]
               puts $output_file "###########################################################"
               puts $output_file "# file: [file tail $INPUTFILE], line $line_nr"
#               set output [ do_replace $output "\"" "" ]
#               set o_len [string length $output]
#               for {set pos 0} { $pos < $o_len } { incr pos 80 } {
#                  if { $pos >= $o_len } {
#                     #last line
#                     puts $output_file "[string range $output $pos end]"
#                  } else {
#                     puts $output_file [string range $output $pos [expr ( $pos + 79 )]]
#                  }
#               }
               puts $output_file $output
            }
         }
         incr i 1
         if { [ string compare $output_file "stdout"] != 0  } {
            puts -nonewline "\r       \rline: $i "
            flush stdout
         }
      }
      flush $output_file
      puts ""
      if { [ string compare $output_file "stdout"] != 0 } {
         close $output_file
         puts "output written to file\n$OUTPUTFILE"
      }
   }
}
