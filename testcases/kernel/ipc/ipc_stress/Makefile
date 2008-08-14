CFLAGS += -O -g -D_LINUX_ -Wall 
LOADLIBES += -lpthread

SRCS=$(wildcard *.c)

TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done ;
	cp run_semaphore_test_01.sh ../../../bin/run_semaphore_test_01.sh
clean:
	rm -f $(TARGETS)
