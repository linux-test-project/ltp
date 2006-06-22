# To cross compile, override one or more of CC, AR, CROSS_CFLAGS,
# LOADLIBES, LDFLAGS, & LIB_DIR and be sure to always build from the top level.
#
# To override them from the make commandline, do something like this:
# $ CROSS_COMPILER=/opt/cegl-1.4/hardhat/devkit/ppc/405/bin/powerpc-linux-
# $ make \
#     CROSS_CFLAGS="-mcpu=403 -D__PPC405__" \
#     LDFLAGS=-static \
#     LOADLIBES="-lpthread -lc -lresolv -lnss_dns -lnss_files -lm -lc"
#
# Alternately, to override them by editing this file, uncomment the 
# following lines:
#   CROSS_COMPILER=/opt/ppc64/powerpc64-linux/bin/powerpc64-linux-
#   CROSS_CFLAGS= -mpowerpc64
#   LDFLAGS=-static 
#   LOADLIBES=-lpthread -lc -lresolv -lnss_dns -lnss_files -lm -lc
#   LIB_DIR=/lib64
#   export LOADLIBES LIB_DIR
#
# Or, you can save all your settings into the local 'config.mk' file.
# Just use the same syntax as above for the Makefile.
#
# uClinux Users: make sure you add -DUCLINUX to your CFLAGS
#
# Note: If you override a variable from the commandline all
# assignments to it in the Makefiles will be ignored. To set it both 
# in the commandline and in the Makefiles use a dummy variable like in
# CFLAGS

ifdef CROSS_COMPILE
CROSS_COMPILER = $(CROSS_COMPILE)
endif
ifdef CROSS_COMPILER
CC=$(CROSS_COMPILER)gcc
AR=$(CROSS_COMPILER)ar
endif

export CFLAGS += -Wall $(CROSS_CFLAGS)
export CC AR LDFLAGS

-include config.mk

all: libltp.a 
	@$(MAKE) -C pan $@
	@$(MAKE) -C testcases $@
	@$(MAKE) -C tools $@
	@echo
	@echo "***********************************************"
	@echo "** You now need to do a make install as root **"
	@echo "***********************************************"

install: all
	@$(MAKE) -C testcases install
	@$(MAKE) -C tools install
	@./IDcheck.sh

libltp.a:
	@$(MAKE) -C lib $@

uclinux: uclinux_libltp.a
	#@$(MAKE) -C pan all
	@$(MAKE) -C testcases uclinux
	@$(MAKE) -C tools all
	@echo
	@echo "*******************************************************"
	@echo "** You now need to do a make uclinux_install as root **"
	@echo "*******************************************************"

uclinux_install: uclinux
	@$(MAKE) -C testcases uclinux_install
	@$(MAKE) -C tools install
	@./IDcheck.sh

uclinux_libltp.a:
	@$(MAKE) -C lib UCLINUX=1 libltp.a

menuconfig:
	@./ltpmenu

clean:
	@$(MAKE) -C lib $@
	@$(MAKE) -C pan $@
	@$(MAKE) -C tools $@
	@$(MAKE) -C testcases $@
