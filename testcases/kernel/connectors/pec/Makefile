CFLAGS += -I../../../../include -Wall
LDLIBS += -L../../../../lib -lltp

SRCS:=$(wildcard *.c)
TARGETS:=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)
 
install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; chmod +x run_pec_test ; done ;
	ln -f run_pec_test ../../../bin/
	chmod +x run_pec_test
clean:
	rm -f $(TARGETS)
