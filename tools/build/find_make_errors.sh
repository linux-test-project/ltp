#!/bin/sh

MAKE=${MAKE:=gmake}

$MAKE -k clean >/dev/null 2>&1
$MAKE -k all 2>&1 | grep ' error: ' > make-errors

cut -f 1 -d : < make-errors | sort -u | while read f; do
        find . -name $f -type f
    done > make-errors-files-only
