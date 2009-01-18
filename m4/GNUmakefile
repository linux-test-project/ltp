

PREFIX=/opt/ltp
M4MACROS=$(notdir $(wildcard *.m4))

all:

install: $(M4MACROS)
	for i in $(M4MACROS); do \
		install -D -m 644 $$i $(DESTDIR)/$(PREFIX)/share/aclocal/$$i; \
	done

