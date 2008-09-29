CFLAGS+= -DPACKAGE=\"stress\" -DVERSION=\"0.17pre11\" 

LDLIBS+= -lm

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@ln -f $(TARGETS) ../../testcases/bin/genload

clean:
	rm -fr $(TARGETS)
