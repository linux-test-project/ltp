
rem @(#)basic.bat	1.1	98/10/26 Connectathon Testsuite
@echo off
echo.
echo Starting CONNECTATHON BASIC tests
echo.

set TESTDIR=%2
set TESTARG1=%3
set TESTARG2=%4
set OLDPATH=%PATH%

echo Arg1 is the network_drive: to create the test dir on = %1%
rem  Note: Arg1 should also be the drive you start the macro (ie. L:)
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

set NFSTESTDIR=%TESTDIR%1                 
test1 %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \

test2 %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%3
rem test3 Only 1 Param
test3 %TESTARG1% 
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%4
test4 %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%5
test5 %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%6
rem "'test6 Param 1 must be <= Param 2'"
test6 %TESTARG1% %TESTARG1%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%7
test7 %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%       

set NFSTESTDIR=%TESTDIR%8
test8 %TESTARG1% %TESTARG2%
if errorlevel 1 echo *** Test Failed ***
%1
rem No cd .. or deltree /y needed

set NFSTESTDIR=%TESTDIR%9
rem test9 Only 1 Param
test9 %TESTARG1%
if errorlevel 1 echo *** Test Failed ***
%1
cd \
deltree /y %NFSTESTDIR%  

echo "Congratulations, you completed the basic tests!"

:Exit

PATH=%OLDPATH%

