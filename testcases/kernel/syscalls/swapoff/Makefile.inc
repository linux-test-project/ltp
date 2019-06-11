LIBDIR			+= ../swapon/
LIBSWAPON		:= $(LIBDIR)/libswapon.o
$(LIBSWAPON):
	$(MAKE) -C $(LIBDIR)
CPPFLAGS		+= -I$(abs_srcdir)/$(LIBDIR)
LDFLAGS			+= -L$(abs_builddir)/$(LIBDIR)
