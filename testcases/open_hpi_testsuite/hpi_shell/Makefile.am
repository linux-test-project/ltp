
.NOTPARALLEL:

MAINTAINERCLEANFILES 	= Makefile.in
MOSTLYCLEANFILES	= @TEST_CLEAN@

REMOTE_SOURCES		= openhpiclient.cpp openhpiclient.h

EXTRA_DIST		= hpi_cmd.sh

INCLUDES		= -I$(top_srcdir)/include -I$(top_srcdir)/utils
COMMONLIBS		= $(top_builddir)/hpi_ui_lib/libopenhpiui.la

if OH_DAEMON_ENABLED
INCLUDES 		+= -I$(top_srcdir)/openhpid \
			   -I$(top_srcdir)/marshal \
			   -DCLIENT
COMMONLIBS		+= $(top_builddir)/marshal/libohtcpconnx.la \
			   $(top_builddir)/utils/libopenhpiutils.la
else
COMMONLIBS		+= $(top_builddir)/src/libopenhpi.la \
			   @STATIC_PLUGIN_LIBS@ @STATIC_PLUGIN_EXTRA_LIBS@
endif

AM_CFLAGS 		= @CFLAGS@ -D_REENTRANT

bin_PROGRAMS		= hpi_shell
dist_bin_SCRIPTS	= hpi_cmd

HS_SOURCES		= \
	hpi_cmd.h \
	hpi_cmd.c \
	cmdparser.c \
	commands.c \
	ctrl_inv.c \
	inputcmd.c \
	sensor.c \
	session.c

hpi_shell_SOURCES = $(HS_SOURCES)

if OH_DAEMON_ENABLED
hpi_shell_SOURCES += $(REMOTE_SOURCES)
endif

hpi_shell_LDADD        = $(COMMONLIBS)
hpi_shell_LDFLAGS      =

hpi_cmd: hpi_cmd.sh
	cp -f $(srcdir)/hpi_cmd.sh hpi_cmd

$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		ln -s ../openhpid/$@; \
	fi

clean-local:
	rm -f *~ *.o $(REMOTE_SOURCES) hpi_cmd
