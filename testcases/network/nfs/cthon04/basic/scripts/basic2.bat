
rem @(#)basic2.bat	1.1	98/10/26 Connectathon Testsuite
@echo off
echo.
echo Starting CONNECTATHON BASIC tests 4a, 5a, 5b, 7a & 7b
echo.

set TESTDIR=%2
set TESTARG1=%3
set TESTARG2=%4
set OLDPATH=%PATH%

echo Arg1 is the network_drive: to create the test dir on = %1%
rem  Note: Arg1 should be the drive you start the macro from also
echo Arg2 is the basename of the directory to create = %TESTDIR%
echo Arg3 is the value of test arg1 = %TESTARG1%
echo Arg4 is the value of test arg2 = %TESTARG2%
echo Arg5 is the full path to the directory containing the tests = %5%
echo.

choice /t:C,10 /n /c:AC (A-bort or C-ontinue):
if errorlevel 2 goto Continue
if errorlevel 1 goto Exit 
:Continue

PATH=%5;%PATH%

%1
cd \

deltree /y %TESTDIR%*

set NFSTESTDIR=%TESTDIR%4
test4a %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%5
test5a %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \

set NFSTESTDIR=%TESTDIR%5
test5b %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%7
test7a %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%7
test7b %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

echo "Congratulations, you completed the basic tests!"

:Exit

PATH=%OLDPATH%

