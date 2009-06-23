

PREFIX=/usr
MANPAGES=ltp-bump.1 doio.1 iogen.1 ltp-pan.1

all:

install: $(MANPAGES)
	@set -e; for i in $(MANPAGES); do \
		install -D -m 644 $$i $(DESTDIR)/$(PREFIX)/share/man/man1/$$i; \
	done

