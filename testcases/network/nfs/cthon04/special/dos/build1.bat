@echo off
rem @(#)build1.bat	1.1	98/10/26 Connectathon Testsuite
rem Cthon Test Suite NMAKE Build Script for Microsoft Visual C++ 1.5

rem This batch file is used to build ONE of the SPECIAL components of
rem the Connectathon testsuite. Please review the readme.txt for 
rem particular issues in building for the Dos/Windows environment.

DEL %1.OBJ
NMAKE %1.MAK > %1.LOG
TYPE %1.LOG
DIR %1.exe
