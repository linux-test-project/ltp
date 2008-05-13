# Propagate make "commands" (TARGETS) to subdirs
TARGETS=all clean install uninstall
SUBDIRS=bin

.PHONY: $(TARGETS)

$(TARGETS):%:
	@set -e ; \
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@ ; \
	done
