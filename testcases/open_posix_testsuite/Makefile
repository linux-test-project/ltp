SUBDIRS = `ls */Makefile | sed "s/Makefile//g"`

all:
	@set -e; rm -f 00_Build_Errors; for i in $(SUBDIRS); do $(MAKE) -i -C $$i 2>> 00_Build_Errors; done
	@echo ""
	@echo "**********************************************"
	@echo "Open POSIX Test Suite build errors located in `pwd`/00_Build_Errors"
	@echo "**********************************************"
	@echo ""
	@sleep 5

install:
	@set -e; rm -f 00_Install_Errors; for i in $(SUBDIRS); do $(MAKE) -i -C $$i install 2>> 00_Install_Errors ; done
	@echo ""
	@echo "************************************************"
	@echo "Open POSIX Test Suite install errors located in `pwd`/00_Install_Errors"
	@echo "************************************************"
	@echo ""
	@sleep 5

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -i -C $$i clean ; done; rm -f 00_*_Errors


