CFLAGS += -O -g -D_LINUX -Wall

SRCS=$(wildcard *_tc*.c)
TARGETS=sched_driver sched_tc0 sched_tc1 sched_tc2 sched_tc3 sched_tc4 sched_tc5 sched_tc6 

all: $(TARGETS)
	./generate.sh

sched.o: sched.h sched.c

$(TARGETS): sched.o

install:
	@set -e; for i in $(TARGETS) sched.datafile ; do ln -f $$i ../../../bin/$$i ; done ;
	ln -f sched_stress.sh ../../../bin/sched_stress.sh
clean:
	rm -f $(TARGETS) sched.o sched.datafile
