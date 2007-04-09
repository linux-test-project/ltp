CFLAGS = -Wall

all:
	cd ./crasher_mod ; make ; cd ../
	cd ./lkdtm_mod ; make ; cd ../

clean:
	make -C crasher_mod clean
	make -C lkdtm_mod clean
