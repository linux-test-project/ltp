CFLAGS+= -I../../../../include -Wall -g 
LOADLIBES+= -lpthread -lm -L../../../../lib -lltp

SRCS=$(wildcard float*.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

SUBDIRS=bessel exp_log iperb power trigo

all: $(TARGETS)
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i; done

clean: 
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
	rm -f $(TARGETS) 
	@set -e; for i in $(TARGETS); do rm -rf ../../../bin/$$i ; done

