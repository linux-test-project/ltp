@echo off
rem @(#)build1.bat	1.1	98/10/26 Connectathon Testsuite

echo Cthon Test Suite NMAKE Build Script for Microsoft Visual C++ 2.0

rem This batch file is used to build ONE of the SPECIAL components of
rem the Connectathon testsuite. Please review the readme.txt for 
rem particular issues in building for the Dos/Windows environment.

NMAKE %1.MAK > %1.LOG
TYPE %1.LOG
DIR %1.exe
