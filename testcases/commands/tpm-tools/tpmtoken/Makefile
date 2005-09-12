SUBDIRS	= tpmtoken_init		\
	tpmtoken_setpasswd	\
	tpmtoken_import		\
	tpmtoken_protect	\
	tpmtoken_objects

all:

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:

