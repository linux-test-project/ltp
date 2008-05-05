
.NOTPARALLEL:

MAINTAINERCLEANFILES 	= Makefile.in
MOSTLYCLEANFILES	= @TEST_CLEAN@

EXTRA_DIST		= hpi_cmd.sh

INCLUDES		= -I$(top_srcdir)/include -I$(top_srcdir)/utils
COMMONLIBS		= $(top_builddir)/hpi_ui_lib/libopenhpiui.la

if OH_DAEMON_ENABLED
INCLUDES 		+= -I$(top_srcdir)/openhpid \
			   -I$(top_srcdir)/marshal \
			   -DCLIENT
COMMONLIBS		+= $(top_builddir)/marshal/libohtcpconnx.la \
			   $(top_builddir)/utils/libopenhpiutils.la \
			   $(top_builddir)/openhpid/libopenhpi.la
else
COMMONLIBS		+= $(top_builddir)/src/libopenhpi.la \
			   @STATIC_PLUGIN_LIBS@ @STATIC_PLUGIN_EXTRA_LIBS@
endif

AM_CFLAGS 		= @CFLAGS@ -D_REENTRANT

bin_PROGRAMS		= hpi_shell
nodist_bin_SCRIPTS	= hpi_cmd

HS_SOURCES		= \
	hpi_cmd.h \
	hpi_cmd.c \
	cmdparser.c \
	commands.c \
	ctrl_inv.c \
	inputcmd.c \
	sensor.c \
	session.c \
	dimi.c \
	fumi.c

hpi_shell_SOURCES = $(HS_SOURCES)

hpi_shell_LDADD        = $(COMMONLIBS)
hpi_shell_LDFLAGS      =

hpi_cmd: hpi_cmd.sh
	cp -f $(srcdir)/hpi_cmd.sh hpi_cmd

clean-local:
	rm -f *~ *.o hpi_cmd
