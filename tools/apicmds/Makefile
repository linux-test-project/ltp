CFLAGS+=	-I../../include
LOADLIBES+=	-L../../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
CMDLIST=tst_brk tst_brkm tst_res tst_resm tst_exit tst_flush tst_brkloop tst_brkloopm

all: $(TARGETS)

install:
	@ln -fs $(TARGETS) ../testcases/bin/brk
	@ln -fs $(TARGETS) ../testcases/bin/brkm 
	@ln -f $(TARGETS) ../testcases/bin/res
	@ln -f $(TARGETS) ../testcases/bin/resm
	@ln -f $(TARGETS) ../testcases/bin/exit
	@ln -f $(TARGETS) ../testcases/bin/flush
	@ln -f $(TARGETS) ../testcases/bin/brkloop
	@ln -f $(TARGETS) ../testcases/bin/brkloopm

#install:
#	@for i in $(TARGETS); do ln -f $$i ../../testcases/bin/tst_brk ; done
#	@for i in $(TARGETS); do ln -f $$i ../../testcases/bin/tst_brkm ; done
#	@for i in $(TARGETS); do ln -f $$i ../../testcases/bin/tst_res ; done
#	@for i in $(TARGETS); do ln -f $$i ../../testcases/bin/tst_resm ; done
#	@for i in $(TARGETS); do ln -f $$i ../../testcases/bin/tst_exit ; done
#	@for i in $(TARGETS); do ln -f $$i ../../testcases/bin/tst_flush ; done
#	@for i in $(TARGETS); do ln -f $$i ../../testcases/bin/tst_tmpdir ; done

clean:
	rm -f $(TARGETS) $(CMDLIST)
