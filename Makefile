
# To cross compile set vars like this and be sure to always build
# from the top level...

#CROSS_COMPILER=/opt/ppc64/powerpc64-linux/bin/
#CC=$(CROSS_COMPILER)gcc
#AR=$(CROSS_COMPILER)ar
CFLAGS+= -Wall
#LDFLAGS+= -static
#LOADLIBES+= -lpthread -lc -lresolv -lnss_dns -lnss_files -lm -lc
export CC AR LDFLAGS LOADLIBES


all: libltp.a 
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
	@$(MAKE) -C testcases $@
	@echo
	@echo "***********************************************"
	@echo "** You now need to do a make install as root **"
	@echo "***********************************************"

install: all
	@$(MAKE) -C testcases install
	./IDcheck.sh

libltp.a:
	@$(MAKE) -C lib $@

clean:
	@$(MAKE) -C lib $@
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
	@$(MAKE) -C testcases $@
