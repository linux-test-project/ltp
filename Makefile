
CFLAGS+=
LDFLAGS+=

all: libltp.a
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@

libltp.a:
	@$(MAKE) -C lib $@

clean:
	@$(MAKE) -C lib $@
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@
