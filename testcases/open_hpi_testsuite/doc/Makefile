# Copyright (c) 2003 by Intel Corp.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Authors:
#     Kevin Gao <kevin.gao@intel.com>

SERVER=shell1.sourceforge.net
LDIR=test_specification/
DIR=/home/groups/o/op/openhpi/htdocs/test_spec

RSYNC=rsync -rlvC --exclude '.emacs*' --rsh=ssh ./$(LDIR) $(SERVER):$(DIR)
	CHMOD=ssh $(SERVER) "find $(DIR) -user `whoami` -exec chmod g+w {} \;"
	CHMODD=ssh $(SERVER) "find $(DIR) -user `whoami` -type d -exec chmod g+s {} \;"
	

DOCS = test_specification.sgml \
       bookinfo.sgml           \
       features.sgml           \
       strategies.sgml         \
       environment.sgml        \
       introduction.sgml       \
       test_specification.sgml

all: $(DOCS)
	db2html test_specification.sgml

pdf: $(DOCS)
	db2pdf test_specification.sgml

publish: all
	$(RSYNC)
	$(CHMOD)
	$(CHMODD)


clean:
	rm -f *~ 
	rm -fR test_specification
	rm -fR test_specification.junk
