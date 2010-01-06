#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################

export TMPDIR=${TMPDIR:-/tmp}
export LTPTMP=${TMPDIR}/p9auth_ltp
export TST_TOTAL=3
export TCID="p9auth"

export TST_COUNT=1

rm -rf $LTPTMP
mkdir $LTPTMP
chmod 755 $LTPTMP

comms="$LTPTMP/childgo $LTPTMP/d/childready $LTPTMP/d/childfail $LTPTMP/d/childpass $LTPTMP/childexit"

cleanup() {
	rm -rf $LTPTMP/d $comms
	mkdir -p $LTPTMP/d
	chown -R ltp $LTPTMP/d
}

if [ "$(id -ru)" -ne 0 ]; then
	tst_resm TBROK "Must start p9auth tests as root"
	exit 1
fi

if ! ltpuid=$(id -u ltp); then
	tst_resm TCONF "Failed to find ltp userid"
	exit 1
fi

# TEST 1: ltp setuids to 0 but no valid hash
# launch an unprivileged helper
cleanup

su ltp p9unpriv.sh &
while [ ! -f $LTPTMP/d/childready ]; do :; done
touch $LTPTMP/childgo
until [ -f $LTPTMP/d/childfail -o -f $LTPTMP/d/childpass ]; do :; done
if [ -f $LTPTMP/d/childpass ]; then
	tst_resm TFAIL "child could setuid with bad hash"
	exit 1
fi
tst_resm TPASS "child couldn't setuid with bad hash"

# TEST 2: ltp setuids to 0 with valid hash

# create the hash.  randstr doesn't have to be int, but it's ok
cleanup
randstr=$RANDOM
txt="$ltpuid@0"
echo -n "$txt" > $LTPTMP/txtfile
openssl sha1 -hmac "$randstr" $LTPTMP/txtfile | awk '{ print $2 '} > $LTPTMP/hex
unhex < $LTPTMP/hex > /dev/caphash
# give the child its token
echo -n "$txt@$randstr" > $LTPTMP/d/txtfile
chown ltp $LTPTMP/d/txtfile

su ltp p9unpriv.sh &
while [ ! -f $LTPTMP/d/childready ]; do :; done
touch $LTPTMP/childgo
until [ -f $LTPTMP/d/childfail -o -f $LTPTMP/d/childpass ]; do :; done
if [ -f $LTPTMP/d/childfail ]; then
	tst_resm TFAIL "child couldn't setuid with good hash"
	exit 1
fi
tst_resm TPASS "child could setuid with good hash"

# TEST 3: 0 setuids to 0 with hash valid for ltp user
cleanup
randstr=$RANDOM
txt="0@0"
echo -n "$txt" > $LTPTMP/txtfile
openssl sha1 -hmac "$randstr" $LTPTMP/txtfile | awk '{ print $2 '} > $LTPTMP/hex
unhex < $LTPTMP/hex > /dev/caphash
# give the child its token
echo -n "$txt@$randstr" > $LTPTMP/d/txtfile
chown ltp $LTPTMP/d/txtfile

su ltp p9unpriv.sh &
while [ ! -f $LTPTMP/d/childready ]; do :; done
touch $LTPTMP/childgo
until [ -f $LTPTMP/d/childfail -o -f $LTPTMP/d/childpass ]; do :; done;
if [ -f $LTPTMP/d/childpass ]; then
	tst_resm TFAIL "child could setuid from wrong source uid"
	exit 1
fi
tst_resm TPASS "child couldn't setuid from wrong source uid"

touch $LTPTMP/childexit

exit 0
