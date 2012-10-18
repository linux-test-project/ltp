# Quickie makefile to (sort of) automate various pounder tasks.

# Copyright (C) 2003-2006 IBM
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# Quickie makefile to do pounder stuff.
NAME=ltpounder
CFLAGS=-O3 -Wall -g
HELPERS=timed_loop infinite_loop run-helper fancy_timed_loop

install:;
	./Install

helpers: $(HELPERS)

fancy_timed_loop: fancy_timed_loop.c debug.o
	$(CC) $(CFLAGS) -o fancy_timed_loop fancy_timed_loop.c debug.o

timed_loop: timed_loop.c debug.o
	$(CC) $(CFLAGS) -o timed_loop timed_loop.c debug.o

infinite_loop: infinite_loop.c debug.o
	$(CC) $(CFLAGS) -o infinite_loop infinite_loop.c debug.o

run-helper: run.c debug.o proclist.o
	$(CC) $(CFLAGS) -o run-helper run.c debug.o proclist.o

srctar:;
	cd ../../../; make clean; tar -cvzf $(NAME)-`date '+%Y-%m-%d'`.src.tar.gz --exclude '*CVS*' $(NAME)

bintar:;
	echo "Making binary tarball for" `uname -m` "platform."
	./Install
	cd ../../../; tar -czf $(NAME)-`date '+%Y-%m-%d'`.`uname -m`.tar.gz $(NAME) --exclude='*CVS*'

helpers_clean:;
	rm -rf $(HELPERS) *.o

# Clean out all cruft from a pounder run
clean: helpers_clean
	./pounder -u
	if [ -d opt ]; then cd opt; for i in *; do if [ -d $$i ]; then cd $$i; make clean; cd -; fi; done; fi;
	cd src; for i in *; do if [ -d $$i ]; then cd $$i; make clean; cd -; fi; done;
	rm -rf tmp pounder.pid

# Put everything back as it was when this tarball was unpacked.
mrclean: helpers_clean;
	./pounder -u
	rm -rf opt log run-helper infinite_loop timed_loop debug.o tmp proclist.o
	find src/ -name Makefile | while read f; do cd `dirname $$f`; $(MAKE) clean; cd -; done;
	rm -rf `find tests/* 2>/dev/null | grep -v CVS`
	rm -rf pounder.pid

distclean: mrclean;


testtar:;
	tar -cvzf default-tests.tar.gz tests/D* tests/T*
