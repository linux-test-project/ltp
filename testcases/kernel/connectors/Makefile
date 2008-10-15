SUBDIRS = pec

all:
	@set -e; \
	if ! test -f /proc/net/connector; then \
		echo "system does not support process event connector"; \
	else for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done; true; fi

install:
	@set -e; \
	ln -f connector_test.sh ../../bin/connector_test.sh; \
	chmod ugo+x connector_test.sh; \
	if ! test -f /proc/net/connector; then \
		echo "system does not support process event connector"; \
	else for i in $(SUBDIRS); do $(MAKE) -C $$i install; done; true; fi

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done
