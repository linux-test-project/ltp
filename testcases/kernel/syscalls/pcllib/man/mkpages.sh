#!/bin/sh

groff -t -e -mandoc -Tascii pcl.3 | col -bx > pcl.txt
groff -t -e -mandoc -Tps pcl.3 > pcl.ps
man2html < pcl.3 | sed 's/<BODY>/<BODY text="#0000FF" bgcolor="#FFFFFF">/' > pcl.html

