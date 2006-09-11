SCRIPTS = mcast4-pktfld01 mcast4-pktfld02 \
	  mcast6-pktfld01 mcast6-pktfld02

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
