# Microsoft Developer Studio Project File - Name="SchedLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SchedLib - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "SchedLib.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "SchedLib.mak" CFG="SchedLib - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "SchedLib - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "SchedLib - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SchedLib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".\\" /I "..\..\qmon_nt" /I "..\qmonnt" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\cull\src" /I "..\..\commd" /I "..\..\history" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "WIN32NATIVE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "SchedLib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\qmonnt" /I "..\..\rmon\src" /I "..\..\cull\src" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\commd" /I "..\..\history" /I "..\..\3rdparty\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "WIN32NATIVE" /D "WIN32" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "SchedLib - Win32 Release"
# Name "SchedLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Sources Codine"

# PROP Default_Filter ""
# Begin Group "Sources Schedd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\schedd\cod_api_attributes.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_c_event.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_complex_schedd.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_fill_components.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_job_schedd.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_orders.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_pe_schedd.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_range_schedd.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_schedd_text.c
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_select_queue.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_static_load.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\debit.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\event.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\interactive_sched.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\load_correction.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\or_alloc.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\scale_usage.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\schedd_message.c
# End Source File
# Begin Source File

SOURCE=..\..\schedd\schedd_monitor.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\slots_used.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\sort_hosts.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\subordinate_schedd.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\suspend_thresholds.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# Begin Source File

SOURCE=..\..\schedd\valid_queue_user.c
# ADD CPP /I "..\..\qmonnt"
# SUBTRACT CPP /I "..\..\qmon_nt"
# End Source File
# End Group
# Begin Group "Source Api"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\apilib\cod_parse_num_par.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_parse_num_val.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\product_mode.c
# End Source File
# Begin Source File

SOURCE=..\..\apilib\resolve_host.c
# End Source File
# End Group
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Header Codine"

# PROP Default_Filter ""
# Begin Group "Header schedd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\cod_api_attributes.h
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_api_attributes.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_api_intern.h
# End Source File
# Begin Source File

SOURCE=..\cod_c_event.h
# End Source File
# Begin Source File

SOURCE=..\cod_complex_schedd.h
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_complex_schedd.h
# End Source File
# Begin Source File

SOURCE=..\cod_fill_components.h
# End Source File
# Begin Source File

SOURCE=..\cod_job_schedd.h
# End Source File
# Begin Source File

SOURCE=..\cod_orders.h
# End Source File
# Begin Source File

SOURCE=..\cod_parse_num_par.h
# End Source File
# Begin Source File

SOURCE=..\cod_parse_num_val.h
# End Source File
# Begin Source File

SOURCE=..\cod_pe_schedd.h
# End Source File
# Begin Source File

SOURCE=..\cod_range_schedd.h
# End Source File
# Begin Source File

SOURCE=..\..\schedd\cod_schedd_text.h
# End Source File
# Begin Source File

SOURCE=..\cod_select_queue.h
# End Source File
# Begin Source File

SOURCE=..\cod_static_load.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\commlib.h
# End Source File
# Begin Source File

SOURCE=..\debit.h
# End Source File
# Begin Source File

SOURCE=..\event.h
# End Source File
# Begin Source File

SOURCE=..\interactive_sched.h
# End Source File
# Begin Source File

SOURCE=..\load_correction.h
# End Source File
# Begin Source File

SOURCE=..\or_alloc.h
# End Source File
# Begin Source File

SOURCE=..\product_mode.h
# End Source File
# Begin Source File

SOURCE=..\resolve_host.h
# End Source File
# Begin Source File

SOURCE=..\scale_usage.h
# End Source File
# Begin Source File

SOURCE=..\sched.h
# End Source File
# Begin Source File

SOURCE=..\schedd_monitor.h
# End Source File
# Begin Source File

SOURCE=..\slots_used.h
# End Source File
# Begin Source File

SOURCE=..\sort_hosts.h
# End Source File
# Begin Source File

SOURCE=..\subordinate_schedd.h
# End Source File
# Begin Source File

SOURCE=..\suspend_thresholds.h
# End Source File
# Begin Source File

SOURCE=..\valid_queue_user.h
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
# Begin Group "Header utilib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\utilib\cod_me.h
# End Source File
# Begin Source File

SOURCE=..\..\utilib\cod_prognames.h
# End Source File
# End Group
# Begin Group "Header api"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\apilib\cod_answerL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_api.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_boundaries.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_complexL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_hostL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\cod_jobL.h
# End Source File
# Begin Source File

SOURCE=..\..\apilib\resolve_host.h
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

SOURCE=..\..\src\codrmon.h
# End Source File
# Begin Source File

SOURCE=..\..\src\proto.h
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

SOURCE=..\..\cull\src\cull_list.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\cull_listP.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_multitype.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_pack.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_sort.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_what.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\cull_where.h
# End Source File
# Begin Source File

SOURCE=..\..\cull\src\pack.h
# End Source File
# End Group
# Begin Group "Header commd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\commd\commd.h
# End Source File
# Begin Source File

SOURCE=..\..\commd\commd_message_flags.h
# End Source File
# End Group
# Begin Group "Header history"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\history\cod_strptime.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\..\apilib\cod_parse_num_val.h
# End Source File
# Begin Source File

SOURCE=..\QMonNt\win32nativetypes.h
# End Source File
# End Group
# End Target
# End Project
