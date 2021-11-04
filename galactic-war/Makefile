CC=gcc
CFLAGS=-Wall -Wextra -std=c99

TARGETS=main

build: $(TARGETS)

main: Main.c CircularDoublyLinkedList.c CircularDoublyLinkedList.h utils.h
	$(CC) $(CFLAGS) Main.c CircularDoublyLinkedList.c -o main

pack:
	zip -FSr 313CA_StanciuAlex_Tema1.zip README Makefile *.c *.h

clean:
	rm -f $(TARGETS)

.PHONY: pack clean
