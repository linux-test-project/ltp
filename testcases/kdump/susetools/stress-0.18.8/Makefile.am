MAINTAINERCLEANFILES = install-sh mkinstalldirs missing Makefile.in \
                       aclocal.m4 configure COPYING Log.cvs ChangeLog
EXTRA_DIST = ChangeLog

SUBDIRS = . src doc test

ChangeLog: Log.cvs
	rcs2log -u "apw	Amos Waterland	apw@rossby.metr.ou.edu" -L $^ | \
		sed 's,/home/apw/cvsroot/stress/src/,,g' | fmt -w 78 > $@

Log.cvs:
	cvs log > $@
