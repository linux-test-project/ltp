
OSV := $(shell uname -m|grep i[3-6]86)
ifeq (,$(OSV))
else
export CFLAGS+= -DARCH_i386
endif


SUBDIRS = ht_enabled ht_affinity ht_interrupt

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
