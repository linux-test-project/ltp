rem @(#)build.bat	1.1	98/10/26 Connectathon Testsuite
@echo off

rem Cthon Test Suite NMAKE Build Script for Microsoft Visual C++ 1.5

rem The Cthon Test Suite can not be build reliably with Microsoft 
rem Visual C++ Workbench. This is due to Workbench rebuilding the
rem dependency files if the source is moved from one location to
rem another. When this occurs the Unix format pathnames for the
rem Unix include files are added into the makefile and when the
rem compiler runs it complains it can not find the files. So this
rem script was created to run NMAKE with the makefiles instead.
rem This requires that the PATH, LIB and INCLUDE environment
rem variables be set up correctly, or their location passed as 
rem parameter %1 to this script.

set LIB=%LIB%;%1\LIB
set INCLUDE=%INCLUDE%;%1\INCLUDE
set PATH=%PATH%;%1\BIN

NMAKE TEST1.MAK
NMAKE TEST2.MAK
NMAKE TEST3.MAK
NMAKE TEST4.MAK
NMAKE TEST5.MAK
NMAKE TEST6.MAK
NMAKE TEST7.MAK
NMAKE TEST8.MAK
NMAKE TEST9.MAK
NMAKE TEST4a.MAK
NMAKE TEST5a.MAK
NMAKE TEST5b.MAK
NMAKE TEST7a.MAK
NMAKE TEST7b.MAK



