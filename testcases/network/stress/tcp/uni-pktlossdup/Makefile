SCRIPTS = tcp4-uni-pktlossdup01 \
	  tcp4-uni-pktlossdup02 tcp4-uni-pktlossdup03 \
	  tcp4-uni-pktlossdup04 tcp4-uni-pktlossdup05 \
	  tcp4-uni-pktlossdup06 tcp4-uni-pktlossdup07 \
	  tcp4-uni-pktlossdup08 \
	  tcp4-uni-pktlossdup09 tcp4-uni-pktlossdup10 \
	  tcp4-uni-pktlossdup11 tcp4-uni-pktlossdup12 \
	  tcp4-uni-pktlossdup13 tcp4-uni-pktlossdup14 \
	  tcp6-uni-pktlossdup01 \
	  tcp6-uni-pktlossdup02 tcp6-uni-pktlossdup03 \
	  tcp6-uni-pktlossdup04 tcp6-uni-pktlossdup05 \
	  tcp6-uni-pktlossdup06 tcp6-uni-pktlossdup07 \
	  tcp6-uni-pktlossdup08 \
	  tcp6-uni-pktlossdup09 tcp6-uni-pktlossdup10 \
	  tcp6-uni-pktlossdup11 tcp6-uni-pktlossdup12 \
	  tcp6-uni-pktlossdup13 tcp6-uni-pktlossdup14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
