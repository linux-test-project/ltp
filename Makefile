
CFLAGS+= -Wall
LDFLAGS+=

all: libltp.a
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@

libltp.a:
	@$(MAKE) -C lib $@

clean:
	@$(MAKE) -C lib $@
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
