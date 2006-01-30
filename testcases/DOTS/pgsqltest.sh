#!/bin/bash

JAVA_BIN=/usr/local/jdk1.3.1/bin
testcase=$1

echo "Running Test Case $testcase "

$JAVA_BIN/java -Xms100m -Xmx500m -Xss12m dots.framework.Dots -config /dots/Dots/pgsql.ini -case $testcase
