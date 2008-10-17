

PREFIX=/usr
MANPAGES=bump.1 doio.1 iogen.1 pan.1

all:

install: $(MANPAGES)
	for i in $(MANPAGES); do \
		install -D -m 644 $$i $(DESTDIR)/$(PREFIX)/share/man/man1/$$i; \
	done

