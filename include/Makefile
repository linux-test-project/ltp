

PREFIX=/opt/ltp
HEADERS=compiler.h dataascii.h databin.h file_lock.h forker.h open_flags.h \
	pattern.h random_range.h rmobj.h search_path.h string_to_tokens.h \
	str_to_bytes.h test.h tlibio.h usctest.h write_log.h \
	linux_syscall_numbers.h libtestsuite.h config.h

.PHONY: all install
all:

install: $(HEADERS)
	@set -e; for i in $(HEADERS); do \
		install -D -m 644 $$i $(DESTDIR)/$(PREFIX)/include/ltp/$$i; \
	done

.PHONY: clean distclean maintainer-clean
clean: ac-clean
distclean: clean ac-distclean
maintainer-clean: distclean ac-maintainer-clean

.PHONY: ac-clean ac-distclean ac-maintainerclean
ac-clean:
ac-distclean:
	rm -f config.h
ac-maintainer-clean:
	rm -f config.h.in
