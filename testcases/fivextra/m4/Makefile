#
# $Id: Makefile,v 1.1 2005/03/22 17:51:30 mridge Exp $
#

TESTDIR=m4test

all:	

install:	all
	@mkdir -p ../../bin/$(TESTDIR)
	@cp -f m4test.sh ../../bin 
	@cp -r $(TESTDIR) ../../bin/
uninstall:
	@rm -rf ../../bin/$(TESTDIR)
	@rm -f ../../bin/m4test.sh

clean distclean:

