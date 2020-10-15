# Microsoft Developer Studio Project File - Name="QMonNt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=QMonNt - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "qmonnt.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "qmonnt.mak" CFG="QMonNt - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "QMonNt - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "QMonNt - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QMonNt - Win32 Release"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\src" /I "..\cull\src" /I "..\commd" /I "..\utilib" /I "..\QMonNt" /I "..\rmon\src" /I "..\apilib" /I "..\..\schedd" /I ".\\" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\cull\src" /I "..\..\commd" /I "..\..\history" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32NATIVE" /D "WIN32" /D "_DEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\GuiTools\Release\GuiTools.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "QMonNt - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\src" /I "..\cull\src" /I "..\commd" /I "..\utilib" /I "..\QMonNt" /I "..\rmon\src" /I "..\apilib" /I "..\..\schedd" /I ".\\" /I "..\..\rmon\src" /I "..\..\cull\src" /I "..\..\src" /I "..\..\utilib" /I "..\..\rmon" /I "..\..\apilib" /I "..\..\cull" /I "..\..\commd" /I "..\..\history" /I "..\..\3rdparty\include" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32NATIVE" /D "WIN32" /FR /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\GuiTools\Debug\GuiTools.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /profile /nodefaultlib

!ENDIF 

# Begin Target

# Name "QMonNt - Win32 Release"
# Name "QMonNt - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Source Debugging"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugC.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DebugDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugFrmWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugView.cpp
# End Source File
# Begin Source File

SOURCE=.\SetDebugLevelDialog.cpp
# End Source File
# End Group
# Begin Group "Source Codineanbindung"

# PROP Default_Filter ""
# End Group
# Begin Group "Source Views"

# PROP Default_Filter ""
# End Group
# Begin Group "Source Queue Pages"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\AddCheckDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AdvancedPage.cpp
# End Source File
# Begin Source File

SOURCE=.\AnswerList.cpp
# End Source File
# Begin Source File

SOURCE=.\AnswerMessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CodComplexAtributeView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodHostView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodJobView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodMessageView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodNothingView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodObj.cpp
# End Source File
# Begin Source File

SOURCE=.\CodPropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=.\CodQueueRootView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodQueueView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodQueueViewCheckpointing.cpp
# End Source File
# Begin Source File

SOURCE=.\CodQueueViewList.cpp
# End Source File
# Begin Source File

SOURCE=.\CodSet.cpp
# End Source File
# Begin Source File

SOURCE=.\CodThreadInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\CodTreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\CodView.cpp
# End Source File
# Begin Source File

SOURCE=.\ComplexAtribute.cpp
# End Source File
# Begin Source File

SOURCE=.\ComplexAtributeSet.cpp
# End Source File
# Begin Source File

SOURCE=.\ComplexAtributeView.cpp
# End Source File
# Begin Source File

SOURCE=.\Complexes.cpp
# End Source File
# Begin Source File

SOURCE=.\DefaultSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\EditValuesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GeneralPage.cpp
# End Source File
# Begin Source File

SOURCE=.\Host.cpp
# End Source File
# Begin Source File

SOURCE=.\HostSet.cpp
# End Source File
# Begin Source File

SOURCE=.\Job.cpp
# End Source File
# Begin Source File

SOURCE=.\JobListView.cpp
# End Source File
# Begin Source File

SOURCE=.\JobSet.cpp
# End Source File
# Begin Source File

SOURCE=.\LogSaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\QMonNt.cpp
# End Source File
# Begin Source File

SOURCE=.\hlp\QMonNt.hpj
# End Source File
# Begin Source File

SOURCE=.\QMonNt.rc
# End Source File
# Begin Source File

SOURCE=.\QMonNtDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\QMonNtView.cpp
# End Source File
# Begin Source File

SOURCE=.\Queue.cpp
# End Source File
# Begin Source File

SOURCE=.\QueueComplex.cpp
# End Source File
# Begin Source File

SOURCE=.\QueueSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\apilib\setup.c

!IF  "$(CFG)" == "QMonNt - Win32 Release"

!ELSEIF  "$(CFG)" == "QMonNt - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SplitterFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Header Debugging"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Debug.h
# End Source File
# Begin Source File

SOURCE=.\DebugC.h
# End Source File
# Begin Source File

SOURCE=.\DebugDoc.h
# End Source File
# Begin Source File

SOURCE=.\DebugFrmWnd.h
# End Source File
# Begin Source File

SOURCE=.\DebugView.h
# End Source File
# Begin Source File

SOURCE=.\SetDebugLevelDialog.h
# End Source File
# End Group
# Begin Group "Header Codineanbindung"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CodThreadInfo.h
# End Source File
# End Group
# Begin Group "Header Views"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CodJobView.h
# End Source File
# Begin Source File

SOURCE=.\CodMessageView.h
# End Source File
# Begin Source File

SOURCE=.\CodTreeView.h
# End Source File
# Begin Source File

SOURCE=.\QMonNtView.h
# End Source File
# Begin Source File

SOURCE=.\SplitterFrame.h
# End Source File
# End Group
# Begin Group "Header Queue Pages"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CodPropertyPage.h
# End Source File
# Begin Source File

SOURCE=.\CodQueueRootView.h
# End Source File
# Begin Source File

SOURCE=.\CodQueueView.h
# End Source File
# Begin Source File

SOURCE=.\CodQueueViewCheckpointing.h
# End Source File
# Begin Source File

SOURCE=.\CodQueueViewList.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AddCheckDlg.h
# End Source File
# Begin Source File

SOURCE=.\AdvancedPage.h
# End Source File
# Begin Source File

SOURCE=.\AnswerList.h
# End Source File
# Begin Source File

SOURCE=.\AnswerMessageBox.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\CodComplexAtributeView.h
# End Source File
# Begin Source File

SOURCE=.\CodHostView.h
# End Source File
# Begin Source File

SOURCE=.\CodNothingView.h
# End Source File
# Begin Source File

SOURCE=.\CodObj.h
# End Source File
# Begin Source File

SOURCE=.\CodSet.h
# End Source File
# Begin Source File

SOURCE=.\CodView.h
# End Source File
# Begin Source File

SOURCE=.\ComplexAtribute.h
# End Source File
# Begin Source File

SOURCE=.\ComplexAtributeSet.h
# End Source File
# Begin Source File

SOURCE=.\ComplexAtributeView.h
# End Source File
# Begin Source File

SOURCE=.\Complexes.h
# End Source File
# Begin Source File

SOURCE=.\DefaultSettings.h
# End Source File
# Begin Source File

SOURCE=.\EditValuesDlg.h
# End Source File
# Begin Source File

SOURCE=.\GeneralPage.h
# End Source File
# Begin Source File

SOURCE=.\Host.h
# End Source File
# Begin Source File

SOURCE=.\HostSet.h
# End Source File
# Begin Source File

SOURCE=.\Job.h
# End Source File
# Begin Source File

SOURCE=.\JobListView.h
# End Source File
# Begin Source File

SOURCE=.\JobSet.h
# End Source File
# Begin Source File

SOURCE=.\LogSaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Messages.h
# End Source File
# Begin Source File

SOURCE=.\NodeInfo.h
# End Source File
# Begin Source File

SOURCE=.\QMonNt.h
# End Source File
# Begin Source File

SOURCE=.\QMonNtDoc.h
# End Source File
# Begin Source File

SOURCE=.\Queue.h
# End Source File
# Begin Source File

SOURCE=.\QueueComplex.h
# End Source File
# Begin Source File

SOURCE=.\QueueSet.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_qmon.ico
# End Source File
# Begin Source File

SOURCE=.\res\QMonNt.ico
# End Source File
# Begin Source File

SOURCE=.\res\QMonNt.rc2
# End Source File
# Begin Source File

SOURCE=.\res\QMonNtDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\queuesta.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\treeicon.bmp
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxPrint.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\MakeHelp.bat
# End Source File
# Begin Source File

SOURCE=.\hlp\QMonNt.cnt
# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# End Group
# End Target
# End Project
