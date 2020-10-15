# Microsoft Developer Studio Project File - Name="GridwareCodine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GridwareCodine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GridwareCodine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GridwareCodine.mak" CFG="GridwareCodine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GridwareCodine - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GridwareCodine - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GridwareCodine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "C:\W2000.Release"
# PROP Intermediate_Dir "C:\W2000.Release\GridwareCodine"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../GridwareKernel" /I "../StarterMethod" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D _WIN32_WINNT=0x0500 /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GridwareCodine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "C:\W2000.Debug"
# PROP Intermediate_Dir "C:\W2000.Debug\GridwareCodine"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../GridwareKernel" /I "../StarterMethod" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D _WIN32_WINNT=0x0500 /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "GridwareCodine - Win32 Release"
# Name "GridwareCodine - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\CJob.cpp
# End Source File
# Begin Source File

SOURCE=.\CLimit.cpp
# End Source File
# Begin Source File

SOURCE=.\CShepherd.cpp
# End Source File
# Begin Source File

SOURCE=.\CShepherdEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\CUsage.cpp
# End Source File
# Begin Source File

SOURCE=.\CWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CConfiguration.h
# End Source File
# Begin Source File

SOURCE=.\CJob.h
# End Source File
# Begin Source File

SOURCE=.\CLimit.h
# End Source File
# Begin Source File

SOURCE=.\CShepherd.h
# End Source File
# Begin Source File

SOURCE=.\CShepherdEnvironment.h
# End Source File
# Begin Source File

SOURCE=.\CUsage.h
# End Source File
# Begin Source File

SOURCE=.\CWorker.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# End Target
# End Project
