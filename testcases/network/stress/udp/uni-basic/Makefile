SCRIPTS = udp4-uni-basic01 \
	  udp4-uni-basic02 udp4-uni-basic03 \
	  udp4-uni-basic04 udp4-uni-basic05 \
	  udp4-uni-basic06 udp4-uni-basic07 \
	  udp6-uni-basic01 \
	  udp6-uni-basic02 udp6-uni-basic03 \
	  udp6-uni-basic04 udp6-uni-basic05 \
	  udp6-uni-basic06 udp6-uni-basic07

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
