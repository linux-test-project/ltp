MAINTAINERCLEANFILES = Makefile.in

INCLUDES 		= -I$(top_srcdir)/include 

COMMONLIBS		= $(top_builddir)/src/libopenhpi.la

AM_CFLAGS 		= @CFLAGS@

bin_PROGRAMS		= hpialarmpanel \
                          hpievent \
	                  hpifru \
                          hpireset \
                          hpisensor \
                          hpisel \
                          hpiinv \
                          hpiwdt

include_HEADERS		= 

hpialarmpanel_SOURCES	= hpialarmpanel.c
hpialarmpanel_LDADD	= $(COMMONLIBS)
hpialarmpanel_LDFLAGS	= 

hpievent_SOURCES	= hpievent.c
hpievent_LDADD		= $(COMMONLIBS)
hpievent_LDFLAGS	= 

hpifru_SOURCES		= hpifru.c
hpifru_LDADD		= $(COMMONLIBS)
hpifru_LDFLAGS		= 

hpireset_SOURCES	= hpireset.c
hpireset_LDADD		= $(COMMONLIBS)
hpireset_LDFLAGS	= 

hpisel_SOURCES		= hpisel.c
hpisel_LDADD		= $(COMMONLIBS)
hpisel_LDFLAGS		= 

hpisensor_SOURCES	= hpisensor.c
hpisensor_LDADD		= $(COMMONLIBS)
hpisensor_LDFLAGS	= 

hpiwdt_SOURCES		= hpiwdt.c
hpiwdt_LDADD		= $(COMMONLIBS)
hpiwdt_LDFLAGS		= 

hpiinv_SOURCES		= hpiinv.c
hpiinv_LDADD		= $(COMMONLIBS)
hpiinv_LDFLAGS		= 

clean:
	rm -f *~

