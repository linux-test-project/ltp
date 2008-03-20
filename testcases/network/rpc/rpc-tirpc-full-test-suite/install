#!/bin/sh

# Installation tool for RPC & TIRPC Test Suite
# By C. LACABANNE - cyril.lacabanne@bull.net
# creation : 2007-06-06 revision : 2007-07-06


SERVERTSTPACKDIR=/clnfsv4/ltp-full-20080131/testcases/network/rpc/rpc-tirpc-full-test-suite/tests_pack
CLIENTTSTPACKDIR=/clnfsv4/ltp-full-20080131/testcases/network/rpc/rpc-tirpc-full-test-suite/tests_pack
LOCALIP=localhost
SERVERIP=localhost
CLIENTIP=localhost
SERVERUSER=root
CLIENTUSER=root
TSTPACKDIR=/clnfsv4/ltp-full-20080131/testcases/network/rpc/rpc-tirpc-full-test-suite/tests_pack

# ***************************************
# *** Test Suite deployment & install ***
# ***************************************

# clean before deploying
make clean

# deploying locally
cc -I/usr/include/tirpc -x c $TSTPACKDIR/cleaner.c.src -ltirpc -lpthread -o $TSTPACKDIR/cleaner.bin

# deployment on client if needed
if [ $LOCALIP != $CLIENTIP ]
then
	scp -r $TSTPACKDIR $CLIENTUSER@$CLIENTIP:$CLIENTTSTPACKDIR
	scp Makefile.clnt $CLIENTUSER@$CLIENTIP:$CLIENTTSTPACKDIR/Makefile
	ssh $CLIENTUSER@$CLIENTIP "make -f $CLIENTTSTPACKDIR/Makefile all"
	ssh $CLIENTUSER@$CLIENTIP "cc -I/usr/include/tirpc -x c $CLIENTTSTPACKDIR/cleaner.c.src -ltirpc -lpthread -o $CLIENTTSTPACKDIR/cleaner.bin"
fi

# deployment on server if needed
if [ $LOCALIP != $SERVERIP ]
then
	scp -r $TSTPACKDIR $SERVERUSER@$SERVERIP:$SERVERTSTPACKDIR
	scp Makefile.svc $SERVERUSER@$SERVERIP:$SERVERTSTPACKDIR/Makefile
	ssh $SERVERUSER@$SERVERIP "make -f $SERVERTSTPACKDIR/Makefile all"
	ssh $SERVERUSER@$SERVERIP "cc -I/usr/include/tirpc -x c $SERVERTSTPACKDIR/cleaner.c.src -ltirpc -lpthread -o $SERVERTSTPACKDIR/cleaner.bin"
fi

exit 0
