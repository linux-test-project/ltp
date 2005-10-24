SCRIPTS = ftp4-download-stress ftp6-download-stress \
	  ftp-download-stress01-rmt ftp-download-stress02-rmt \
	  ftp4-upload-stress ftp6-upload-stress \
	  ftp-upload-stress01-rmt ftp-upload-stress02-rmt

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../bin/$$i ; done
