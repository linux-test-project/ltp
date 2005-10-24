SCRIPTS = dns4-stress dns6-stress \
	  dns-stress01-rmt dns-stress02-rmt

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../bin/$$i ; done
