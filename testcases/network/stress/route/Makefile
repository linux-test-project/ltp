SCRIPTS = route4-change-dst route4-change-gw route4-change-if \
	  route4-redirect route4-ifdown route4-rmmod \
	  route6-change-dst route6-change-gw route6-change-if \
	  route6-redirect route6-ifdown route6-rmmod

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../bin/$$i ; done
