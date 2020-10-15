#!/bin/sh

output=$1

echo "/* auto generated file */"             > $output
echo "#include \"cull.h\""                   >>$output
echo "#include \"sge_all_listsL.h\""         >>$output
#echo "#include \"sge_object.h\""             >>$output
echo ""                                      >>$output
echo "const lDescr *"                        >>$output
echo "object_get_subtype(int nm)"            >>$output
echo "{"                                     >>$output
echo "   const lDescr *ret = NULL;"          >>$output
echo "   switch (nm) {"                      >>$output
while read dummy name type; do
   if [ $type != CULL_ANY_SUBTYPE ]; then
      echo "      case $name:"               >>$output
      echo "         ret = $type;"           >>$output
      echo "         break;"                 >>$output
   fi
done
echo "   }"                                  >>$output
echo "   return ret;"                        >>$output
echo "}"                                     >>$output
