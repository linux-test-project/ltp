CFLAGS+= -DPACKAGE=\"stress\" -DVERSION=\"0.17pre11\" -DSTDC_HEADERS=1 \
		  -DHAVE_UNISTD_H=1 -DHAVE_LIBM=1  -I. -Wall -D_FILE_OFFSET_BITS=64 \
		  -D_LARGEFILE_SOURCE 

LDLIBS+= -L../../lib -lltp -lm

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@ln -f $(TARGETS) ../../testcases/bin/genload

clean:
	rm -fr $(TARGETS)
