
.NOTPARALLEL:

MAINTAINERCLEANFILES 	= Makefile.in
MOSTLYCLEANFILES	= @TEST_CLEAN@

EXTRA_DIST		= hpi_ui.h

INCLUDES		= -I$(top_srcdir)/include -I$(top_srcdir)/utils

INCLUDES 		+= -I$(top_srcdir)/openhpid \
			   -I$(top_srcdir)/marshal \
			   -DCLIENT
COMMONLIBS		= $(top_builddir)/marshal/libohtcpconnx.la \
			   $(top_builddir)/utils/libopenhpiutils.la \
			   $(top_builddir)/openhpid/libopenhpi.la

bin_PROGRAMS		= hpi_shell

HS_SOURCES		= \
	hpi_cmd.h \
	hpi_cmd.c \
	cmdparser.c \
	commands.c \
	ctrl_inv.c \
	inputcmd.c \
	sensor.c \
	session.c \
	show.c \
	service.c \
	dimi.c \
	fumi.c

hpi_shell_SOURCES = $(HS_SOURCES)

hpi_shell_LDADD   = $(COMMONLIBS)

clean-local:
	rm -f *~ *.o
