# Microsoft Developer Studio Project File - Name="CodApi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CodApi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CodApi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CodApi.mak" CFG="CodApi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CodApi - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CodApi - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CodApi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\src" /I "..\cull\src" /I "..\commd" /I "..\utilib" /I "..\QMonNT" /I "..\rmon\src" /I "..\apilib" /I "..\QMonNt" /I "..\qmonnt" /I ".\\" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\cull\src" /I "..\..\commd" /I "..\..\history" /D "NDEBUG" /D "_WINDOWS" /D "WIN32NATIVE" /D "WIN32" /D "_DEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CodApi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /Gy /I "..\QMonNt" /I "..\qmonnt" /I ".\\" /I "..\..\rmon\src" /I "..\..\cull\src" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\commd" /I "..\..\history" /I "..\..\3rdparty\include" /D "_DEBUG" /D "_WINDOWS" /D "WIN32NATIVE" /D "WIN32" /D "NOCOMMCOMPRESS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "CodApi - Win32 Release"
# Name "CodApi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Source Codine"

# PROP Default_Filter ""
# Begin Group "Source Commd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\commd\commd_io.c

!IF  "$(CFG)" == "CodApi - Win32 Release"

!ELSEIF  "$(CFG)" == "CodApi - Win32 Debug"

# ADD CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\commd\commlib.c
# End Source File
# Begin Source File

SOURCE=..\..\commd\commlib_last_heard.c
# End Source File
# Begin Source File

SOURCE=..\..\commd\commlib_util.c
# End Source File
# End Group
# Begin Group "Source Cull"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\cull\src\cull_db.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_lerrno.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_list.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_multitype.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_pack.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_parse.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_sort.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_what.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_where.c
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\pack.c
# End Source File
# End Group
# Begin Group "Source Utilib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\utilib\cod_arch.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_crc.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_exit.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_getme.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_log.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_prognames.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_string.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_time.c
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_wildmat.c
# End Source File
# End Group
# Begin Group "Source Apilib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\apilib\api_setup.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\api_shutdown.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_all_listsL.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_any_request.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_api_request.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\qm_name.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\setup.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\setup_path.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\utility.c
# End Source File
# End Group
# Begin Group "Source src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\shutdown.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\QMonNT\win32nativetypes.h
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Group "Header Codine"

# PROP Default_Filter ""
# Begin Group "Header apilib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\apilib\cod_all_listsL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_answerL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_api.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_api_intern.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_boundaries.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_calendarL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_ckptL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_complexL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_confL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_cw3L.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_deadlineL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_eventL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_histdirL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_hostL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_identL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_job_queueL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_job_refL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_job_reportL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_jobL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_krbL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_list_typeL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_load_reportL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_manopL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_multiL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_orderL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_paL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_path.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_peL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_ptfL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_qexecL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_qsiL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_queueL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_rangeL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_reportL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_requestL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_schedconfL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_share_tree_nodeL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_stringL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_text.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_time_eventL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_usageL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_userprjL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_usersetL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\parse_qsubL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\qm_name.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\usage.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\utility.h
# End Source File
# End Group
# Begin Group "Header commd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\commd\commd.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\commd_io.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\commd_message_flags.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\commlib.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\commlib_util.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\commproc.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\host.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\message.h
# End Source File
# End Group
# Begin Group "Header cull"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\cull\cull.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_db.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_dump_scan.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_lerrno.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_lerrnoP.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_list.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\cull_listP.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_multitype.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_multitypeP.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_pack.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_parse.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_sort.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_sortP.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_what.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_whatP.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_where.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_whereP.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\pack.h
# End Source File
# End Group
# Begin Group "Header rmon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\rmon\src\rmon.h
# End Source File
# Begin Source File

SOURCE=..\..\rmon\src\rmon_monitoring_level.h
# End Source File
# End Group
# Begin Group "Header src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\basis_types.h
# End Source File
# Begin Source File

SOURCE=..\..\src\cod.h
# End Source File
# Begin Source File

SOURCE=..\..\src\cod_c_api.h
# End Source File
# Begin Source File

SOURCE=..\..\src\codrmon.h
# End Source File
# Begin Source File

SOURCE=..\..\src\def.h
# End Source File
# Begin Source File

SOURCE=..\..\src\proto.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shutdown.h
# End Source File
# Begin Source File

SOURCE=..\..\src\sig_handlers.h
# End Source File
# Begin Source File

SOURCE=..\..\src\symbols.h
# End Source File
# End Group
# Begin Group "Header utilib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\utilib\cod_arch.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_crc.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_exit.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_getme.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_log.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_max_nis_retries.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_me.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_prognames.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_string.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_time.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_uidgid.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_wildmat.h
# End Source File
# End Group
# End Group
# End Group
# Begin Source File

SOURCE=.\DebugC.h
# End Source File
# End Target
# End Project
