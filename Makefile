
CFLAGS+= -Wall
LDFLAGS+=

all: libltp.a
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
	@$(MAKE) -C testcases install

libltp.a:
	@$(MAKE) -C lib $@

clean:
	@$(MAKE) -C lib $@
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
	@$(MAKE) -C testcases $@
