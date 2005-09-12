SUBDIRS	= tpm_selftest		\
	tpm_version		\
	tpm_getpubek		\
	tpm_takeownership	\
	tpm_restrictpubek	\
	tpm_changeauth		\
	tpm_clear

all:

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:

