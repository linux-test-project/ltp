target_rel_dir := $(if $(cwd_rel_from_top),$(cwd_rel_from_top)/,)

%.o: %.c
ifdef VERBOSE
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
else
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
	@echo CC $(target_rel_dir)$@
endif

ifdef VERBOSE
COMPILE.c=$(CC) $(CPPFLAGS) $(CFLAGS) -c
else
COMPILE.c=@echo CC $(target_rel_dir)$@; $(CC) $(CPPFLAGS) $(CFLAGS) -c
endif

%: %.o
ifdef VERBOSE
	$(CC) $(LDFLAGS) $^ $(LTPLDLIBS) $(LDLIBS) -o $@
else
	@$(CC) $(LDFLAGS) $^ $(LTPLDLIBS) $(LDLIBS) -o $@
	@echo LD $(target_rel_dir)$@
endif

$(HOST_MAKE_TARGETS): %: %.c
ifdef VERBOSE
	$(HOSTCC) $(HOST_CFLAGS) $(HOST_LDFLAGS) $< $(HOST_LDLIBS) -o $@
else
	@$(HOSTCC) $(HOST_CFLAGS) $(HOST_LDFLAGS) $< $(HOST_LDLIBS) -o $@
	@echo HOSTCC $(target_rel_dir)$@
endif

%: %.c
ifdef VERBOSE
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ $(LTPLDLIBS) $(LDLIBS) -o $@
else
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ $(LTPLDLIBS) $(LDLIBS) -o $@
	@echo CC $(target_rel_dir)$@
endif

.PHONY: $(CHECK_TARGETS)
$(CHECK_TARGETS): check-%: %.c
ifdef VERBOSE
	-$(CHECK_NOFLAGS) $<
	-$(CHECK) $(CHECK_FLAGS) $(CPPFLAGS) $(CFLAGS) $<
else
	@echo CHECK $(target_rel_dir)$<
	@-$(CHECK_NOFLAGS) $<
	@-$(CHECK) $(CHECK_FLAGS) $(CPPFLAGS) $(CFLAGS) $<
endif
