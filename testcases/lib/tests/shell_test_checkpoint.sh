#!/bin/sh

. tst_env.sh

tst_res TINFO "Waiting for a checkpoint 0"
tst_checkpoint wait 10000 0
tst_res TPASS "Continuing after checkpoint"
