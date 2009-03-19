#
# Copyright (c) Casey Schaufler, 2009
#
# This program is free software;  you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

SCRIPTS = \
	smack_set_ambient.sh \
	smack_set_cipso.sh \
	smack_set_current.sh \
	smack_set_direct.sh \
	smack_set_doi.sh \
	smack_set_load.sh \
	smack_set_netlabel.sh \
	smack_set_onlycap.sh \
	smack_file_access.sh

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))
INSTALLTARGETS = $(TARGETS) $(SCRIPTS)
all: $(TARGETS)

INSTALLTARGETS = $(TARGETS) $(SCRIPTS)
install: $(INSTALLTARGETS)
	@set -e; for i in $(INSTALLTARGETS); do ln -f $$i ../../../bin/$$i ; chmod +x ../../../bin/$$i; done

clean:
	rm -f $(TARGETS) *.o
