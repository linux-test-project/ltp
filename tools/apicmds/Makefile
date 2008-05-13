CFLAGS+=	-D_GNU_SOURCE -I../../include
LOADLIBES+=	-L../../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
CMDLIST=tst_brk tst_brkm tst_res tst_resm tst_exit tst_flush tst_brkloop tst_brkloopm tst_kvercmp

all: $(TARGETS)

install:
	@ln -f $(TARGETS) ../../testcases/bin/tst_brk
	@ln -f $(TARGETS) ../../testcases/bin/tst_brkm 
	@ln -f $(TARGETS) ../../testcases/bin/tst_res
	@ln -f $(TARGETS) ../../testcases/bin/tst_resm
	@ln -f $(TARGETS) ../../testcases/bin/tst_exit
	@ln -f $(TARGETS) ../../testcases/bin/tst_flush
	@ln -f $(TARGETS) ../../testcases/bin/tst_brkloop
	@ln -f $(TARGETS) ../../testcases/bin/tst_brkloopm
	@ln -f $(TARGETS) ../../testcases/bin/tst_kvercmp

clean:
	rm -f $(TARGETS)
