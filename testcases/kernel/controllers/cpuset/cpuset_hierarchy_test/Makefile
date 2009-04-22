SRCS = $(wildcard *.sh)

all:
	@true

clean:
	@true

install:
	@set -e;
	for i in $(SRCS); do ln -f $$i ../../../../bin/$$i; chmod +x $$i; done

