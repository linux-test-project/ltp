MAINTAINERCLEANFILES = Makefile.in
MOSTLYCLEANFILES        = @TEST_CLEAN@

INCLUDES                = @OPENHPI_INCLUDES@

COMMONLIBS = @STATIC_PLUGIN_LIBS@ @STATIC_PLUGIN_EXTRA_LIBS@

if OH_DAEMON_ENABLED
COMMONLIBS		+= $(top_builddir)/openhpid/libopenhpi.la
else
COMMONLIBS              += $(top_builddir)/src/libopenhpi.la
endif

AM_CFLAGS               = @CFLAGS@

bin_PROGRAMS = \
	hpievents \
	hpifan \
        hpiinv \
	hpiel \
	hpisensor \
        hpipower \
	hpitree \
	hpialarms \
	hpireset \
	hpiwdt \
	hpithres \
	hpitop   \
	hpisettime \
	hpionIBMblade

include_HEADERS         =

hpialarms_SOURCES   = hpialarms.c
hpialarms_LDADD     = $(COMMONLIBS)
hpialarms_LDFLAGS   =

hpithres_SOURCES        = hpithres.c
hpithres_LDADD          = $(COMMONLIBS)
hpithres_LDFLAGS        =

hpifan_SOURCES          = hpifan.c
hpifan_LDADD            = $(COMMONLIBS)
hpifan_LDFLAGS          =

hpitop_SOURCES          = hpitop.c
hpitop_LDADD            = $(COMMONLIBS)
hpitop_LDFLAGS          =

hpireset_SOURCES        = hpireset.c
hpireset_LDADD          = $(COMMONLIBS)
hpireset_LDFLAGS        =

hpiel_SOURCES           = hpiel.c
hpiel_LDADD             = $(COMMONLIBS)
hpiel_LDFLAGS           =

hpisensor_SOURCES       = hpisensor.c
hpisensor_LDADD         = $(COMMONLIBS)
hpisensor_LDFLAGS       =

hpiwdt_SOURCES          = hpiwdt.c
hpiwdt_LDADD            = $(COMMONLIBS)
hpiwdt_LDFLAGS          =

hpiinv_SOURCES          = hpiinv.c
hpiinv_LDADD            = $(COMMONLIBS)
hpiinv_LDFLAGS          =

hpisettime_SOURCES      = hpisettime.c
hpisettime_LDADD        = $(COMMONLIBS)
hpisettime_LDFLAGS      =

hpievents_SOURCES      = hpievents.c
hpievents_LDADD        = $(COMMONLIBS)
hpievents_LDFLAGS      =

hpipower_SOURCES      = hpipower.c
hpipower_LDADD        = $(COMMONLIBS)
hpipower_LDFLAGS      =

hpitree_SOURCES      = hpitree.c
hpitree_LDADD        = $(COMMONLIBS)
hpitree_LDFLAGS      =

hpionIBMblade_SOURCES      = hpionIBMblade.c
hpionIBMblade_LDADD        = $(COMMONLIBS)
hpionIBMblade_LDFLAGS      =

clean-local:
	rm -f *~ *.o

