################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################

HAVE_P9AUTH := $(shell sh checkp9auth.sh yesno)

SCRIPTS = runp9auth.sh checkp9auth.sh p9priv.sh p9unpriv.sh

CFLAGS += -I../../../../include -Wall
LDLIBS += -L../../../../lib -lltp -lcap

ifeq ($(HAVE_P9AUTH),yes)
SRCS    = $(wildcard *.c)
TARGETS+= $(patsubst %.c,%,$(SRCS))
endif

all: $(TARGETS)

INSTALLTARGETS = $(TARGETS) $(SCRIPTS)
install: $(INSTALLTARGETS)
	@set -e; for i in $(INSTALLTARGETS); do ln -f $$i ../../../bin/$$i ; chmod +x ../../../bin/$$i; done

clean:
	rm -f $(TARGETS) *.o unhex
