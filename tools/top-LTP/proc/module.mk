# This file gets included into the main Makefile, in the top directory.

# for lib$(NAME).so and /usr/include/($NAME) and such
NAME      :=  proc

SHARED := 1

SONAME    :=  lib$(NAME).so.$(LIBVERSION)

LIBSRC :=  $(wildcard proc/*.c)
LIBHDR :=  $(wildcard proc/*.h)
LIBOBJ :=  $(LIBSRC:.c=.o)

#ALL        += proc/lib$(NAME).a
#INSTALL    += $(usr/lib)/lib$(NAME).a # plus $(usr/include)$(NAME) gunk

ifeq ($(SHARED),1)
ALL        += proc/$(SONAME)
INSTALL    += $(lib)/$(SONAME)
FPIC       := -fpic
LIBPROC    := proc/$(SONAME)
else
ALL        += proc/lib$(NAME).a
LIBPROC    := proc/lib$(NAME).a
endif

# Separate rule for this directory, to use -fpic or -fPIC
$(filter-out proc/version.o,$(LIBOBJ)): proc/%.o: proc/%.c
	$(CC) -c $(CFLAGS) $(FPIC) $< -o $@

LIB_X := COPYING module.mk
TARFILES += $(LIBSRC) $(LIBHDR) $(addprefix proc/,$(LIB_X))


# Clean away all output files, .depend, and symlinks.
# Use wildcards in case the version has changed.
CLEAN += proc/.depend proc/lib*.so* proc/lib*.a $(LIBOBJ)
DIRS  += proc/

proc/lib$(NAME).a: $(LIBOBJ)
	$(AR) rcs $@ $^

proc/$(SONAME): $(LIBOBJ)
	$(CC) -shared -Wl,-soname,$(SONAME) -o $@ $^ -lc
	cd proc && $(ln_sf) $(SONAME) lib$(NAME).so


# AUTOMATIC DEPENDENCY GENERATION -- GCC AND GNUMAKE DEPENDENT
proc/.depend: $(LIBSRC) $(LIBHDR)
	$(strip $(CC) $(LIB_CFLAGS) -MM -MG $(LIBSRC) > $@)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),tar)
-include proc/.depend
endif
endif


$(lib)/$(SONAME) : proc/$(SONAME)
	$(install) --mode a=rx --strip $< $@
	cd $(lib) && $(ln_sf) $(SONAME) lib$(NAME).so
	$(ldconfig)

#$(usr/lib)/lib$(NAME).a : proc/lib$(NAME).a
#	$(install) --mode a=r --strip $< $@

# Junk anyway... supposed to go in /usr/include/$(NAME)
#$(HDRFILES) ??? : $(addprefix proc/,$(HDRFILES)) ???
#	$(install) --mode a=r $< $@


proc/version.o:	proc/version.c proc/version.h
	$(CC) $(CFLAGS) $(FPIC) -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\" -c -o $@ $<
