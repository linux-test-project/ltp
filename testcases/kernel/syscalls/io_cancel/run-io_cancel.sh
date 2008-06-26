#!/bin/sh

if [ -f $LTPROOT/testcases/bin/io_cancel01 ]; then
         $LTPROOT/testcases/bin/io_cancel01
else
         echo "io_cancel01 0 CONF : System doesn't support execution of the test"

fi

