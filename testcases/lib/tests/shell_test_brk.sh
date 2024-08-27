#!/bin/sh

. tst_env.sh

tst_brk TCONF "This exits test and the next message should not be reached"
tst_res TFAIL "If you see this the test failed"
