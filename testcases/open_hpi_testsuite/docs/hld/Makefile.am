#
# Copyright (c) 2003, Intel Corporation
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or 
# without modification, are permitted provided that the following 
# conditions are met:
# 
# Redistributions of source code must retain the above copyright 
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the distribution.
# 
# Neither the name of Intel Corporation nor the names 
# of its contributors may be used to endorse or promote products 
# derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  
DIAGRAMS = $(shell echo $(srcdir)/images/*png $(srcdir)/images/*gif) 

SERVER = shell1.sourceforge.net
HLDDIR = /home/groups/o/op/openhpi/htdocs/manual
SFUSER = $(shell if test $(SFUID) != "" > /dev/null 2>&1; then echo $(SFUID); else echo $(USER); fi;)

MAINTAINERCLEANFILES 	= Makefile.in  

openhpi_docsdir		= $(datadir)/doc/$(PACKAGE_NAME)-$(VERSION)/html

openhpi_docs_DATA	= openhpi-manual/*.html

openhpi_imgdir		= $(datadir)/doc/$(PACKAGE_NAME)-$(VERSION)/html/images

openhpi_img_DATA	= openhpi-manual/images/*

SGML_FILES              = $(shell echo $(srcdir)/*.dtd) $(shell echo $(srcdir)/*.sgml)

EXTRA_DIST		= $(DIAGRAMS) $(SGML_FILES) htmlfilter.pl

all:

pdf-am:
	docbook2pdf openhpi-manual.sgml

ps-am:
	docbook2ps openhpi-manual.sgml

openhpi-manual/*.html: $(SGML_FILES)
	-docbook2html -o openhpi-manual $(srcdir)/openhpi-manual.sgml
	-mkdir -p openhpi-manual/images
	-cp $(DIAGRAMS) openhpi-manual/images
	-$(srcdir)/htmlfilter.pl $(shell echo $(builddir)/openhpi-manual/*html)
	-chmod a+x $(shell echo $(srcdir)/openhpi-manual/*html)
	if test ! -e openhpi-manual/book1.html; then \
		ln openhpi-manual/index.html openhpi-manual/book1.html; \
	fi;

openhpi-manual/images/*: openhpi-manual/*.html

publish: openhpi-manual/*.html
	-echo "XBitHack on" > openhpi-manual/.htaccess
	-rsync -e ssh -rP --delete openhpi-manual/ $(SFUSER)@$(SERVER):$(HLDDIR)
	-ssh $(SFUSER)@$(SERVER) "find $(HLDDIR) -user $(SFUSER) -exec chmod g+w {} \;"
	-ssh $(SFUSER)@$(SERVER) "find $(HLDDIR) -user $(SFUSER) -type d -exec chmod g+s {} \;"

clean-local:
	rm -fR openhpi-manual
	rm -f *~ *.tex *.aux *.log *.out
