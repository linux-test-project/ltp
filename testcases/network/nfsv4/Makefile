SUBDIRS := $(patsubst %/Makefile,%,$(wildcard */Makefile))

all clean install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done
