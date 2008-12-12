#!/bin/sh

pthserv &
pthcli 127.0.0.1
clientCode=$?
killall pthserv
serverCode=$?
if [ $clientCode -ne 0 ] || [ $serverCode -ne 0 ]; then
    exit 1
fi
exit 0
