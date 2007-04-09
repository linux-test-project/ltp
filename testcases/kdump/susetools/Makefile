CFLAGS = -Wall

all:
	make -C dtt_tools all
	cd ./crasher_mod ; make ; cd ../
	cd ./stress-0.18.8 ; ./configure && make ; cd ../

clean:
	make -C dtt_tools clean
	make -C stress-0.18.8 clean
	make -C crasher_mod clean
