
CFLAGS += -I../../../../include -I../libclone -Wall
LDLIBS += -L../../../../lib -L../libclone ../libclone/libclone.a -lltp

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))
NOLTP_TARGETS = $(patsubst %.c,%_noltp,$(SRCS))

%_noltp : %.c
	$(CC) -g -DNO_LTP -o $@ $< ../libclone/libclone.a

all: $(TARGETS)

noltp:  $(NOLTP_TARGETS)

clean:
	rm -f $(TARGETS) *.o $(NOLTP_TARGETS)

install:
	@set -e; for i in $(TARGETS) runipcnstest.sh check_ipcns_enabled ; do ln -f $$i ../../../bin/$$i ; chmod +x runipcnstest.sh ; done

noltp_check: noltp
	./runtests_noltp.sh
