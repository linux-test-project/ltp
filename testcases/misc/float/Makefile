CFLAGS := -Wall -O -g
LDFLAGS := -lpthread -lm

SRCS=$(wildcard float*.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

SUBDIRS=bessel exp_log iperb power trigo

all: $(TARGETS)
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install: all 
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done
	@for i in $(TARGETS); do ln -f $$i ../../bin/$$i; done

clean: 
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
	rm -f $(TARGETS) 
	@for i in $(TARGETS); do rm -rf ../../bin/$$i ; done

