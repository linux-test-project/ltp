SUBDIRS = `ls -d * | grep -v Makefile`

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	mkdir -p bin
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	rm -rf bin/*
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

