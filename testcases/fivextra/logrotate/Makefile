TARGETS=`find . -maxdepth 1 -perm +111 -type f`

all: $(CTARGETS)

install:
	@for i in $(TARGETS) ; do ln -f $$i ../../bin/$$i || break ; done ;

uninstall:
	@for j in $(TARGETS) ; do rm -f ../../bin/$$j ; done ;

clean:
	rm -f *~
