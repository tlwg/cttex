all: cttex dict2state

cttex: cttex.c map.o
	gcc -O2 -o cttex cttex.c map.o

map.o: map.c
	gcc -O2 -c map.c

dictsort: dictsort.c
	gcc -O2 -o dictsort dictsort.c

dict2state: dict2state.c
	gcc -O2 -o dict2state dict2state.c

map.c: tdict.txt dict2state
	./dict2state

tdict.txt: tdict.org tdict.hui
	cat tdict.org tdict.hui > tdict.txt

clean: 
	rm -f cttex dictsort dict2state tdict.h map.c map.h *~ *.o \
	tdict.sorted tdict.txt

