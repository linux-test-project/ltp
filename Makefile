
CFLAGS+= -Wall
LDFLAGS+=

all: libltp.a
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
	@$(MAKE) -C ltctests $@
	@$(MAKE) -C ltctests install

libltp.a:
	@$(MAKE) -C lib $@

clean:
	@$(MAKE) -C lib $@
	@$(MAKE) -C doio $@
	@$(MAKE) -C tests $@
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
	@$(MAKE) -C ltctests $@
