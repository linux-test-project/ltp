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

REPORTDIR = report_html
CONFORMDIR = conform_html
SERVER = shell1.sf.net
RDIR = /home/groups/o/op/openhpi/htdocs/coverage_report
CONFORMRDIR = /home/groups/o/op/openhpi/htdocs/conform_report
SFUSER = $(shell if test $(SFUID) != "" > /dev/null 2>&1; then echo $(SFUID); else echo $(USER); fi;)

MAINTAINERCLEANFILES 	= Makefile.in

MOSTLYCLEANFILES 	= @TEST_CLEAN@ 

# we want all the local perl programs
EXTRA_DIST              = $(shell ls *pl) 

clean-local:
	rm -rf $(REPORTDIR) $(CONFORMDIR)

report:
	./testcoverage.pl
	./coverage_report.pl

conform:
	./testconformance.pl
	./conformance_report.pl


pconform:
	-chmod a+x $(shell echo $(CONFORMDIR)/*html)
	-echo "XBitHack on" > $(CONFORMDIR)/.htaccess
	-echo "Options +Indexes" >> $(CONFORMDIR)/.htaccess 
	-rsync -e ssh -rP --delete $(CONFORMDIR)/ $(SERVER):$(CONFORMRDIR)
	-ssh $(SERVER) "find $(CONFORMRDIR) -user `whoami` -exec chmod g+w {} \;"
	-ssh $(SERVER) "find $(CONFORMRDIR) -user `whoami` -type d -exec chmod g+s {} \;"

preport: 
	-chmod a+x $(shell echo $(REPORTDIR)/*html)
	-echo "XBitHack on" > $(REPORTDIR)/.htaccess
	-echo "Options +Indexes" >> $(REPORTDIR)/.htaccess 
	-rsync -e ssh -rP --delete $(REPORTDIR)/ $(SFUSER)@$(SERVER):$(RDIR)
	-ssh $(SFUSER)@$(SERVER) "find $(RDIR) -user $(SFUSER) -exec chmod g+w {} \;"
	-ssh $(SFUSER)@$(SERVER) "find $(RDIR) -user $(SFUSER) -type d -exec chmod g+s {} \;"

publish-conform:: conform pconform

publish-report:: report preport

