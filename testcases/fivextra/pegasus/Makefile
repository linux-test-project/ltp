###########################################################################
# name of file  : Makefile                                                #
# description   : make(1) description file for pegasus                    #
###########################################################################


TARGETS=pegasus_test.sh file1.dat file2.dat file3.dat x.xml

all:

install: all
	for i in $(TARGETS) ; do ln -f $$i ../../bin || break ; done

uninstall:
	for j in $(TARGETS) ; do rm -f ../../bin/$$j ; done

clean:
	@rm -f *~
