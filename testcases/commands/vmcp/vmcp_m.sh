#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# vmcp tool and module test
#
# The tool allows Linux users to send commands to the z/VM control program (CP).
# The normal usage is to invoke vmcp with the command you want to execute.
#
# The test case contains one shell script:
#
# basically executes the vmcp tool with different parameters and verifies that
# output and exitcodes are as expected

TST_CNT=2
TST_TESTFUNC=vmcp_main
TST_NEEDS_CMDS="vmcp"

vmcp_run()
{

        $2
        if [ $? -eq $1 ]; then
            tst_res TPASS "'$2' returned '$1'"
        else
            tst_res TFAIL "'$2' did not return '$1'"
        fi
}

vmcp_main1()
{
        tst_res TINFO "Verify basic VMCP commands"
        vmcp_run 0 "vmcp --version";
        vmcp_run 0 "vmcp --help";
        vmcp_run 0 "vmcp -v";
        vmcp_run 0 "vmcp -h";
        vmcp_run 0 "vmcp q dasd";
}

vmcp_main2()
{
        tst_res TINFO "Verify error conditions"
        vmcp_run 4 "vmcp -L"
        vmcp_run 4 "vmcp -m q dasd"
        vmcp_run 1 "vmcp dasddasddasd"
}

. tst_test.sh
tst_run
