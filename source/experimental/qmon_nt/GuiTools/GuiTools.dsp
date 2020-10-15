# Microsoft Developer Studio Project File - Name="GuiTools" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GuiTools - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GuiTools.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GuiTools.mak" CFG="GuiTools - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GuiTools - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GuiTools - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GuiTools - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\qmonnt" /I ".\\" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\cull\src" /I "..\..\commd" /I "..\..\history" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /D "WIN32" /D "_DEBUG" /D "WIN32NATIVE" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /out:"../QMonNT/Release/GuiTools.dll"

!ELSEIF  "$(CFG)" == "GuiTools - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\qmonnt" /I ".\\" /I "..\..\rmon\src" /I "..\..\cull\src" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\commd" /I "..\..\history" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /D "WIN32NATIVE" /D "WIN32" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../QMonNT/Debug/GuiTools.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GuiTools - Win32 Release"
# Name "GuiTools - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CBoxEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ClickList.cpp
# End Source File
# Begin Source File

SOURCE=.\GuiTools.cpp
# End Source File
# Begin Source File

SOURCE=.\GuiTools.def
# End Source File
# Begin Source File

SOURCE=.\GuiTools.rc
# End Source File
# Begin Source File

SOURCE=.\InPlaceEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\InPlaceList.cpp
# End Source File
# Begin Source File

SOURCE=.\PopupEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\popuptimectrl.cpp
# End Source File
# Begin Source File

SOURCE=..\QMonNT\ScrollPropertyPage.cpp

!IF  "$(CFG)" == "GuiTools - Win32 Release"

# ADD CPP /GR

!ELSEIF  "$(CFG)" == "GuiTools - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\QMonNT\SizePropertySheet.cpp

!IF  "$(CFG)" == "GuiTools - Win32 Release"

# ADD CPP /GR

!ELSEIF  "$(CFG)" == "GuiTools - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\QMonNT\TreeCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=..\QMonNT\ViewDlgBar.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CBoxEdit.h
# End Source File
# Begin Source File

SOURCE=.\ClickList.h
# End Source File
# Begin Source File

SOURCE=.\InPlaceEdit.h
# End Source File
# Begin Source File

SOURCE=.\InPlaceList.h
# End Source File
# Begin Source File

SOURCE=.\PopupEdit.h
# End Source File
# Begin Source File

SOURCE=.\popuptimectrl.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=..\QMonNT\ScrollPropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\QMonNT\SizePropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\QMonNT\TreeCtrlEx.h
# End Source File
# Begin Source File

SOURCE=..\QMonNT\ViewDlgBar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\GuiTools.rc2
# End Source File
# End Group
# End Target
# End Project
# Section GuiTools : {72ADFD6C-2C39-11D0-9903-00A0C91BC942}
# 	1:23:CG_IDR_POPUP_COMBO_EDIT:102
# 	1:28:CG_IDR_POPUP_POPUP_TIME_CTRL:103
# End Section
