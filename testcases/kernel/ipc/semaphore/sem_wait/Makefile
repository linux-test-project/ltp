CFLAGS := -Wall -I ../../../../../include -I ../include
LDFLAGS := -L ../../../../../lib -lltp -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=sem_wait01

all: $(TARGETS)
	@chmod 755 $(TEST)

install:
	for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
