#! /bin/sh

###########################################################################
##                                                                       ##
## Copyright (c) 2015, Red Hat Inc.                                      ##
##                                                                       ##
## This program is free software: you can redistribute it and/or modify  ##
## it under the terms of the GNU General Public License as published by  ##
## the Free Software Foundation, either version 3 of the License, or     ##
## (at your option) any later version.                                   ##
##                                                                       ##
## This program is distributed in the hope that it will be useful,       ##
## but WITHOUT ANY WARRANTY; without even the implied warranty of        ##
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          ##
## GNU General Public License for more details.                          ##
##                                                                       ##
## You should have received a copy of the GNU General Public License     ##
## along with this program. If not, see <http://www.gnu.org/licenses/>.  ##
##                                                                       ##
## Author: Li Wang <liwang@redhat.com>                                   ##
##                                                                       ##
###########################################################################
##                                                                       ##
## Summary: check signal:signal_generate gives 2 more fields: grp res    ##
##                                                                       ##
## This testcase is writing for signal events change:                    ##
##       6c303d3 tracing: let trace_signal_generate() report more info...##
##       163566f tracing: send_sigqueue() needs trace_signal_generate()  ##
##                                                                       ##
###########################################################################

export TCID="ftrace_regression02"
export TST_TOTAL=1

. ftrace_lib.sh

ftrace_signal_test()
{
	# Set envent
	echo 'signal:signal_generate' > $TRACING_PATH/set_event
	echo 1 > $TRACING_PATH/tracing_on
	echo > $TRACING_PATH/trace

	# just to generate trace
	for i in $(seq 100); do
		ls -l /proc > /dev/null 2>&1
	done

	grep -q 'grp=[0-9] res=[0-9]' $TRACING_PATH/trace
	if [ $? -eq 0 ]; then
		tst_resm TPASS "finished running the test."
	else
		tst_resm TFAIL "running the test failed, please check log message."
	fi
}

if tst_kvcmp -lt "3.2"; then
	tst_brkm TCONF "The test should be run in kernels >= 3.2.0 Skip the test..."
fi

ftrace_signal_test

tst_exit
