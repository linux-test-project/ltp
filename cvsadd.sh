#!/bin/bash

set -x

find . -type d | egrep -v CVS | xargs -n 5000 cvs add 
find . -not -type d | egrep -v CVS | xargs -n 5000 cvs add
