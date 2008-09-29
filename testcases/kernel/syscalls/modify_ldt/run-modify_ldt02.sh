#!/bin/sh

syscall=modify_ldt

if [ -f $LTPROOT/testcases/bin/${syscall}02 ]; then
         $LTPROOT/testcases/bin/${syscall}02
else
         echo "${syscall}02 0 CONF : System doesn't support execution of the test"

fi

