/* Copyright 2021 <Alex Stanciu> */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "LinkedList.h"

struct info {
	void *key;
	void *value;
};

typedef struct server_memory server_memory;

struct server_memory {
	linked_list_t **buckets; /* Array de liste simplu-inlantuite. */
	/* Nr. total de noduri existente curent in toate bucket-urile. */
	unsigned int size;
	unsigned int hmax; /* Nr. de bucket-uri. */
	/* (Pointer la) Functie pentru a calcula valoarea hash asociata cheilor. */
	unsigned int (*hash_function)(void*);
	/* (Pointer la) Functie pentru a compara doua chei. */
	int (*compare_function)(void*, void*);
};

server_memory*
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*));

void
ht_put(server_memory* ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

void *
ht_get(server_memory* ht, void *key);

int
ht_has_key(server_memory* ht, void *key);

void
ht_remove_entry(server_memory* ht, void *key);

unsigned int
ht_get_size(server_memory* ht);

unsigned int
ht_get_hmax(server_memory* ht);

void
ht_free(server_memory* ht);

/*
 * Functii de comparare a cheilor:
 */
int
compare_function_ints(void *a, void *b);

int
compare_function_strings(void *a, void *b);

/*
 * Functii de hashing:
 */
unsigned int
hash_function_int(void *a);

unsigned int
hash_function_string(void *a);

#endif  // HASHTABLE_H_
