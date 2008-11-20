MAINTAINERCLEANFILES = Makefile.in
MOSTLYCLEANFILES     = @TEST_CLEAN@

INCLUDES             = @OPENHPI_INCLUDES@

EXTRA_DIST	     = oh_clients.h

COMMONLIBS 	     = @STATIC_PLUGIN_LIBS@ @STATIC_PLUGIN_EXTRA_LIBS@

COMMONLIBS	     += $(top_builddir)/openhpid/libopenhpi.la

CLIENTS_SRC 	     = clients.c

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

hpialarms_SOURCES   = hpialarms.c $(CLIENTS_SRC)
hpialarms_LDADD     = $(COMMONLIBS) 

hpithres_SOURCES        = hpithres.c $(CLIENTS_SRC)
hpithres_LDADD          = $(COMMONLIBS)

hpifan_SOURCES          = hpifan.c $(CLIENTS_SRC)
hpifan_LDADD            = $(COMMONLIBS)

hpitop_SOURCES          = hpitop.c $(CLIENTS_SRC)
hpitop_LDADD            = $(COMMONLIBS)

hpireset_SOURCES        = hpireset.c $(CLIENTS_SRC)
hpireset_LDADD          = $(COMMONLIBS)

hpiel_SOURCES           = hpiel.c $(CLIENTS_SRC)
hpiel_LDADD             = $(COMMONLIBS)

hpisensor_SOURCES       = hpisensor.c $(CLIENTS_SRC)
hpisensor_LDADD         = $(COMMONLIBS)

hpiwdt_SOURCES          = hpiwdt.c $(CLIENTS_SRC)
hpiwdt_LDADD            = $(COMMONLIBS)

hpiinv_SOURCES          = hpiinv.c $(CLIENTS_SRC)
hpiinv_LDADD            = $(COMMONLIBS)

hpisettime_SOURCES      = hpisettime.c $(CLIENTS_SRC)
hpisettime_LDADD        = $(COMMONLIBS)

hpievents_SOURCES      = hpievents.c $(CLIENTS_SRC)
hpievents_LDADD        = $(COMMONLIBS)

hpipower_SOURCES      = hpipower.c $(CLIENTS_SRC)
hpipower_LDADD        = $(COMMONLIBS)

hpitree_SOURCES      = hpitree.c $(CLIENTS_SRC)
hpitree_LDADD        = $(COMMONLIBS)

hpionIBMblade_SOURCES      = hpionIBMblade.c $(CLIENTS_SRC)
hpionIBMblade_LDADD        = $(COMMONLIBS)

clean-local:
	rm -f *~ *.o

