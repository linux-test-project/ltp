test_base: tbase_ki.o user_tbase.o
	gcc tbase_ki.o user_tbase.o -o test_base

tbase_ki.o: tbase_ki.c
	gcc -c tbase_ki.c

user_tbase.o: user_tbase.c
	gcc -c user_tbase.c

clean: 
	 rm -f *.o 2>/dev/null || true
