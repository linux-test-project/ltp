SCRIPTS = ssh4-stress ssh6-stress \
	  ssh-stress01-rmt ssh-stress02-rmt ssh-stress03-rmt

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../bin/$$i ; done
