#
# Copyright IBM 2005
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

MAINTAINERCLEANFILES 	= Makefile.in

EXTRA_DIST		= openhpi.pod.7   openhpid.pod.8      \
                          hpialarms.pod.1 hpifan.pod.1        \
                          hpipower.pod.1  hpisettime.pod.1    \
                          hpitree.pod.1   hpiel.pod.1         \
                          hpiinv.pod.1    hpireset.pod.1      \
                          hpithres.pod.1  hpiwdt.pod.1        \
                          hpievents.pod.1 hpionIBMblade.pod.1 \
                          hpisensor.pod.1 hpitop.pod.1


%.1: %.pod.1
	pod2man --release=$(VERSION) --name=`echo $@ | sed 's/.1//'` \
		-s 1 -c "OpenHPI" $< $@

%.3: %.pod.3
	pod2man --release=$(VERSION) --name=`echo $@ | sed 's/.3//'` \
		-s 3 -c "OpenHPI" $< $@

%.5: %.pod.5
	pod2man --release=$(VERSION) --name=`echo $@ | sed 's/.5//'` \
		-s 5 -c "OpenHPI" $< $@

%.7: %.pod.7
	pod2man --release=$(VERSION) --name=`echo $@ | sed 's/.7//'` \
		-s 7 -c "OpenHPI" $< $@

%.8: %.pod.8
	pod2man --release=$(VERSION) --name=`echo $@ | sed 's/.8//'` \
		-s 8 -c "OpenHPI" $< $@

#mandir = $(datadir)/man
man_MANS = openhpi.7   openhpid.8      \
           hpialarms.1 hpifan.1        \
           hpipower.1  hpisettime.1    \
           hpitree.1   hpiel.1         \
           hpiinv.1    hpireset.1      \
           hpithres.1  hpiwdt.1        \
           hpievents.1 hpionIBMblade.1 \
           hpisensor.1 hpitop.1

clean-local: am_config_clean-local

am_config_clean-local:
	rm -f *~
	find . -name \*\.[13578] | grep -v pod | xargs rm -f


.PHONY: FORCE
