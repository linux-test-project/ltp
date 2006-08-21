CFLAGS += -Wall

SRCS=$(wildcard *_tc*.c)
TARGETS=sched_driver sched_tc0 sched_tc1 sched_tc2 sched_tc3 sched_tc4 sched_tc5 sched_tc6 

all: $(TARGETS) sched_datafile

sched_datafile:
	../../../../tools/make-file.sh sched_datafile 1200010

sched.o: sched.h sched.c

$(TARGETS): sched.o

install:
	@set -e; for i in $(TARGETS) sched_datafile ; do ln -f $$i ../../../bin/$$i ; done ;
	ln -f sched_stress.sh ../../../bin/sched_stress.sh
clean:
	rm -f $(TARGETS) sched.o sched_datafile
