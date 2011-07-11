#
#  $Id: Makefile,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
#
#  Send feedback to <socketcan-users@lists.berlios.de>

CFLAGS    = -O2 -Wall -Wno-parentheses \
	-fno-strict-aliasing \
	-DETH_P_CAN=0x000C \
	-DPF_CAN=29 \
	-DAF_CAN=PF_CAN

PROGRAMS =      tst-filter tst-rcv-own-msgs

all: $(PROGRAMS)

install:
	cp -f $(PROGRAMS) /usr/local/bin

clean:
	rm -f $(PROGRAMS)

distclean:
	rm -f $(PROGRAMS) *~

