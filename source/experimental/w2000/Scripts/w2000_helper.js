var StdIn, StdOut, StdErr;


//   The Contents of this file are made available subject to the terms of
//   the Sun Industry Standards Source License Version 1.2
// 
//   Sun Microsystems Inc., March, 2001
// 
// 
//   Sun Industry Standards Source License Version 1.2
//   =================================================
//   The contents of this file are subject to the Sun Industry Standards
//   Source License Version 1.2 (the \"License\"); You may not use this file
//   except in compliance with the License. You may obtain a copy of the
//   License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
// 
//   Software provided under this License is provided on an \"AS IS\" basis,
//   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
//   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
//   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
//   See the License for the specific provisions governing your rights and
//   obligations concerning the Software.
// 
//   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
// 
//   Copyright: 2001 by Sun Microsystems, Inc.
// 
//   All Rights Reserved.
// 

// !!!
// It is absolutely neccessary to get references of the Std*-Streams
// If we would us WScript.StdIn directly then we could loose some Data from StdIn
StdOut = WScript.StdOut;
StdIn = WScript.StdIn;
StdErr = WScript.StdErr;

Shell = new ActiveXObject("WScript.Shell");


////////////////////////////////////////////////////////////////////////////////////////

ScriptName = "W2000_Helper.js"

Arch = "w2000";
GrdRoot = Shell.Environment.Item("GRD_ROOT");
GrdRootSpool = GrdRoot + "\\spool";
GrdRootBin = GrdRoot + "\\bin\\" + Arch;

ShepherdBinary = "Shepherd.exe";

////////////////////////////////////////////////////////////////////////////////////////

Error_Unknown = 1;
Error_NoCommand = 2;
Error_InvArgCount = 3;
Error_MkSpoolFailed = 4;
Error_DelSpoolFailed = 5;
Error_MkFileFromStdIn = 6;
Error_WriteFileToStdOut = 7;
Error_ChangeJobState = 8;

function GetException(ErrorId) {
   if (ErrorId == Error_Unknown) {
      return new(Error_Unknown, "Unknown error");
   } else if (ErrorId == Error_NoCommand) {
      return new Error(Error_NoCommand, "Unknown Command as first parameter");
   } else if (ErrorId == Error_InvArgCount) {
      return new Error(Error_InvArgCount , "Invalid number of Arguments");
   } else if (ErrorId == Error_MkSpoolFailed) {
      return new Error(Error_MkSpoolFailed , "Making of spool directory failed");
   } else if (ErrorId == Error_DelSpoolFailed) {
      return new Error(Error_DelSpoolFailed , "Deleting of spool directory failed");
   } else if (ErrorId == Error_MkFileFromStdIn) {
      return new Error(Error_MkFileFromStdIn, "Creating file from StdIn failed");
   } else if (ErrorId == Error_WriteFileToStdOut) {
      return new Error(Error_WriteFileToStdOut, "Writing file to StdOut failed");
   } else if (ErrorId == Error_ChangeJobState) {
      return new Error(Error_ChangeJobState, "Unable to change job state");
   } else {
      return new(Error_Unknown, "Unknown error");
   }
}

function MakeSpoolDirectory() {
   var Ret;
   var Jid;
   var Tid;
   var FileSystemObject;
   var SpoolDirectory;

   if (WScript.Arguments.Length != 3) {
      throw GetException(Error_InvArgCount)
   } else {
      Jid = WScript.Arguments.Item(1);
      Tid = WScript.Arguments.Item(2);
   }

   try {	
      FileSystemObject = new ActiveXObject("Scripting.FileSystemObject");
      SpoolDirectory = FileSystemObject.CreateFolder(GrdRootSpool + "\\" + Jid + "." + Tid);
   } catch (e) {
      var ex;

      ex = GetException(Error_MkSpoolFailed);
      ex.description = ex.description + ": " + e.description;
      throw ex;
   }
}

function RemoveSpoolDirectory() {
   var Ret;
   var Jid;
   var Tid;
   var FileSystemObject;

   if (WScript.Arguments.Length != 3) {
      throw GetException(Error_InvArgCount)
   } else {
      Jid = WScript.Arguments.Item(1);
      Tid = WScript.Arguments.Item(2);
   }

   try {	
      FileSystemObject = new ActiveXObject("Scripting.FileSystemObject");
      FileSystemObject.DeleteFolder(GrdRootSpool + "\\" + Jid + "." + Tid, true);
   } catch (e) {
      var ex;

      ex = GetException(Error_DelSpoolFailed);
      ex.description = ex.description + ": " + e.description;
      throw ex;
   }
}

function StdInToFile() {
   var Ret;
   var Jid;
   var Tid;
   var Filename;
   var FileSystemObject;
   var NewFile;

   if (WScript.Arguments.Length != 4) {
      throw GetException(Error_InvArgCount)
   } else {
      Jid = WScript.Arguments.Item(1);
      Tid = WScript.Arguments.Item(2);
      Filename = WScript.Arguments.Item(3); 
   }

   try {	
      FileSystemObject = new ActiveXObject("Scripting.FileSystemObject");
      NewFile = FileSystemObject.CreateTextFile(GrdRootSpool + "\\" + Jid + "." + Tid + "\\" + Filename, true, false);
      while (!StdIn.AtEndOfStream) {
         NewFile.WriteLine(StdIn.ReadLine());
      }
      NewFile.Close();
   } catch (e) {
      var ex;

      ex = GetException(Error_MkFileFromStdIn) + " " + Jid + " " + Tid + " " + Filename;
      ex.description = ex.description + ": " + e.description;
      throw ex;
   }
}

function FileToStdOut(no_error) {
   var ForReading = 1;
   var TristateFalse = 0;
   var Jid;
   var Tid;
   var Filename;
   var FileSystemObject;
   var File;

   if (WScript.Arguments.Length != 4) {
      throw GetException(Error_InvArgCount)
   } else {
      Jid = WScript.Arguments.Item(1);
      Tid = WScript.Arguments.Item(2);
      Filename = WScript.Arguments.Item(3); 
   }

   try {	
      FileSystemObject = new ActiveXObject("Scripting.FileSystemObject");
      File = FileSystemObject.OpenTextFile(GrdRootSpool + "\\" + Jid + "." + Tid + "\\" + Filename, ForReading, false, TristateFalse);
      while (!File.AtEndOfStream) {
         StdOut.WriteLine(File.ReadLine());
      }
      File.Close();
   } catch (e) {
      if (!no_error) {
         var ex;

         ex = GetException(Error_WriteFileToStdOut);
         ex.description = ex.description + ": " + e.description;
         throw ex;
      }
   }
}

function ChangeJobState(Command) {
   var Jid;
   var Tid;

   if (WScript.Arguments.Length != 3) {
      throw GetException(Error_InvArgCount)
   } else {
      Jid = WScript.Arguments.Item(1);
      Tid = WScript.Arguments.Item(2);
   }

   try {	
      var CommandLine;

      CommandLine = GrdRootBin + "\\" + ShepherdBinary + " " + Command + " " + GrdRootSpool + "\\" + Jid + "." + Tid;
      Shell.Run(CommandLine, 10, true);
   } catch (e) {
      var ex;

      ex = GetException(Error_ChangeJobState);
      ex.description = command + ": "+ ex.description + ": " + e.description;
      throw ex;
   }
}

function Main() {
   var Command;
   var ErrorId;
   var Arguments;

   if (WScript.Arguments.Length >= 1) {
      Command = WScript.Arguments(0);
   }

   try {
      if (Command == "MAKE_SPOOL_DIRECTORY") {
         MakeSpoolDirectory();
      } else if (Command == "REMOVE_SPOOL_DIRECTORY") {
         RemoveSpoolDirectory();
      } else if (Command == "STDIN_TO_FILE") {
         StdInToFile();
      } else if (Command == "FILE_TO_STDOUT") {
         FileToStdOut(false);
      } else if (Command == "FILE_TO_STDOUT_NO_ERROR") {
         FileToStdOut(true);
      } else if (Command == "START_JOB") {
         ChangeJobState("start");
      } else if (Command == "TERMINATE_JOB") {
         ChangeJobState("terminate");
      } else if (Command == "SUSPEND_JOB") {
         ChangeJobState("suspend");
      } else if (Command == "RESUME_JOB") {
         ChangeJobState("resume");
      } else {
         throw GetException(Error_NoCommand);
      }
   } catch (e) {
      StdErr.WriteLine("");
      StdErr.WriteLine("EXCEPTION in " + ScriptName);
      StdErr.WriteLine("Facility Code: " + (e.number>>16 & 0x1FFF));
      StdErr.WriteLine("Exception Code: " + (e.number & 0xFFFF));
      StdErr.WriteLine("Description: " + e.description);
   }
}


Main();




