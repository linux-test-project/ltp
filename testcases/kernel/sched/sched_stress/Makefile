CFLAGS := -O -g -D_LINUX
LDFLAGS := sched.o

SRCS=$(wildcard *_tc*.c)
TARGETS=sched_driver sched_tc0 sched_tc1 sched_tc2 sched_tc3 sched_tc4 sched_tc5 sched_tc6 

all: sched.o $(TARGETS)

sched.o: sched.h sched.c
	$(CC) $(CFLAGS) -c sched.c
clean:
	rm -f $(TARGETS) sched.o
