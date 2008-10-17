

PREFIX=/usr
MANPAGES=forker.3 get_attrib.3 parse_open_flags.3 parse_opts.3 \
	parse_ranges.3 pattern.3 random_range.3 random_range_seed.3 \
	rmobj.3 string_to_tokens.3 str_to_bytes.3 tst_res.3 \
	tst_set_error.3 tst_sig.3 tst_tmpdir.3 usctest.3 write_log.3

all:

install: $(MANPAGES)
	for i in $(MANPAGES); do \
		install -D -m 644 $$i $(DESTDIR)/$(PREFIX)/share/man/man3/$$i; \
	done

