###########################################################################
# name of file  : Makefile                                                #
# description   : make(1) description file for susefirewall2_test.sh      #
###########################################################################

TARGETS=susefirewall2_test.sh susefirewall2.conf

all: $(CTARGETS)

install:
	@for i in $(TARGETS) ; do ln -f $$i ../../bin/ || break ; done ;

uninstall:
	@for j in $(TARGETS) ; do rm -f ../../bin/$$j ; done ;

clean:
	@rm -f $(CTARGETS)
