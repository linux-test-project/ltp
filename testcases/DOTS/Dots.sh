#!/bin/sh
java dots.framework.Dots -config mysql.ini -case $*
rm -fr clob*
rm -fr blob*

