rem @(#)run.bat	1.1	98/10/26 Connectathon Testsuite
echo Cthon Special Testsuite > log
DUPREQ.EXE > DUPREQ.LOG
echo DUPREQ.LOG >> log
type DUPREQ.LOG >> log
EXCLTEST.EXE 5 > EXCLTEST.LOG
echo EXCLTEST.LOG >> log
type EXCLTEST.LOG >> log
FSTAT.EXE > FSTAT.LOG
echo FSTAT.LOG >> log
type FSTAT.LOG >> log
HOLEY.EXE > HOLEY.LOG
echo HOLEY.LOG >> log
type HOLEY.LOG >> log
NEGSEEK.EXE > NEGSEEK.LOG
echo NEGSEEK.LOG >> log
type NEGSEEK.LOG >> log
NFSIDEM.EXE > NFSIDEM.LOG
echo NFSIDEM.LOG >> log
type NFSIDEM.LOG >> log
NSTAT.EXE 5 > NSTAT.LOG
echo NSTAT.LOG >> log
type NSTAT.LOG >> log
.\RENAME.EXE 5 > RENAME.LOG
echo RENAME.LOG >> log
type RENAME.LOG >> log
REWIND.EXE > REWIND.LOG
echo REWIND.LOG >> log
type REWIND.LOG >> log
STAT.EXE JUNK1 > STAT.LOG
echo STAT.LOG >> log
type STAT.LOG >> log
STAT2.EXE JUNK2 5 5 > STAT2.LOG
echo STAT2.LOG >> log
type STAT2.LOG >> log
TOUCHN.EXE 5 > TOUCHN.LOG
echo TOUCHN.LOG >> log
type TOUCHN.LOG >> log
TRUNCATE.EXE > TRUNCATE.LOG
echo TRUNCATE.LOG >> log
type TRUNCATE.LOG >> log
OP_REN.EXE > OP_REN.LOG
echo OP_REN.LOG >> log
type OP_REN.LOG >> log
OP_UNLK.EXE > OP_UNLK.LOG
echo OP_UNLK.LOG >> log
type OP_UNLK.LOG >> log
OP_CHMOD.EXE > OP_CHMOD.LOG
echo OP_CHMOD.LOG >> log
type OP_CHMOD.LOG >> log

