SUBDIRS = apicmds genload netpipe-2.4 netpipe-2.4-ipv6 #top-LTP

CFLAGS+= -Wall -I../include
LOADLIBES+= -L../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done
	@set -e; ln -f gethost ../testcases/bin

clean:
	rm -f $(TARGETS)
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
