#!/bin/bash

pthserv &
pthcli 127.0.0.1
killall pthserv

