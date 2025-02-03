# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2020-2022

target_rel_dir := $(if $(cwd_rel_from_top),$(cwd_rel_from_top)/,)

%.o: %.S
ifdef VERBOSE
	$(AS) $(ASFLAGS) -c -o $@ $<
else
	@$(AS) $(ASFLAGS) -c -o $@ $<
	@echo AS $(target_rel_dir)$@
endif

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

.PHONY: $(CHECK_HEADER_TARGETS)
$(CHECK_HEADER_TARGETS): check-%.h: %.h
ifdef VERBOSE
	-$(CHECK_NOFLAGS) $<
else
	@echo CHECK $(target_rel_dir)$<
	@-$(CHECK_NOFLAGS) $<
endif

.PHONY: $(SHELL_CHECK_TARGETS)
$(SHELL_CHECK_TARGETS): check-%.sh: %.sh
ifdef VERBOSE
	-$(SHELL_CHECK) $<
else
	@echo CHECK $(target_rel_dir)$<
	@-$(SHELL_CHECK) $<
endif
