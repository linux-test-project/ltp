SUBDIRS = pec

all:
	@set -e; \
	if ../../bin/tst_kvercmp 2 6 15; then \
		echo "system does not support process event connector"; \
	else for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done; true; fi

install:
	@set -e; \
	ln -f connector_test.sh ../../bin/connector_test.sh; \
	chmod ugo+x connector_test.sh; \
	if ../../bin/tst_kvercmp 2 6 15; then \
		echo "system does not support process event connector"; \
	else for i in $(SUBDIRS); do $(MAKE) -C $$i install; done; true; fi

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done
