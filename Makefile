
CFLAGS+=
LDFLAGS+=

all: libltp.a
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@
	@$(MAKE) -C pan $@

libltp.a:
	@$(MAKE) -C lib $@

clean:
	@$(MAKE) -C lib $@
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@
	@$(MAKE) -C pan $@
