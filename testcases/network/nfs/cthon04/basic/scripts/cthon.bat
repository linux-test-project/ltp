rem @(#)cthon.bat	1.1	98/10/26 Connectathon Testsuite
@echo off

rem %1 path to executable
rem %2 name of test
rem %3 testdir name
rem %4 logfile name
rem %+ parameters


PATH=%1;%PATH%
set NFSTESTDIR=%3

@echo on
redir -r2 %2 %5 %6 %7 %8 %9 >> %4

