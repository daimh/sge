#!/bin/tclsh8.3
# expect script 
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


# This script is used to insert an ADOC header into a TCL or C source code file!
# ADOC (Extract docs from source code) has a GNU General Public License 
# and was written by Tobias Ferber <ukjg@rz.uni-karlsruhe.de>
# 
global ADOC_START ADOC_END ADOC_FIRSTOFLINE
global OUTPUT_FILE SOURCE_FILE SOURCE_MODE
global SOURCE_PROC      SOURCE_PROC_COUNT
global SOURCE_ADOC_PROC SOURCE_ADOC_PROC_COUNT
global INTERACTIVE_FLAG
global MODULE_NAME REPLACE_FLAG
global ONLY_ONE_FUNCTION
global LOG_OUTPUT

set LOG_OUTPUT stderr
set INTERACTIVE_FLAG 0
set REPLACE_FLAG 0

set SOURCE_MODE "undefined"
set SOURCE_FILE "undefined"
set OUTPUT_FILE "undefined"
set MODULE_NAME "undefined"
set ONLY_ONE_FUNCTION "undefined"

set ADOC_START(C)       "/*"
set ADOC_END(C)         "*/"
set ADOC_FIRSTOFLINE(C) "*"

set ADOC_START(TCL)       "#"
set ADOC_END(TCL)         "#"
set ADOC_FIRSTOFLINE(TCL) "#"

set MAX_LINE_LENGTH 79




#
#                                                             max. column:     |
#
#****** doc_tool/format_text() ******
#  NAME
#     format_text -- ??? 
#
#  SYNOPSIS
#     format_text { text t_start t_end } 
#
#  FUNCTION
#     ??? 
#
#  INPUTS
#     text    - ??? 
#     t_start - ??? 
#     t_end   - ??? 
#
#  RESULT
#     ??? 
#
#  EXAMPLE
#     ??? 
#
#  NOTES
#     ??? 
#
#  BUGS
#     ??? 
#
#  SEE ALSO
#     ???/???
#*******************************
#
proc format_text { text t_start t_end } {

   global LOG_OUTPUT

   set word_list ""
   set new_text [ split $text "\n" ]
   foreach elem $new_text { 
      foreach word $elem {
         lappend word_list $word
      }
      lappend word_list "_new_line_"
   }
   set return_list ""
   set act_line ""
   for {set i 0} {$i < $t_start } {incr i 1 } {
      append act_line " "
   }
   
   foreach elem $word_list {
      if { [ string compare $elem "_new_line_" ] == 0 } {
         lappend return_list $act_line
         set act_line ""
         for {set i 0} {$i < $t_start } {incr i 1 } {
            append act_line " "
         }
         continue
      }
      set act_line_length [ string length $act_line ]
      set act_word_length [ string length $elem ]
      if { [ expr ( $act_line_length + $act_word_length ) ] >= $t_end } {
         lappend return_list $act_line
         set act_line ""
         for {set i 0} {$i < $t_start } {incr i 1 } {
            append act_line " "
         }
      }
      append act_line "$elem "
   }


   return $return_list
}

#
#                                                             max. column:     |
#
#****** doc_tool/create_doc_header() ******
#  NAME
#     create_doc_header -- ??? 
#
#  SYNOPSIS
#     create_doc_header { output module function short_description 
#     long_description synompsis input result example notes bugs see_also 
#     { style C } } 
#
#  FUNCTION
#     ??? 
#
#  INPUTS
#     output            - ??? 
#     module            - ??? 
#     function          - ??? 
#     short_description - ??? 
#     long_description  - ??? 
#     synompsis         - ??? 
#     input             - ??? 
#     result            - ??? 
#     example           - ??? 
#     notes             - ??? 
#     bugs              - ??? 
#     see_also          - ??? 
#     { style C }       - ??? 
#
#  RESULT
#     ??? 
#
#  EXAMPLE
#     ??? 
#
#  NOTES
#     ??? 
#
#  BUGS
#     ??? 
#
#  SEE ALSO
#     ???/???
#*******************************
#
proc create_doc_header { output module function short_description long_description synompsis input result example notes bugs see_also { style C }  } {
   global ADOC_START ADOC_END ADOC_FIRSTOFLINE MAX_LINE_LENGTH ONLY_ONE_FUNCTION LOG_OUTPUT

   upvar $input myinput 
   upvar $see_also mylink

   if { [ string compare "undefined" $ONLY_ONE_FUNCTION] != 0 } {
      if { [ string compare $function $ONLY_ONE_FUNCTION ] != 0 } {
         return
      }  
   }

   puts $LOG_OUTPUT "insert \"$style\"-Style header for function \"$function\"..."

   if { [ string first "*" $ADOC_START($style) ] >= 0 } {
      set output_text "$ADOC_START($style)***** $module/${function}() ******"
      set output_length [ string length "${output_text}" ]
      set space_text ""
      set nr_spaces [ expr ( 80 - $output_length ) ]
      for { set i 0} {$i < $nr_spaces} {incr i 1} {
         append space_text "*" 
      }
      puts $output "${output_text}${space_text}" 
   } else {
      set output_text "$ADOC_START($style)****** $module/${function}() ******"
      set output_length [ string length "${output_text}" ]
      set space_text ""
      set nr_spaces [ expr ( 80 - $output_length ) ]
      for { set i 0} {$i < $nr_spaces} {incr i 1} {
         append space_text "*" 
      }
      puts $output "${output_text}${space_text}" 
   }
   puts $output "$ADOC_FIRSTOFLINE($style)  NAME"
   set tmp_txt [ format_text "${function}() -- $short_description" 5 $MAX_LINE_LENGTH ]
   puts $output "$ADOC_FIRSTOFLINE($style)[lindex $tmp_txt 0]"

   if { [ string compare $synompsis "" ] != 0 } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  SYNOPSIS"
      set tmp_txt [ format_text $synompsis 5 $MAX_LINE_LENGTH ]
      foreach elem $tmp_txt {
         puts $output "$ADOC_FIRSTOFLINE($style)$elem"
      }
   }

   if { [ string compare $long_description "" ] != 0 } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  FUNCTION"
      set tmp_txt [ format_text $long_description 5 $MAX_LINE_LENGTH ]
      foreach elem $tmp_txt {
         puts $output "$ADOC_FIRSTOFLINE($style)$elem"
      }
   }

   if { [ info exists myinput(param) ] == 1 && [ info exists myinput(description) ] == 1 } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  INPUTS"


      set dummy ""
      set dummy_length 0
      for {set i 0} {$i < [llength $myinput(param) ] } {incr i} {
          set p_leng [ string length [ lindex $myinput(param) $i ] ]
          if { $dummy_length < $p_leng } {
             set dummy_length $p_leng
          }
      }
      for {set i 0} {$i < $dummy_length } {incr i} {
          append dummy " "
      }


      for {set i 0} {$i < [llength $myinput(param) ] } {incr i} {
#          puts $output "$ADOC_FIRSTOFLINE($style)     [ lindex $myinput(param) $i ]   - [ lindex $myinput(description) $i ]"

         set tmp_txt  [ format_text "[ lindex $myinput(description) $i ]" 0 [ expr ( $MAX_LINE_LENGTH - $dummy_length - 3 ) ] ]
         set first 0
         foreach elem $tmp_txt {
            if { $first == 0 } {
               set missing [ expr ( $dummy_length -  [ string length [ lindex $myinput(param) $i ] ] ) ]
               set missing_txt ""
               for {set m 0} {$m < $missing } {incr m} {
                   append missing_txt " "
               }

               puts $output "$ADOC_FIRSTOFLINE($style)     [ lindex $myinput(param) $i ]$missing_txt - $elem"
               incr first 1
            } else {
               puts $output "$ADOC_FIRSTOFLINE($style)     $dummy   $elem"
            }
         }
      }
   }

   if { [ string compare $result "" ] != 0 } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  RESULT"
      set tmp_txt [ format_text $result 5 $MAX_LINE_LENGTH ]
      foreach elem $tmp_txt {
         puts $output "$ADOC_FIRSTOFLINE($style)$elem"
      }

   }

   if { [ string compare $example "" ] != 0 } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  EXAMPLE"
      set tmp_txt [ format_text $example 5 $MAX_LINE_LENGTH ]
      foreach elem $tmp_txt {
         puts $output "$ADOC_FIRSTOFLINE($style)$elem"
      }

   }

   if { [ string compare $notes "" ] != 0 } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  NOTES"
      set tmp_txt [ format_text $notes 5 $MAX_LINE_LENGTH ]
      foreach elem $tmp_txt {
         puts $output "$ADOC_FIRSTOFLINE($style)$elem"
      }

   }

   if { [ string compare $bugs "" ] != 0 } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  BUGS"
      set tmp_txt [ format_text $bugs 5 $MAX_LINE_LENGTH ]
      foreach elem $tmp_txt {
         puts $output "$ADOC_FIRSTOFLINE($style)$elem"
      }

   }

   if { [ info exists mylink(module) ] == 1 && [ info exists mylink(function) ] == 1  } {
      puts $output "$ADOC_FIRSTOFLINE($style)"
      puts $output "$ADOC_FIRSTOFLINE($style)  SEE ALSO"
      for {set i 0} {$i < [llength $mylink(module) ] } {incr i} {
         puts $output "$ADOC_FIRSTOFLINE($style)     [ lindex $mylink(module) $i ]/[ lindex $mylink(function) $i ]"
      }

   }
   set output_text "$ADOC_FIRSTOFLINE($style)*******************************"
   set output_length [ string length "${output_text}" ]
   set space_text ""
   set nr_spaces [ expr ( 80 - $output_length ) ]
   if { [ string compare "TCL" $style ] != 0 } {
      incr nr_spaces -2
   }
   for { set i 0} {$i < $nr_spaces} {incr i 1} {
      append space_text "*" 
   }
   puts -nonewline $output "${output_text}${space_text}" 
   if { [ string compare "TCL" $style ] != 0 } { 
      puts $output "$ADOC_END($style)"
   } else {
      puts $output ""
   }
}


#
#                                                             max. column:     |
#
#****** doc_tool/get_user_input() ******
#  NAME
#     get_user_input -- ??? 
#
#  SYNOPSIS
#     get_user_input { what def } 
#
#  FUNCTION
#     ??? 
#
#  INPUTS
#     what - ??? 
#     def  - ??? 
#
#  RESULT
#     ??? 
#
#  EXAMPLE
#     ??? 
#
#  NOTES
#     ??? 
#
#  BUGS
#     ??? 
#
#  SEE ALSO
#     ???/???
#*******************************
#
proc get_user_input { what def } {
   global LOG_OUTPUT
   puts -nonewline $what
   flush stdout 
   gets stdin myinput
   if { [ string compare $myinput "" ] == 0 } {
      return $def
   }
   return $myinput
}

#
#                                                             max. column:     |
#
#****** doc_tool/get_char_count() ******
#  NAME
#     get_char_count -- ??? 
#
#  SYNOPSIS
#     get_char_count { the_string charstring } 
#
#  FUNCTION
#     ??? 
#
#  INPUTS
#     the_string - ??? 
#     charstring - ??? 
#
#  RESULT
#     ??? 
#
#  EXAMPLE
#     ??? 
#
#  NOTES
#     ??? 
#
#  BUGS
#     ??? 
#
#  SEE ALSO
#     ???/???
#*******************************
#
proc get_char_count { the_string charstring } {

   global LOG_OUTPUT
   set help $the_string
   set counter 0

   while { [ string first $charstring $help ] >= 0 } {
       incr counter 1
       set pos [  string first $charstring $help ]
       set help [ string range $help [ expr ( $pos + 1 ) ] end ]
   }
   return $counter
}

proc get_char_count_without_strings { the_string charstring } {
   global LOG_OUTPUT
   
   set new_string [ get_line_without_strings $the_string ]
   return [  get_char_count "$new_string" "$charstring" ]
}

proc get_line_without_strings { the_string  } {
   global LOG_OUTPUT
   set help $the_string
   set without_string ""
   set counter 0
   set string_found 0

   foreach delemitter "{\"} {\`} {\'}" {
      while { [ string first $delemitter $help ] >= 0 } {
          set string_found 1
          set pos [  string first  $delemitter $help ]
          append without_string [ string range $help 0 [ expr ( $pos + 0 ) ] ]
          set help [ string range $help [ expr ( $pos + 1 ) ] end ]
          set pos [  string first  $delemitter $help ]
          set help [ string range $help [ expr ( $pos + 1 ) ] end ]
          append without_string $delemitter
      }
      if { $string_found == 1 } {
         append without_string $help
      } else {
         set without_string $help
      }
      set counter 0
      set string_found 0
      set help $without_string
      set without_string ""
   }
 
   while { [ string first "/*" $help ] >= 0 } {
       set string_found 1
       set pos [  string first "/*" $help ]
       append without_string [ string range $help 0 [ expr ( $pos + 0 ) ] ]
       set help [ string range $help [ expr ( $pos + 1 ) ] end ]
       set pos [  string first  "*/" $help ]
       set help [ string range $help [ expr ( $pos + 1 ) ] end ]
       append without_string "*/"
   }
   if { $string_found == 1 } {
      append without_string $help
   } else {
      set without_string $help
   }
   set counter 0
   set string_found 0
   set help $without_string
   set without_string ""

   return $help
}

#
#                                                             max. column:     |
#
#****** doc_tool/parse_tcl_file() ******
#  NAME
#     parse_tcl_file -- ??? 
#
#  SYNOPSIS
#     parse_tcl_file { file } 
#
#  FUNCTION
#     ??? 
#
#  INPUTS
#     file - ??? 
#
#  RESULT
#     ??? 
#
#  EXAMPLE
#     ??? 
#
#  NOTES
#     ??? 
#
#  BUGS
#     ??? 
#
#  SEE ALSO
#     ???/???
#*******************************
#
proc parse_tcl_file { file } {
  
   global SOURCE_PROC SOURCE_PROC_COUNT LOG_OUTPUT
   global SOURCE_ADOC_PROC SOURCE_ADOC_PROC_COUNT
   
   set SOURCE_PROC_COUNT 0
   if { [ info exists SOURCE_PROC ] } {
      unset SOURCE_PROC
   }
   set SOURCE_ADOC_PROC_COUNT 0
   if { [ info exists SOURCE_ADOC_PROC ] } {
      unset SOURCE_ADOC_PROC
   }
   set line_nr 0
   set fd [ open $file "r" ]

   while { [gets $fd line] >= 0 } {
      puts -nonewline $LOG_OUTPUT "line nr: $line_nr     \r"
      flush stdout
      set help [ string trim $line ]

      set proc_pos [ string first "proc" $help ]
      set first_com [ string first "#" $help ]

      if { $first_com == 0 } {
#          puts "found comment: $line"
          set com_pos [ string first "******" $line ]
          if { $com_pos >= 0 } {
              set help [ string range $line [ expr ( $com_pos + 6 ) ] end ]
              set help [ split $help "\/" ]
              if { [ llength $help ] > 1 } {
#                 puts "found adoc header in line $line_nr"
                 set adoc_module [ lindex [ lindex $help 0 ] 0 ]
                 set adoc_function [ lindex [ lindex $help 1 ] 0 ]
                 set SOURCE_ADOC_PROC($SOURCE_ADOC_PROC_COUNT,proc_name) $adoc_function
                 set SOURCE_ADOC_PROC($SOURCE_ADOC_PROC_COUNT,line) $line_nr
                 incr SOURCE_ADOC_PROC_COUNT 1
              }
          }
      }
      set line_incrementation 1 
      if { $proc_pos == 0 } {

          set last  [ string last  "\{" $help ]
          set last_close [ string last "\}" $help  ] 

          if { $last < $last_close } {
             puts $LOG_OUTPUT "proc definition has more than one line !!!"


             while { $last < $last_close } {
                gets $fd new_line
                incr line_incrementation 1
                set help "$help [ string trim $new_line ]"

                set last  [ string last  "\{" $help ]
                set last_close [ string last "\}" $help  ] 
                puts $LOG_OUTPUT "new proc line, lines: $line_incrementation:\n$help"
             }

#             set no_open   [ get_char_count $help "\{" ]
#             set no_closed [ get_char_count $help "\}" ]

#             puts "open:   $no_open"
#             puts "closed: $no_closed"   
          }



          set first [ string first "\{" $help ]
          set line_length [ string length $help ]

 

          if { $proc_pos < $first && $first >= 0 } {
             set proc_name [ string range $help 5 [ expr ( $first - 2 ) ] ]
             set help2 [ string range $help [ expr ( $first - 2 ) ] end ]
             set last  [ string last  "\}" $help2 ]
             incr last -1
             set first [ string first "\{" $help2 ]
             incr first 1
             set proc_para1 [ string range $help2 $first $last ]
             set proc_para ""
             foreach param $proc_para1 {
                if { [ llength $param ] > 1 } {
                    lappend proc_para "\{$param\}"
                } else {
                    lappend proc_para $param
                }
             }
#             puts "found procedure in line $line_nr"
             set SOURCE_PROC($SOURCE_PROC_COUNT,proc_name) $proc_name
             set SOURCE_PROC($SOURCE_PROC_COUNT,proc_para) $proc_para
             set SOURCE_PROC($SOURCE_PROC_COUNT,line) $line_nr
             set SOURCE_PROC($SOURCE_PROC_COUNT,is_new) 1
             incr SOURCE_PROC_COUNT 1
          }
      }
      incr line_nr $line_incrementation
   }
   close $fd

   for { set pr 0 } { $pr<$SOURCE_PROC_COUNT } { incr pr 1 } {
      for { set adoc 0 } { $adoc<$SOURCE_ADOC_PROC_COUNT } { incr adoc 1 } {
          set adoc_proc_name $SOURCE_ADOC_PROC($adoc,proc_name)
          if { [ string first "()" $adoc_proc_name] >= 0 } {
             set adoc_proc_name [ string range $adoc_proc_name 0 [ expr ( [string length $adoc_proc_name] - 3 ) ] ]
          }
          if { [ string compare $SOURCE_PROC($pr,proc_name) $adoc_proc_name ] == 0  } {
              puts $LOG_OUTPUT "procedure \"$SOURCE_PROC($pr,proc_name)\" in line $SOURCE_PROC($pr,line) has allready an adoc header (line $SOURCE_ADOC_PROC($adoc,line))"
              set SOURCE_PROC($pr,is_new) 0
          }
      } 
   }   

}

#
#                                                             max. column:     |
#
#****** doc_tool/parse_c_file() ******
#  NAME
#     parse_c_file -- ??? 
#
#  SYNOPSIS
#     parse_c_file { file } 
#
#  FUNCTION
#     ??? 
#
#  INPUTS
#     file - ??? 
#
#  RESULT
#     ??? 
#
#  EXAMPLE
#     ??? 
#
#  NOTES
#     ??? 
#
#  BUGS
#     ??? 
#
#  SEE ALSO
#     ???/???
#*******************************
#
#
proc parse_c_file { file } {
  
   global SOURCE_PROC SOURCE_PROC_COUNT LOG_OUTPUT
   global SOURCE_ADOC_PROC SOURCE_ADOC_PROC_COUNT
   
   set SOURCE_PROC_COUNT 0
   if { [ info exists SOURCE_PROC ] } {
      unset SOURCE_PROC
   }
   set SOURCE_ADOC_PROC_COUNT 0
   if { [ info exists SOURCE_ADOC_PROC ] } {
      unset SOURCE_ADOC_PROC
   }
   set line_nr 0
   set fd [ open $file "r" ]

   set no_open  0
   set no_close 0
   set open_buffer "" 
   set commend_mode 0       

   while { [gets $fd line] >= 0 } {
      incr line_nr 1
      set line_buffer($line_nr) "$line "
      puts -nonewline $LOG_OUTPUT "line nr: $line_nr     \r"
      flush stdout
     
      set com_pos [ string first "****** " $line ]
      if { $com_pos >= 0 } {
         set adoc_help [ string range $line [ expr ( $com_pos + 6 ) ] end ]
         set adoc_help [ split $adoc_help "\/" ]
         if { [ llength $adoc_help ] > 1 } {
                 set adoc_module [ lindex [ lindex $adoc_help 0 ] 0 ]
                 set adoc_function [ lindex [ lindex $adoc_help 1 ] 0 ]
                 set ignore [ string first "(" $adoc_function ]
                 if { $ignore >= 0 } {
                    set adoc_function [ string range $adoc_function 0 [ expr ( $ignore - 1 ) ] ]
                 }
                 puts $LOG_OUTPUT "found adoc header for \"$adoc_function\" in line $line_nr"

#                 puts "procedure: \"$adoc_function\""
                 set SOURCE_ADOC_PROC($SOURCE_ADOC_PROC_COUNT,proc_name) $adoc_function
                 set SOURCE_ADOC_PROC($SOURCE_ADOC_PROC_COUNT,line) $line_nr
                 incr SOURCE_ADOC_PROC_COUNT 1
              }
      }
      set long_commend 0
      if { $commend_mode == 1 } {
         set long_commend 1
      } 
      if { [ string first "/*" [get_line_without_strings $line] ] >= 0 } {
         set commend_mode 1       
      }
      if { [ string first "*/" [get_line_without_strings $line] ] >= 0 } {
         if { $long_commend == 0 } {
            set commend_mode 0
         }
      }

      if { $commend_mode == 0 } { 
         incr no_open  [ get_char_count_without_strings $line "\{" ]
         if { $no_open > 0 } {
            lappend open_buffer $line_nr
         } 
         incr no_close [ get_char_count_without_strings $line "\}" ]
      } else {
         puts $LOG_OUTPUT "ignoring line $line_nr"
      }
      if { [ string first "*/" [get_line_without_strings $line] ] >= 0 } {
         set commend_mode 0
      }

      

      if { $no_open == $no_close && $no_open != 0 } {
         set _open  0
         set _close 0
         set _buf_line [ lindex $open_buffer 0 ]
         set no_func_found 0
#         puts "found evtl. procedure, line $_buf_line"
 
         if { $_buf_line > 0 } { 
            if { [string first "struct" $line_buffer($_buf_line)] >= 0  } {
               puts $LOG_OUTPUT "--> struct found"
               set no_func_found 1
            } 

            set temp1 [ string first "\{"  $line_buffer($_buf_line) ]
            set temp2 [ string first "="  $line_buffer($_buf_line) ]
            
            if { $temp1 > $temp2 && $temp2 >= 0 } {
               set no_func_found 1
               puts $LOG_OUTPUT "--> equal found, line $_buf_line"
            }
                        


         }

         set commend_mode 0         

         while { $_buf_line > 0 && $no_func_found == 0} {
             set tmp_buffer $line_buffer($_buf_line)
             set commend_start [ string first "/*" $tmp_buffer ] 
             set commend_end   [ string first "*/" $tmp_buffer ]
             if { $commend_end >= 0 } {
                 set commend_mode 1
             }
             if { $commend_start >= 0 } {
                 set commend_mode 0 
             }

             if { $commend_start >= 0 && $commend_end >= 0 } {
               set help  [ string range $tmp_buffer 0 [ expr ( $commend_start - 1 ) ] ]
               set help2 [ string range $tmp_buffer [ expr ( $commend_end + 2 ) ] end ]
               set tmp_buffer "${help}${help2}"
#               puts "*** ignoring comment in function declaration ***"
             } 
             if { $commend_mode == 0 } {
                incr _open  [ get_char_count_without_strings $tmp_buffer "(" ]
                incr _close [ get_char_count_without_strings $tmp_buffer ")" ]
             }
#           puts "$line_buffer($_buf_line)"
    #         puts "[ string first ";" $line_buffer($_buf_line) ]"
    #         puts "[ string first "typedef" $line_buffer($_buf_line) ]"
#             puts $tmp_buffer
             foreach not_allowed "#include RCSID #define \}" {
                if { [ string first $not_allowed $tmp_buffer ] >= 0 } {
                   set no_func_found 1
                   puts $LOG_OUTPUT "no func found \"$not_allowed\""
                   break
                }
             }
             if { $no_func_found == 1 } {
                break
             }

             if { $_open == $_close && $_open != 0 } {
                if { [string first "#define" $line_buffer($_buf_line)] > 0 ||
                     [string first "struct" $line_buffer($_buf_line)] > 0 ||
                     [string first "typedef" $line_buffer($_buf_line)] > 0 ||
                     [string first ";" $line_buffer($_buf_line)] > 0   } {
                   set no_func_found 1
                   puts $LOG_OUTPUT "typedef or struct or declaration found"
                   break
                }
#                set func_temp_name1 $line_buffer([expr ($_buf_line-1)])
#                puts $func_temp_name1
#                if { [ string first "struct" $func_temp_name1 ] >= 0 } {
#                   set no_func_found 1
#                   break
#                }
      
    
                if { [ string first "\}" $line_buffer([expr ($_buf_line-1)])] < 0 &&
                     [ string first "*/" $line_buffer([expr ($_buf_line-1)])] < 0 && 
                     [ string first "#"  $line_buffer([expr ($_buf_line-1)])] < 0 && 
                     [ string first "typedef"  $line_buffer([expr ($_buf_line-1)])] < 0 && 
                     [ string first ";"  $line_buffer([expr ($_buf_line-1)])] < 0 } {
                     incr _buf_line -1
                     set SOURCE_PROC($SOURCE_PROC_COUNT,line) [ expr ( $_buf_line - 1 ) ] 
                     set SOURCE_PROC($SOURCE_PROC_COUNT,is_new) 1
 
                     break
                }
#                puts "C function start at line $_buf_line"
                set SOURCE_PROC($SOURCE_PROC_COUNT,line) [ expr ( $_buf_line - 1 ) ]
                set SOURCE_PROC($SOURCE_PROC_COUNT,is_new) 1

                break
             }
             incr _buf_line -1
         }
         
#         if { $_buf_line > 192 } { exit -1 }      
         if { $_buf_line == 0 } {
            set no_func_found 1 
         }
 
         if { $no_func_found == 1 } {
            set no_open 0
            set no_close 0
            set open_buffer ""
            puts $LOG_OUTPUT "continue"
            continue
         }

#        start at $_buf_line, end at [ lindex $open_buffer 0 ]        
         set function_start $_buf_line
         set function_end [ lindex $open_buffer 0 ]
     
#         puts "function start at $_buf_line, end: [ lindex $open_buffer 0 ] "
 
         set function_text "" 
         for { set i $function_start } { $i <= $function_end } { incr i 1 } {
             append function_text $line_buffer($i)
         }

         set do_run 1
         while { $do_run } { 
            set commend_start [ string first "/*" $function_text ] 
            set commend_end   [ string first "*/" $function_text ]
            if { $commend_start >= 0 && $commend_end >= 0 } {
               set help  [ string range $function_text 0 [ expr ( $commend_start - 1 ) ] ]
               set help2 [ string range $function_text [ expr ( $commend_end + 2 ) ] end ]
               set function_text "${help}${help2}"

            } else {
               set do_run 0
            }
         }

#         puts $LOG_OUTPUT "function text: $function_text"
         set pr_pos [ string first "PR_" $function_text ]
         if { $pr_pos >= 0 } {
            puts $LOG_OUTPUT "WARNING: removing PR_ macro"
            puts $LOG_OUTPUT "\"[string range $function_text 0 [ expr ( $pr_pos - 1 ) ] ]\""
            puts $LOG_OUTPUT "\"[string range $function_text [ expr ( $pr_pos + 3 ) ] end]\""
            set function_text_tmp [string range $function_text 0 [ expr ( $pr_pos - 1 ) ]]
            append function_text_tmp [string range $function_text [ expr ( $pr_pos + 3 ) ] end] 
            set function_text $function_text_tmp

            set pr_pos [ string first "(" $function_text]
            set function_text_tmp [string range $function_text 0 [ expr ( $pr_pos - 1 ) ]]
            append function_text_tmp [string range $function_text [ expr ( $pr_pos + 1 ) ] end] 
            set function_text $function_text_tmp
            
            set pr_pos [ string last ")" $function_text]
            set function_text_tmp [string range $function_text 0 [ expr ( $pr_pos - 1 ) ]]
            append function_text_tmp [string range $function_text [ expr ( $pr_pos + 1 ) ] end] 
            set function_text $function_text_tmp


#            puts $LOG_OUTPUT "function text: $function_text"
         }

         set func_name_end [ string first "(" $function_text ]
         set value_test [string first "=" $function_text ]
         set test2 [ string first "\{" $function_text ]
         set test3 [ string first "\"" $function_text ]

#         puts "func_name_end is $func_name_end"

         if { $value_test < $func_name_end && $value_test >= 0 } {
            puts $LOG_OUTPUT "this is no function this is value setup (equal)"
            set no_open 0
            set no_close 0
            set open_buffer ""
            continue
         }
         if { $test2 < $func_name_end && $test2 >= 0 } {
            
            puts $LOG_OUTPUT "this is no function this is value setup (\})"
            set no_open 0
            set no_close 0
            set open_buffer ""
            continue
         }
         if { $test3 < $func_name_end && $test3 >= 0 } {
            
            puts $LOG_OUTPUT "this is no function this is value setup (\")"
            set no_open 0
            set no_close 0
            set open_buffer ""
            continue
         }
         

#         puts "function_text is $function_text"

         set function_full_name [ string range $function_text 0 [ expr ( $func_name_end - 1 ) ] ]
#         puts "ok $function_full_name"
       
 
         set help "[llength $function_full_name]"

         set function_return ""
         for { set i 0 } { $i < [ expr ( $help - 1 ) ] } { incr i 1 } {
             append function_return "[ lindex $function_full_name $i ] "
         }
         set function_name   [ lindex $function_full_name $i ]
         if { [string compare "if" $function_name] == 0 } {
            puts $LOG_OUTPUT "this is no function this is if-statement"
            set no_open 0
            set no_close 0
            set open_buffer ""
            continue

         }

         set function_return [ string trim $function_return ]

           

         while { [ string index $function_name 0 ] == "*" } {
            set function_name [ string range $function_name 1 end ]
            append function_return "*"
         }

         puts $LOG_OUTPUT "found \"C\" function \"$function_name\" in line $SOURCE_PROC($SOURCE_PROC_COUNT,line)"
#         puts "function_return: \"$function_return\""
         set SOURCE_PROC($SOURCE_PROC_COUNT,proc_name) $function_name 
         set SOURCE_PROC($SOURCE_PROC_COUNT,proc_return) $function_return

         set func_param_end [ string first ")" $function_text ]
         set function_parameter_list [ string range $function_text [ expr ( $func_name_end + 1 ) ] [ expr ( $func_param_end - 1 ) ] ]
         set function_parameter_list [ split $function_parameter_list "," ]
#         puts "arguments: [ llength $function_parameter_list]"
         set function_parameter_types [ string range $function_text [ expr ( $func_param_end + 1 ) ] end  ]
 
         set SOURCE_PROC($SOURCE_PROC_COUNT,proc_para) ""
         set elem_counter 0
         set type_split [ split $function_parameter_types ";"]   

         foreach elem $function_parameter_list {
            set var_name [ string trim $elem ]
            if { [ llength $var_name ] == 1 } {
               set argument_string [lindex $type_split $elem_counter]
               set var_pos [ string first $var_name $argument_string ]
               if { $var_pos >= 0 } {
                  set var_type [ string range $argument_string 0 [ expr ( $var_pos - 1 ) ] ]
                  set temp_var_type [ split $var_type "*" ]
                  set temp_var_index 0 
                  set mdmd ""
                  foreach temp_var_elem $temp_var_type {
                     append mdmd [string trim $temp_var_elem]
                     if { $temp_var_index > 0 } {
                         append mdmd "*" 
                     }
                     incr temp_var_index 1
                  }
                  set var_type $mdmd
                  set var_type [ string trim $var_type ]
                  set var_name "$var_type $var_name" 
#                  puts "$var_name\n"
               }
            } 
#            puts "\"$var_name\"" 
            lappend SOURCE_PROC($SOURCE_PROC_COUNT,proc_para) $var_name
            incr elem_counter 1
         }      

#         puts $function_text
        
         set no_open 0
         set no_close 0
         set open_buffer ""
         incr SOURCE_PROC_COUNT 1
      }
   }
   close $fd
   puts $LOG_OUTPUT "                 \r"
   for { set pr 0 } { $pr<$SOURCE_PROC_COUNT } { incr pr 1 } {
      for { set adoc 0 } { $adoc<$SOURCE_ADOC_PROC_COUNT } { incr adoc 1 } {
          set adoc_proc_name $SOURCE_ADOC_PROC($adoc,proc_name)
          if { [ string first "()" $adoc_proc_name] >= 0 } {
             set adoc_proc_name [ string range $adoc_proc_name 0 [ expr ( [string length $adoc_proc_name] - 3 ) ] ]
          }
          if { [ string compare $SOURCE_PROC($pr,proc_name) $adoc_proc_name ] == 0  } {
              puts $LOG_OUTPUT "procedure \"$SOURCE_PROC($pr,proc_name)\" in line $SOURCE_PROC($pr,line) has allready an adoc header (line $SOURCE_ADOC_PROC($adoc,line))"
              set SOURCE_PROC($pr,is_new) 0
          }
      } 
   }   

   
}



#
#                                                             max. column:     |
#
#****** doc_tool/generate_output_file() ******
#  NAME
#     generate_output_file -- ??? 
#
#  SYNOPSIS
#     generate_output_file { source_file } 
#
#  FUNCTION
#     ??? 
#
#  INPUTS
#     source_file - ??? 
#
#  RESULT
#     ??? 
#
#  EXAMPLE
#     ??? 
#
#  NOTES
#     ??? 
#
#  BUGS
#     ??? 
#
#  SEE ALSO
#     ???/???
#*******************************
#
proc generate_output_file { source_file {reset_output_file 1}} {
   
   global OUTPUT_FILE SOURCE_MODE SOURCE_PROC_COUNT SOURCE_ADOC_PROC_COUNT
   global MODULE_NAME SOURCE_PROC SOURCE_ADOC_PROC INTERACTIVE_FLAG
   global REPLACE_FLAG ONLY_ONE_FUNCTION LOG_OUTPUT

   if { $reset_output_file == 1 } {
      set OUTPUT_FILE "${source_file}.out"
      puts $LOG_OUTPUT "output file is \"$OUTPUT_FILE\"" 
   }

   if { [ string compare $SOURCE_MODE "TCL" ] == 0 } {
      puts $LOG_OUTPUT "file \"$source_file\""
      parse_tcl_file $source_file
      puts $LOG_OUTPUT "found $SOURCE_PROC_COUNT TCL procedure(s)."
      puts $LOG_OUTPUT "found $SOURCE_ADOC_PROC_COUNT ADOC header(s)."
   
      set module_name [ file rootname $source_file ] 
      set module_name [ file tail $module_name ]
   
      if { [ string compare $MODULE_NAME "undefined" ] != 0 } {
         set module_name $MODULE_NAME
      } 
   
      set link ""
      unset link 
      set link(module)  "???"
      set link(function) "???"
      
      set source_desc [ open $source_file "r" ]
      if { [string compare $OUTPUT_FILE "stdout" ] == 0 } {
         set target_file "stdout"
      } else {
         set target_file [ open $OUTPUT_FILE "w" ]
      }
      set act_line 0
    
      for { set i 0 } { $i<$SOURCE_PROC_COUNT } { incr i 1 } {
   
   
         if { $SOURCE_PROC($i,is_new) == 0 } {
   #         puts "\"$SOURCE_PROC($i,proc_name)\" is an existing header - continue ..."
            continue
         }
   
         while { $act_line < $SOURCE_PROC($i,line) } {
             puts -nonewline $LOG_OUTPUT "line nr: $act_line     \r"
             flush stdout
             gets $source_desc line
             if { [ string compare "undefined" $ONLY_ONE_FUNCTION] == 0 } {
                puts $target_file $line 
             }
             incr act_line 1
         }
   
   
         set par_count [ llength $SOURCE_PROC($i,proc_para) ]
         set params(param) ""
         set params(description) ""
         foreach elem $SOURCE_PROC($i,proc_para) { 
            lappend params(param) [ string trim $elem] 
            lappend params(description) "???"
         }
   
         set head_module     $module_name
         set head_function   $SOURCE_PROC($i,proc_name)
         set head_short_desc "???"
         set head_long_desc  "???"
         set head_synopsis   "$SOURCE_PROC($i,proc_name) \\\{ $SOURCE_PROC($i,proc_para) \\\}"
         set head_input      params
         set head_results    "???"
         set head_example    "???"
         set head_notes      "???"
         set head_bugs       "???"
         set head_see_also   link
         
         if { $INTERACTIVE_FLAG == 1 } {
            puts "\n*******************************************************************************" 
            puts "procedure \"$head_function\" in module $head_module:"
            set head_short_desc [ get_user_input "short description:(do not go behind the \"*\"-line)\n     $head_function -- " "???" ]           
            set head_long_desc [ get_user_input "long description:\n" "???" ]
   
            set params(description) ""
            puts "parameter: $params(param)"
            foreach elem $params(param) {
               lappend params(description) [ get_user_input "parameter $elem:\n" "???" ] 
            } 
   
            set head_results [ get_user_input "results:\n" "???" ]
            set head_example [ get_user_input "example:\n" "???" ]
            set head_notes [ get_user_input "notes:\n" "???" ]
            set head_bugs [ get_user_input "bugs:\n" "???" ]
   
            set back [ get_user_input "add link? (y/n)" "n" ]
            if { [ string compare "y" $back ] == 0 } {
               set link(module)  ""
               set link(function) ""
               while { 1 } {
                  set tmp_mod [ get_user_input "enter module name or \"x\" to stop\n" "" ]
                  if { [ string compare $tmp_mod "x" ] == 0 } {
                     break
                  }
                  set tmp_func [ get_user_input "enter function name or \"x\" to stop\n" "" ]
                  if { [ string compare $tmp_func "x" ] == 0 } {
                     break
                  }
                  lappend link(module) $tmp_mod
                  lappend link(function) $tmp_func
               }
               if { [string compare $link(module) "" ] == 0 } {
                  set link(module)  "???"
                  set link(function) "???"
               } 
            }
      
         }
         
         create_doc_header $target_file $head_module $head_function $head_short_desc $head_long_desc $head_synopsis $head_input $head_results $head_example $head_notes $head_bugs $head_see_also TCL 
      }
   
      while { [gets $source_desc line] >= 0 } {
         puts -nonewline $LOG_OUTPUT "line nr: $act_line     \r"
         flush stdout
         if { [ string compare "undefined" $ONLY_ONE_FUNCTION] == 0 } {
            puts $target_file $line 
         }
         incr act_line 1
      }
   
   
      flush $target_file
      if { [string compare $target_file "stdout" ] != 0 } {
         close $target_file
      }
      close $source_desc
      puts $LOG_OUTPUT "$act_line lines parsed! Output file was \"$OUTPUT_FILE\"." 

      if { $REPLACE_FLAG == 1 } {
         puts $LOG_OUTPUT "replacing \"$source_file\" with \"$OUTPUT_FILE\""
         file delete $source_file
         file rename $OUTPUT_FILE $source_file
      } 
   }

   if { [ string compare $SOURCE_MODE "C" ] == 0 } {
      puts $LOG_OUTPUT "file \"$source_file\""
      parse_c_file $source_file

      puts $LOG_OUTPUT "found $SOURCE_PROC_COUNT C function(s)."
      puts $LOG_OUTPUT "found $SOURCE_ADOC_PROC_COUNT ADOC header(s)."
      
      if { $SOURCE_PROC_COUNT == 0 } {
      }
 
      set module_name [ file rootname $source_file ] 
      set module_name [ file tail $module_name ]
   
      if { [ string compare $MODULE_NAME "undefined" ] != 0 } {
         set module_name $MODULE_NAME
      } 
   
      set link ""
      unset link 
      set link(module)  "???"
      set link(function) "???"
      
      set source_desc [ open $source_file "r" ]
      if { [string compare $OUTPUT_FILE "stdout" ] == 0 } {
         set target_file "stdout"
      } else {
         set target_file [ open $OUTPUT_FILE "w" ]
      }
      set act_line 0
    
      for { set i 0 } { $i<$SOURCE_PROC_COUNT } { incr i 1 } {
   
   
         if { $SOURCE_PROC($i,is_new) == 0 } {
   #         puts "\"$SOURCE_PROC($i,proc_name)\" is an existing header - continue ..."
            continue
         }
   
         while { $act_line < $SOURCE_PROC($i,line) } {
             puts -nonewline $LOG_OUTPUT "line nr: $act_line     \r"
             flush stdout
             gets $source_desc line
             if { [ string compare "undefined" $ONLY_ONE_FUNCTION] == 0 } {
                puts $target_file $line 
             }
             incr act_line 1
         }
   
   
         set par_count [ llength $SOURCE_PROC($i,proc_para) ]
         set params(param) ""
         set params(description) ""
         foreach elem $SOURCE_PROC($i,proc_para) { 
            lappend params(param) [ string trim $elem] 
            lappend params(description) "???"
         }
   
         set head_module     $module_name
         set head_function   "$SOURCE_PROC($i,proc_name)"
         set head_short_desc "???"
         set head_long_desc  "???"
         set head_synopsis   "$SOURCE_PROC($i,proc_return) "
         append head_synopsis "$SOURCE_PROC($i,proc_name)"
         append head_synopsis "("
         
         set have_params 0
         foreach o $SOURCE_PROC($i,proc_para) {
             append head_synopsis "$o, "
             set have_params 1
         }

         
         if { $have_params == 1 } {
            set head_synopsis [ string range $head_synopsis 0 [ expr ( [ string length $head_synopsis ] - 3 ) ] ]
         } 

         append head_synopsis ")"
         set head_input      params
         set head_results    "$SOURCE_PROC($i,proc_return) - "
         set head_example    "???"
         set head_notes      "???"
         set head_bugs       "???"
         set head_see_also   link
         
         if { $INTERACTIVE_FLAG == 1 } {
            puts "\n*******************************************************************************" 
            puts "procedure \"$head_function\" in module $head_module:"
            set head_short_desc [ get_user_input "short description:(do not go behind the \"*\"-line)\n     $head_function -- " "???" ]           
            set head_long_desc [ get_user_input "long description:\n" "???" ]
   
            set params(description) ""
            puts "parameter: $params(param)"
            foreach elem $params(param) {
               lappend params(description) [ get_user_input "parameter $elem:\n" "???" ] 
            } 
   
            set head_results [ get_user_input "results:\n" "???" ]
            set head_example [ get_user_input "example:\n" "???" ]
            set head_notes [ get_user_input "notes:\n" "???" ]
            set head_bugs [ get_user_input "bugs:\n" "???" ]
   
            set back [ get_user_input "add link? (y/n)" "n" ]
            if { [ string compare "y" $back ] == 0 } {
               set link(module)  ""
               set link(function) ""
               while { 1 } {
                  set tmp_mod [ get_user_input "enter module name or \"x\" to stop\n" "" ]
                  if { [ string compare $tmp_mod "x" ] == 0 } {
                     break
                  }
                  set tmp_func [ get_user_input "enter function name or \"x\" to stop\n" "" ]
                  if { [ string compare $tmp_func "x" ] == 0 } {
                     break
                  }
                  lappend link(module) $tmp_mod
                  lappend link(function) $tmp_func
               }
               if { [string compare $link(module) "" ] == 0 } {
                  set link(module)  "???"
                  set link(function) "???"
               } 
            }
      
         }
         
         create_doc_header $target_file $head_module "$head_function" $head_short_desc $head_long_desc $head_synopsis $head_input $head_results $head_example $head_notes $head_bugs $head_see_also C 
      }
   
      while { [gets $source_desc line] >= 0 } {
         puts -nonewline $LOG_OUTPUT "line nr: $act_line     \r"
         flush stdout
         if { [ string compare "undefined" $ONLY_ONE_FUNCTION] == 0 } {
             puts $target_file $line 
         }
         incr act_line 1
      }
   
   
      flush $target_file
      if { [string compare $target_file "stdout" ] != 0 } {
         close $target_file
      }
      close $source_desc
      puts $LOG_OUTPUT "$act_line lines parsed! Output file was \"$OUTPUT_FILE\"." 

      if { $REPLACE_FLAG == 1 } {
         puts $LOG_OUTPUT "replacing \"$source_file\" with \"$OUTPUT_FILE\""
         file delete $source_file
         file rename $OUTPUT_FILE $source_file
      } 
   }
   
}

# Main programm:
for { set i 0 } { $i < $argc } { incr i } {
   if {([string compare [lindex $argv $i] "--help"] == 0) || ([string compare [lindex $argv $i] "help"] == 0) } {
      puts "\nusage: expect doc_tool.tcl \[options\] \[<inputfile>|<glob_expr>|\[<inputfile>|<glob_expr>\]\]"
      puts "\n  <glob_expr> e.g. \"*.tcl\"," 
      puts "  <inputfile> e.g. \"tcl_files/sge_procedures.tcl\n"
      puts "\noptions are:"
      puts "  help          - show this"
      puts "  no_warnings   - produce no warning/error messages"
      puts "  func <name>   - only write ADOC header for function <name>"
      puts "  mode <type>   - setting adoc header mode. Available types:"
      puts "                  TCL, C"
      puts "  output <file> - using file as output file or \"stdout\""
      puts "  module <name> - set module name (default is root file name)"
      puts "  interactive   - ask for function descriptions"
      puts "  replace       - replace source file with target file after inserting headers" 
      puts "                  (file backup recommended)"
      puts "\nexample:"
      puts "  doc_tool.tcl mode TCL */*.tcl *.tcl"
      exit 0
   }
   if {([string compare [lindex $argv $i] "--no_warnings"] == 0) || ([string compare [lindex $argv $i] "no_warnings"] == 0) } {
      set LOG_OUTPUT [ open "/dev/null" "w"]
      continue
   }
 
   if { ( [string compare [lindex $argv $i] "--module"] == 0 ) || ([string compare [lindex $argv $i] "module"] == 0) } {
      incr i
      set MODULE_NAME [lindex $argv $i]
      continue
   }

   if { ( [string compare [lindex $argv $i] "--replace"] == 0 ) || ([string compare [lindex $argv $i] "replace"] == 0) } {
      set REPLACE_FLAG 1
      continue
   }

   if { ( [string compare [lindex $argv $i] "--mode"] == 0 ) || ([string compare [lindex $argv $i] "mode"] == 0) } {
      incr i
      set SOURCE_MODE [lindex $argv $i]
      if { [ string compare $SOURCE_MODE "TCL"] != 0 && [ string compare $SOURCE_MODE "C"] != 0 } {
          puts "wrong mode, should be TCL or C"
          exit -1
      }
      continue
   }

   if { ( [string compare [lindex $argv $i] "--func"] == 0 ) || ([string compare [lindex $argv $i] "func"] == 0) } {
      incr i
      set ONLY_ONE_FUNCTION [lindex $argv $i]
      continue
   }

   

   if { ( [string compare [lindex $argv $i] "--interactive"] == 0 ) || ([string compare [lindex $argv $i] "interactive"] == 0) } {
      set INTERACTIVE_FLAG 1
      puts "I will ask for unknown function descriptions (interactive flag is set)"
      continue
   }


   if { ( [string compare [lindex $argv $i] "--output"] == 0 ) || ([string compare [lindex $argv $i] "output"] == 0) } {
      incr i
      set OUTPUT_FILE [lindex $argv $i]
      continue
   }
   set SOURCE_FILE [lrange $argv $i end]
   puts $LOG_OUTPUT "There are [llength $SOURCE_FILE] files to check"
   break
}   


# main
if { [ string compare $SOURCE_FILE "undefined" ] == 0 } {
   puts "no source file"
   exit -1
}

set files ""
foreach elem $SOURCE_FILE {
   lappend files [ glob $elem ]
}
set nr 1
if { [ llength $files ] > 1 } {
   puts "input files:"
   foreach elem $files {
      puts -nonewline $LOG_OUTPUT "\r  $nr: \"$elem\"\r"
      set prefix ""
      if { $nr < 10 } {
         set prefix "00"
      } 
      if { $nr < 100  && $nr >= 10 } {
         set prefix "0"
      }
      puts "$prefix$nr: \"$elem\""
      incr nr 1
   }
   if { [ string compare $OUTPUT_FILE "undefined"  ] != 0 } {
      puts "can't use one output file for more input files"
      exit -1
   } 
   if { [ string compare "y" [ get_user_input "are the source files correct (y/n): ? " "n" ] ] != 0 } {
      puts "break!"
      exit -1
   }
}

set SOURCE_FILE $files
if { [llength $SOURCE_FILE] > 1 } {
   foreach filename $SOURCE_FILE {
      puts $LOG_OUTPUT "************ $filename ********"
      generate_output_file $filename
   }
} else {
   generate_output_file $SOURCE_FILE 0
}
if { [ string compare $SOURCE_MODE "undefined" ] == 0 } {
   puts "no mode selected! use help parameter" 
}

