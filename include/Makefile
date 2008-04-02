

DESTDIR=/opt/ltp
HEADERS=usctest.h test.h compiler.h

all:

install: $(HEADERS)
	for i in $(HEADERS); do \
		install -D -m 644 $$i $(DESTDIR)/include/$$i; \
	done

