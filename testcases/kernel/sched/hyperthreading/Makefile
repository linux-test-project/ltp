
OSV := $(shell uname -m|grep i[3-6]86)
ifeq (,$(OSV))
else
export CFLAGS+= -DARCH_i386
endif


SUBDIRS = ht_enabled ht_affinity ht_interrupt

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
