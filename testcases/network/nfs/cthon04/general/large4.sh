:
#! /bin/sh
#
#       @(#)large4.sh	1.2 97/01/03 Connectathon Testsuite
#	1.4 Lachman ONC Test Suite source
#

$CC $CFLAGS -o large large.c&
$CC $CFLAGS -o large1 large1.c&
$CC $CFLAGS -o large2 large2.c&
$CC $CFLAGS -o large3 large3.c&
wait
rm large large1 large2 large3 
