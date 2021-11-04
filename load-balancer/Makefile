CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
LL=LinkedList
HT=Hashtable

.PHONY: build clean

build: tema2

main.o: main.c
	$(CC) $(CFLAGS) $^ -c
	
$(LL).o: $(LL).c $(LL).h
	$(CC) $(CFLAGS) $^ -c
	
$(HT).o: $(HT).c $(HT).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c
	
tema2: main.o $(LOAD).o $(SERVER).o $(LL).o $(HT).o
	$(CC) $^ -o $@

pack:
	zip -FSr 313CA_StanciuAlex_Tema2.zip README Makefile *.c *.h

clean:
	rm -f *.o tema2 *.h.gch