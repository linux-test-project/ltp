CFLAGS += -Wall -g -Wextra
CPPFLAGS += -I../../../../include
LDLIBS += -lm -L../../../../lib/ -lcontrollers -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

install:
	@set -e;
	ln -f cpuset_funcs.sh ../../../../bin/cpuset_funcs.sh;
	chmod +x cpuset_funcs.sh;
