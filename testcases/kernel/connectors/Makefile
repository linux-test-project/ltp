SUBDIRS = pec

all:
	@set -e; \
	if tst_kvercmp 2 6 15; then \
		echo "system does not support process event connector"; \
	else $(MAKE) -C pec; true; fi

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
