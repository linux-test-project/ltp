SCRIPTS = http4-stress http6-stress \
	  http-stress01-rmt http-stress02-rmt

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../bin/$$i ; done
