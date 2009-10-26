#!/bin/sh
################################################################################
## Copyright (c) Oliver Hartkopp <oliver.hartkopp@volkswagen.de>, 2009        ##
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

if [ $(id -ru) -ne 0 ]; then
     echo You need to be root to execute these tests
     exit 1
fi

cat <<-EOF > /etc/modprobe.d/vcan
# protocol family PF_CAN
alias net-pf-29 can
# protocols in PF_CAN
alias can-proto-1 can-raw
alias can-proto-2 can-bcm
alias can-proto-3 can-tp16
alias can-proto-4 can-tp20
alias can-proto-5 can-mcnet
alias can-proto-6 can-isotp
EOF

modprobe -f can
modprobe -f can_raw
modprobe -f vcan
ip link add dev vcan0 type vcan
ifconfig vcan0 up

./tst-filter-server > output_ltp-can.txt &
./tst-filter-master | tee output_ltp-can-verify.txt


