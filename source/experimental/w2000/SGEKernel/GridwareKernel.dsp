# Microsoft Developer Studio Project File - Name="GridwareKernel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GridwareKernel - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GridwareKernel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GridwareKernel.mak" CFG="GridwareKernel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GridwareKernel - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GridwareKernel - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
# PROP WCE_FormatVersion ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GridwareKernel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "C:\W2000.Release"
# PROP Intermediate_Dir "C:\W2000.Release\GridwareKernel"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D _WIN32_WINNT=0x0500 /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GridwareKernel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "C:\W2000.Debug"
# PROP Intermediate_Dir "C:\W2000.Debug\GridwareKernel"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D _WIN32_WINNT=0x0500 /FD /GZ /c
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

# Name "GridwareKernel - Win32 Release"
# Name "GridwareKernel - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\CError.cpp
# End Source File
# Begin Source File

SOURCE=.\CException.cpp
# End Source File
# Begin Source File

SOURCE=.\CHandle.cpp
# End Source File
# Begin Source File

SOURCE=.\CIoCompletionPort.cpp
# End Source File
# Begin Source File

SOURCE=.\CJobObject.cpp
# End Source File
# Begin Source File

SOURCE=.\CKernelObject.cpp
# End Source File
# Begin Source File

SOURCE=.\CMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\CProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\CRegistryKey.cpp
# End Source File
# Begin Source File

SOURCE=.\CSehTranslator.cpp
# End Source File
# Begin Source File

SOURCE=.\CThread.cpp
# End Source File
# Begin Source File

SOURCE=.\CTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrace.cpp
# End Source File
# Begin Source File

SOURCE=.\CUser.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CBasicTypes.h
# End Source File
# Begin Source File

SOURCE=.\CEnvironment.h
# End Source File
# Begin Source File

SOURCE=.\CError.h
# End Source File
# Begin Source File

SOURCE=.\CException.h
# End Source File
# Begin Source File

SOURCE=.\CHandle.h
# End Source File
# Begin Source File

SOURCE=.\CIoCompletionPort.h
# End Source File
# Begin Source File

SOURCE=.\CJobObject.h
# End Source File
# Begin Source File

SOURCE=.\CKernelObject.h
# End Source File
# Begin Source File

SOURCE=.\CMutex.h
# End Source File
# Begin Source File

SOURCE=.\CProcess.h
# End Source File
# Begin Source File

SOURCE=.\CRegistryKey.h
# End Source File
# Begin Source File

SOURCE=.\CSehTranslator.h
# End Source File
# Begin Source File

SOURCE=.\CThread.h
# End Source File
# Begin Source File

SOURCE=.\CTimer.h
# End Source File
# Begin Source File

SOURCE=.\CTrace.h
# End Source File
# Begin Source File

SOURCE=.\CUser.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# End Target
# End Project
