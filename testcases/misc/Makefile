SUBDIRS = crash disktest f00f float

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

