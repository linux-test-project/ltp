#!/bin/sh

syscall=modify_ldt

if [ -f $LTPROOT/testcases/bin/${syscall}01 ]; then
         $LTPROOT/testcases/bin/${syscall}01
else
         echo "${syscall}01 0 CONF : System doesn't support execution of the test"

fi

