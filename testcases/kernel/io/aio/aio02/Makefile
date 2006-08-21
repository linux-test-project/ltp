# foo.
TEST_SRCS:=$(shell find cases/ -name \*.c | sort -n -t/ -k2)
PROGS:=$(patsubst %.c,%,$(TEST_SRCS))
HARNESS_SRCS:=main.c
# io_queue.c

CFLAGS=--static -Wall  -g -O 
#-lpthread -lrt
LIBAIO=-laio
all: $(PROGS)

$(PROGS): %: %.c $(HARNESS_SRCS)
	$(CC) $(CFLAGS) -DTEST_NAME=\"$<\" -o $@ main.c $(LIBAIO) 

clean:
	rm -f $(PROGS) *.o runtests.out rofile wofile rwfile

