#!/bin/sh -x

# This script will clean out a project of all files that were generated
# while building.  Automake generates all kinds of file, and sometimes
# it is nice to be able to see only the code from the repository.

make maintainer-clean

for i in `find . -name Makefile.in`; do
    rm $i
done

for i in `find . -name Makefile`; do
    rm $i
done

for i in `find . -name '*~'`; do
    rm $i
done

for i in `find . -name '.#*'`; do
    rm $i
done

rm -fR docs/hld/OpenHPI_HLD/
rm -fR docs/test_specification/test_specification
