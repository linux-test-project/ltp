SUBDIRS = apicmds genload netpipe-2.4

CFLAGS+= -Wall -I../include
LOADLIBES+= -L../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	rm -f $(TARGETS)
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
