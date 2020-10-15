#!/bin/sh
#
# Saves cluster configuration into a directory structure.
# Scriptname: save_sge_config.sh
# Module: common functions
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
#  Portions of this code are Copyright 2011 Univa Inc.
#
##########################################################################
#___INFO__MARK_END__

INFOTEXT=echo

if [ -z "$SGE_ROOT" -o -z "$SGE_CELL" ]; then
   $INFOTEXT "Set your SGE_ROOT, SGE_CELL first!"
   exit 1
fi

ARCH=`$SGE_ROOT/util/arch`

MKDIR=mkdir
QCONF=$SGE_ROOT/bin/$ARCH/qconf
QHOST=$SGE_ROOT/bin/$ARCH/qhost
HOST=`$SGE_ROOT/utilbin/$ARCH/gethostname -aname`


Usage()
{
   myname=`basename $0`
   $INFOTEXT \
             "Usage: $myname <backup_dir>|[-help]

<backup_dir> ... Directory in which to store the backup.
                The directory must either be empty or not exist at all."
}

#Dump list to dir
DumpListToLocation()
{
   list=$1
   dir=$2
   opt=$3
   if [ ! -d $dir ]; then
     $MKDIR -p $dir
   fi
   if [ -n "$list" ]; then   
      for item in $list; do
         DumpItemToFile $item $dir $opt
      done
   fi
}


#Dump item to file
DumpItemToFile()
{
   item=$1
   dir=$2
   opt=$3

   $QCONF $opt $item > $dir/${item}
   if [ $? -ne 0 ]; then
      $INFOTEXT "Operation failed: $QCONF $opt > $dir/${item}"
      return 1
   fi
   return 0
}


#Dump qconf option to file
DumpOptionToFile()
{
   opt=$1
   file=$2

   $QCONF $opt > ${file} 2>${file}.err
   ret=$?
   resMsg=`cat ${file}.err`
   rm -f ${file}.err
   if [ $ret -ne 0 ]; then
      case "$resMsg" in
         'no '*)
            return 0
	    ;;
	 *)	
            $INFOTEXT "Operation failed: qconf $opt - $resMsg"
            return 1
	    ;;
      esac		
   fi
}


#Backup selected files from SgeCell (bootstrap, etc.)
BackupSgeCell()
{
   VERSION=`$QCONF -help | sed  -n '1,1 p'` 2>&1
   ret=$?
   if [ "$ret" = "0" ]; then
      echo $VERSION > $DEST_DIR/version
   else
      $INFOTEXT "[CRITICAL] qmaster is not installed"
      exit 1
   fi
   $QCONF -sq > /dev/null 2>&1
   ret=$?
   if [ "$ret" != "0" ]; then
      $INFOTEXT "[CRITICAL] qmaster is not running"
      exit 1
   fi
	
   #sge_root
   cat $SGE_ROOT/$SGE_CELL/common/settings.sh | grep "SGE_ROOT" | head -1 > "${DEST_DIR}/sge_root"
   #sge_cell
   cat $SGE_ROOT/$SGE_CELL/common/settings.sh | grep "SGE_CELL" | head -1 > "${DEST_DIR}/sge_cell"
   #qmaster_port, execd_port
   cat $SGE_ROOT/$SGE_CELL/common/settings.sh | grep "PORT" > "${DEST_DIR}/ports"
	
   #arseqnum, jobseqnum
   tmp_spool=`cat $SGE_ROOT/$SGE_CELL/common/bootstrap | grep qmaster_spool_dir | awk '{ print $2 }'`
   cp ${tmp_spool}/arseqnum "$DEST_DIR" 2>/dev/null
   cp ${tmp_spool}/jobseqnum "$DEST_DIR" 2>/dev/null

   #SGE_CELL content
   mkdir -p "$DEST_DIR/cell"
   cp $SGE_ROOT/$SGE_CELL/common/act_qmaster "${DEST_DIR}/cell"
   cp $SGE_ROOT/$SGE_CELL/common/bootstrap "${DEST_DIR}/cell"
   #cluster_name
   if [ -f $SGE_ROOT/$SGE_CELL/common/cluster_name ]; then
      cp $SGE_ROOT/$SGE_CELL/common/cluster_name "${DEST_DIR}/cell"
   fi
   #host_aliases
   if [ -f $SGE_ROOT/$SGE_CELL/common/host_aliases ]; then
      cp $SGE_ROOT/$SGE_CELL/common/host_aliases "${DEST_DIR}/cell"
   fi
   #qtask
   if [ -f $SGE_ROOT/$SGE_CELL/common/qtask ]; then
      cp $SGE_ROOT/$SGE_CELL/common/qtask "${DEST_DIR}/cell"
   fi
   #sge_aliases
   if [ -f $SGE_ROOT/$SGE_CELL/common/sge_aliases ]; then
      cp $SGE_ROOT/$SGE_CELL/common/sge_aliases "${DEST_DIR}/cell"
   fi
   #sge_ar_request
   if [ -f $SGE_ROOT/$SGE_CELL/common/sge_ar_request ]; then
      cp $SGE_ROOT/$SGE_CELL/common/sge_ar_request "${DEST_DIR}/cell"
   fi
   #sge_request
   if [ -f $SGE_ROOT/$SGE_CELL/common/sge_request ]; then
      cp $SGE_ROOT/$SGE_CELL/common/sge_request "${DEST_DIR}/cell"
   fi
   #sge_qstat
   if [ -f $SGE_ROOT/$SGE_CELL/common/sge_qstat ]; then
      cp $SGE_ROOT/$SGE_CELL/common/sge_qstat "${DEST_DIR}/cell"
   fi
   #sge_qquota
   if [ -f $SGE_ROOT/$SGE_CELL/common/sge_qquota ]; then
      cp $SGE_ROOT/$SGE_CELL/common/sge_qquota "${DEST_DIR}/cell"
   fi
   #sge_qrstat
   if [ -f $SGE_ROOT/$SGE_CELL/common/sge_qrstat ]; then
      cp $SGE_ROOT/$SGE_CELL/common/sge_qrstat "${DEST_DIR}/cell"
   fi
   
   #shadow_masters
   if [ -f $SGE_ROOT/$SGE_CELL/common/shadow_masters ]; then
      cp $SGE_ROOT/$SGE_CELL/common/shadow_masters "${DEST_DIR}/cell"
   fi	
   #Accounting file
   if [ -r "$SGE_ROOT/$SGE_CELL/common/accounting" ]; then
      cp "$SGE_ROOT/$SGE_CELL/common/accounting" "${DEST_DIR}/cell"
   fi	
   #Save dbwriter.conf for simple dbwriter upgrade, if present
   if [ -r "$SGE_ROOT/$SGE_CELL/common/dbwriter.conf" ]; then
      cp "$SGE_ROOT/$SGE_CELL/common/dbwriter.conf" "${DEST_DIR}/cell"
   fi
   #Save JMX settings if present
   if [ -d "$SGE_ROOT/$SGE_CELL/common/jmx" ]; then
      cp -r "$SGE_ROOT/$SGE_CELL/common/jmx" "${DEST_DIR}/cell/jmx"
   fi
   # Hadoop settings, if present
   if [ -d "$SGE_ROOT/$SGE_CELL/common/hadoop" ]; then
      cp -r "$SGE_ROOT/$SGE_CELL/common/hadoop" "${DEST_DIR}/cell/hadoop"
   fi
   # Fixme: should (any of) the CA be copied?

   #Save cluster's windows host info
   res=`$QHOST -l "arch=win*" 2>/dev/null`
   if [ -n "$res" ]; then     # has windows hosts
      echo "$res" | grep "win*" | awk '{print $1}' > "${DEST_DIR}"/win_hosts
   fi
}



########
# MAIN #
########
if [ "$1" = "-help" -o $# -eq 0 ]; then
   Usage
   exit 0
fi

#Dump all curent configuration to the temp directory called ccc
DEST_DIR="${1:?The save directory is required}"

admin_hosts=`$QCONF -sh 2>/dev/null`
if [ -z "$admin_hosts" ]; then
   $INFOTEXT "ERROR: qconf -sh failed. Qmaster is probably not running?"
   exit 1
fi
tmp_adminhost=`$QCONF -sh | grep "^${HOST}$"`
if [ "$tmp_adminhost" != "$HOST" ]; then
   $INFOTEXT "ERROR: Load must be started on admin host (qmaster host recommended)."
   exit 1
fi

if [ ! -d "$DEST_DIR" ]; then
   $MKDIR -p $DEST_DIR
elif [ `ls -1 "$DEST_DIR" | wc -l` -gt 0 ]; then
   echo "ERROR: The save directory $DEST_DIR must be empty"
   exit 1
fi

date '+%Y-%m-%d_%H:%M:%S' > "$DEST_DIR/backup_date"
BackupSgeCell

OLD_SGE_LINE="$SGE_SINGLE_LINE"
SGE_SINGLE_LINE=1
export SGE_SINGLE_LINE

#There are the show options, which are not used
#
#     -sds                          <show detached settings>
#     -secl                         <show event clients>
#     -sep                          <show licensed processors>
#     -shgrp_tree group             <show host group tree>
#     -shgrp_resolved               <show host group hosts>
#     -sobjl obj_spec attr_name val <show object list>
#     -sstnode node_path,...        <show share tree node>
#     -sss                          <show scheduler status>

#     -sh                           <show administrative hosts>
DumpOptionToFile "-sh" "$DEST_DIR/admin_hosts"

#     -ss                           <show submit hosts>
DumpOptionToFile "-ss" "$DEST_DIR/submit_hosts"

#     -sm                           <show managers>
DumpOptionToFile "-sm" "$DEST_DIR/managers"

#     -so                           <show operators>
DumpOptionToFile "-so" "$DEST_DIR/operators"

#     -sc                           <show complexes>
DumpOptionToFile "-sc" "$DEST_DIR/centry"
#add display_win_gui if missing
cat  "$DEST_DIR/centry" | grep "^display_win_gui " > /dev/null 2>&1
if [ $? -ne 0 ]; then
   cp "$DEST_DIR/centry" "$DEST_DIR/centry.bak"
   sed '3i\
display_win_gui     dwg        BOOL        ==    YES         NO         0        0'  "$DEST_DIR/centry.bak" > "$DEST_DIR/centry" 
   rm -f "$DEST_DIR/centry.bak"
fi
#add m_thread if missing (new in 8.0.0)
cat  "$DEST_DIR/centry" | grep "^m_core " > /dev/null 2>&1
if [ $? -ne 0 ]; then
   cp "$DEST_DIR/centry" "$DEST_DIR/centry.bak"
   sed '3i\
m_thread            thread     INT         <=    YES         NO         0        0'  "$DEST_DIR/centry.bak" > "$DEST_DIR/centry" 
   rm -f "$DEST_DIR/centry.bak"
fi

#add m_core if missing (new in 62u5)
cat  "$DEST_DIR/centry" | grep "^m_core " > /dev/null 2>&1
if [ $? -ne 0 ]; then
   cp "$DEST_DIR/centry" "$DEST_DIR/centry.bak"
   sed '3i\
m_core              core       INT         <=    YES         NO         0        0'  "$DEST_DIR/centry.bak" > "$DEST_DIR/centry" 
   rm -f "$DEST_DIR/centry.bak"
fi
#add m_socket if missing (new in 62u5)
cat  "$DEST_DIR/centry" | grep "^m_socket " > /dev/null 2>&1
if [ $? -ne 0 ]; then
   cp "$DEST_DIR/centry" "$DEST_DIR/centry.bak"
   sed '3i\
m_socket            socket     INT         <=    YES         NO         0        0'  "$DEST_DIR/centry.bak" > "$DEST_DIR/centry" 
   rm -f "$DEST_DIR/centry.bak"
fi
#add m_topology if missing (new in 62u5)
cat  "$DEST_DIR/centry" | grep "^m_topology " > /dev/null 2>&1
if [ $? -ne 0 ]; then
   cp "$DEST_DIR/centry" "$DEST_DIR/centry.bak"
   sed '3i\
m_topology          topo       RESTRING    ==    YES         NO         NONE     0'  "$DEST_DIR/centry.bak" > "$DEST_DIR/centry" 
   rm -f "$DEST_DIR/centry.bak"
fi
#add m_topology_inuse if missing (new in 62u5)
cat  "$DEST_DIR/centry" | grep "^m_topology_inuse " > /dev/null 2>&1
if [ $? -ne 0 ]; then
   cp "$DEST_DIR/centry" "$DEST_DIR/centry.bak"
   sed '3i\
m_topology_inuse    utopo      RESTRING    ==    YES         NO         NONE     0'  "$DEST_DIR/centry.bak" > "$DEST_DIR/centry" 
   rm -f "$DEST_DIR/centry.bak"
fi

#     -sel                          <show execution hosts>
list=`$QCONF -sel 2>/dev/null`
#     -se hostname                  <show execution host>
DumpListToLocation "$list" $DEST_DIR/execution "-se"
DumpOptionToFile "-se global" $DEST_DIR/execution/global 

#     -scall                        <show calendar list>
list=`$QCONF -scall 2>/dev/null`
#     -scal calendar_name           <show calendar>
DumpListToLocation "$list" $DEST_DIR/calendars "-scal"

#     -sckptl                       <show ckpt. environment list>
list=`$QCONF -sckptl 2>/dev/null`
#     -sckpt ckpt_name              <show ckpt. environment>
DumpListToLocation "$list" $DEST_DIR/ckpt "-sckpt"

#     -ssconf                       <show scheduler configuration>
DumpOptionToFile "-ssconf" "$DEST_DIR/schedconf"

#     -shgrpl                       <show host group lists>
list=`$QCONF -shgrpl 2>/dev/null`
#     -shgrp group                  <show host group config.>
DumpListToLocation "$list" $DEST_DIR/hostgroups "-shgrp"

#     -suserl                       <show users>
list=`$QCONF -suserl 2>/dev/null`
#     -suser user,...               <show user>
DumpListToLocation "$list" $DEST_DIR/users "-suser"

#     -sprjl                        <show project list>
list=`$QCONF -sprjl 2>/dev/null`
#     -sprj project                 <show project>
DumpListToLocation "$list" $DEST_DIR/projects "-sprj"

#   [-sconfl]                       <show a list of all local configurations>
list=`$QCONF -sconfl 2>/dev/null`
#    -sconf [host,...|global]      <show configuration>
DumpListToLocation "$list" $DEST_DIR/configurations "-sconf"
DumpOptionToFile "-sconf global" "$DEST_DIR/configurations/global"

#     -spl                          <show PE-list>
list=`$QCONF -spl 2>/dev/null`
#     -sp pe_name                   <show PE configuration>
DumpListToLocation "$list" $DEST_DIR/pe "-sp"
# qsort_args could be added here, but that isn't actually necessary

#     -sul                          <show user ACL lists>
list=`$QCONF -sul 2>/dev/null`
#     -su acl_name                  <show user ACL>
DumpListToLocation "$list" $DEST_DIR/usersets "-su"
if [  ! -f $DEST_DIR/usersets/arusers ]; then   #add arusers ACL for 6.2+
   echo "name       arusers
type       ACL
oticket    0
fshare     0
entries    NONE" > $DEST_DIR/usersets/arusers
fi

#     -sql                          <show queue list>
list=`$QCONF -sql 2>/dev/null`
#     -sq wc_queue_list             <show queues>
DumpListToLocation "$list" $DEST_DIR/cqueues "-sq"

#     -srqsl                        <show RQS-list>
list=`$QCONF -srqsl 2>/dev/null`
#     -srqs [rqs_name_list]         <show RQS configuration>
DumpListToLocation "$list" $DEST_DIR/resource_quotas "-srqs"

#     -sstree                       <show share tree>
DumpOptionToFile "-sstree" "$DEST_DIR/sharetree"

#Make files readable for all
chmod -R g+r,o+r "$DEST_DIR"/*

$INFOTEXT "Configuration successfully saved to $DEST_DIR directory."
exit 0
