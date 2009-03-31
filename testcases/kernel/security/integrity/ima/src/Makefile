include ../../../../../../config.mk

ifeq ($(CRYPTO_LIB),-lcrypto)
	TARGETS=ima_mmap ima_measure ima_boot_aggregate
	LDLIBS += $(CRYPTO_LIB)
else
	TARGETS=ima_mmap
endif

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done

clean:
	rm -f $(TARGETS)
