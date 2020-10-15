#!/bin/csh -fb
#
#
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

#===================================
#for comfortable use:
#alias dl 'set argv = ( \!* ); source $SGE_ROOT/util/dl2.csh'
#
#===================================
#
#configuration:
#
#
#	list of layers:
#
set layer_name = (t c b g s h a p)
set layer_full = 255
#
#	list of classes:
#
set class_name = (t i j V m X Y Z)
set class_full = 255
#
#	default value for no debug 
#	(one '0' for each layer)
#
set no_debug = (0 0 0 0 0 0 0 0)
#
#===================================

#===================================
#
#PARSING
#
#===================================

#-----------------------------------
#
#compute bit mask for layers
#

set is_layer = 0
set layer_mask = 0

while ( $#argv > 0 ) 
	@ i = 0
	foreach layer ( $layer_name )
		if ($argv[1] == $layer ) then
			@ layer_mask += (1<<$i)
			set is_layer = 1
			break
		endif
		@ i++
	end
	if ( $is_layer != 1  ) break
	set is_layer = 0
	shift
end 
if ( $layer_mask == 0 ) then
   set layer_mask = $layer_full
endif
unset is_layer


#-----------------------------------
#
#watch out for command
#

set command 
if ( $#argv > 0 ) then 
	switch ($argv[1])
		case "+":
			set command = add 
			breaksw
		case "-":
			set command = sub 
			breaksw
		case "=":
			set command = set 
			breaksw
		default:
         echo "dl2.csh: unknown command $argv[1]"
			goto usage
			breaksw
	endsw
	shift
else 
	set command = ask
endif

#-----------------------------------
#
#compute bit mask for classes
#

set is_class = 0
set class_mask = 0

while ( $#argv > 0 ) 
	@ i = 0
	set is_class = 0
	foreach class ( $class_name )
		if ($argv[1] == $class ) then
			@ class_mask += (1<<$i)
			set is_class = 1
			break
		endif
		@ i++
	end
	if ( $is_class != 1  ) then
      echo "dl2.csh: unknown class $argv[1]"
      goto usage
   endif
	shift
end 
if ( $class_mask == 0 ) then
   set class_mask = $class_full
endif
unset is_class


#===================================
#
#EXECUTING
#
#===================================

set selected
set VAR 

#get old value of SGE_DEBUG_LEVEL or default value
if ( $?SGE_DEBUG_LEVEL ) then
 	set debug = ($SGE_DEBUG_LEVEL)
else
 	set debug = ($no_debug)
endif

@ i = 1
foreach layer ( $layer_name )

	set old = "$debug[$i]"
	@ selected = ($layer_mask & (1<<($i - 1)))
   if ( $selected ) then
      switch ( $command )
         case set:
            @ old = $class_mask
            breaksw
         case add:
            @ old = ($old|$class_mask)
            breaksw
         case sub:
            @ old = ($old&(~ $class_mask))
            breaksw
         case ask:
            set classes  
            @ j = 0
            foreach class ( $class_name )
               if ( $old & (1<<$j) ) then
                  set classes = "$classes$class "
               else
                  set classes = "$classes  "
               endif	
               @ j++
            end
            echo "layer: $layer class: $classes"
            breaksw
      endsw
   endif

	set VAR = "$VAR $old"
	@ i++
end

setenv SGE_DEBUG_LEVEL "$VAR"
setenv SGE_ND true
setenv SGE_ENABLE_COREDUMP true

unset VAR
unset debug 
unset old 
unset i
unset class
unset classes
unset class_name
unset class_mask
unset command
unset selected
unset layer
unset layer_name
unset layer_mask

exit

usage: 
#===================================
#
#PRINT USAGE ON SYNTAX ERROR
#
#===================================
echo "Usage: dl [$layer_name][+-=][$class_name]"
exit 1
