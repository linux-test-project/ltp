#!/bin/sh

MAKE=${MAKE:=gmake}

$MAKE -k clean
$MAKE -k all >make-errors 2>&1

grep ' error: ' make-errors | cut -f 1 -d : | sort -u | \
    while read f; do
        find . -name $f -type f
    done > make-errors-files-only
