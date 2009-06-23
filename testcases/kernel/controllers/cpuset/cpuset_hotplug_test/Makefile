CFLAGS += -Wall -g -Wextra
LDLIBS := -lm

LIBSRCS=$(wildcard ../cpuset_lib/*.c)
LIBOBJECTS=$(patsubst %.c,%.o,$(LIBSRCS))

SRCS=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SRCS))
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

$(TARGETS): %: %.o $(LIBOBJECTS)

clean:
	rm -f $(TARGETS) $(OBJECTS) $(LIBOBJECTS)

install:
	@set -e; \
	for i in $(TARGETS); do ln -f $$i ../../../../bin/$$i; chmod +x $$i; done
	ln -f cpuset_hotplug_test.sh ../../../../bin/cpuset_hotplug_test.sh;
	chmod +x cpuset_hotplug_test.sh;
