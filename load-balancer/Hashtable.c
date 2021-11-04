/* Copyright 2021 <Alex Stanciu> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "Hashtable.h"
#include "server.h"

/*
 * Functii de comparare a cheilor:
 */
int
compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Functii de hashing:
 */
unsigned int
hash_function_int(void *a)
{
	/*
	 * Credits: https://stackoverflow.com/a/12996028/7883884
	 */
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int
hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 * Trebuie alocate si initializate si listele inlantuite.
 */
server_memory*
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	unsigned int i;
	server_memory* ht;
	ht = malloc(sizeof(*ht));
	ht->buckets = malloc(hmax * sizeof(*ht->buckets));
	DIE(ht->buckets == NULL, "Couldn't allocate");
	for(i = 0; i < hmax; i++)
	{
		ht->buckets[i] = ll_create(sizeof(struct info));
		DIE(ht->buckets[i] == NULL, "Couldn't allocate");
	}
	ht->size = 0;
	ht->hmax = hmax;
	ht->compare_function = compare_function;
	ht->hash_function = hash_function;
	return ht;
}

/*
 * Atentie! Desi cheia este trimisa ca un void pointer (deoarece nu se impune tipul ei), in momentul in care
 * se creeaza o noua intrare in hashtable (in cazul in care cheia nu se gaseste deja in ht), trebuie creata o copie
 * a valorii la care pointeaza key si adresa acestei copii trebuie salvata in structura info asociata intrarii din ht.
 * Pentru a sti cati octeti trebuie alocati si copiati, folositi parametrul key_size_bytes.
 *
 * Motivatie:
 * Este nevoie sa copiem valoarea la care pointeaza key deoarece dupa un apel put(ht, key_actual, value_actual),
 * valoarea la care pointeaza key_actual poate fi alterata (de ex: *key_actual++). Daca am folosi direct adresa
 * pointerului key_actual, practic s-ar modifica din afara hashtable-ului cheia unei intrari din hashtable.
 * Nu ne dorim acest lucru, fiindca exista riscul sa ajungem in situatia in care nu mai stim la ce cheie este
 * inregistrata o anumita valoare.
 */
void
ht_put(server_memory* ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
    struct info data;
    data.key = malloc(key_size);
    DIE(!data.key, "Eroare alocare memorie");
    memcpy(data.key, key, key_size);
    data.value = malloc(value_size);
    DIE(!data.value, "Eroare alocare memorie");
    memcpy(data.value, value, value_size);
    ll_node_t *curr = ht->buckets[index]->head;
    for (unsigned int i = 0; i < ht->buckets[index]->size; i++){
        if (ht->compare_function(data.key,
        	((struct info *)(curr->data))->key) == 0) {
            memcpy(((struct info *)(curr->data))->value,
            	data.value, value_size);
            free(data.value);
            free(data.key);
            return;
        }
        curr = curr->next;
    }
    ll_add_nth_node(ht->buckets[index], ht->buckets[index]->size, &data);
    ht->size++;
}

void *
ht_get(server_memory* ht, void *key)
{
	unsigned int i;
	if(ht == NULL)
	{
		fprintf(stderr, "Hashtable doesn't exist\n");
		exit(-1);
	}
	unsigned int index = ht->hash_function(key);
	index %= ht->hmax;
	ll_node_t *curr = ht->buckets[index]->head;
	for(i = 0; i < ht->buckets[index]->size; i++)
	{
		if(ht->compare_function(key, ((struct info *)curr->data)->key) == 0)
			return ((struct info *)curr->data)->value;
		curr = curr->next;
	}
	return NULL;
}

/*
 * Functie care intoarce:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable folosind functia put
 * 0, altfel.
 */
int
ht_has_key(server_memory* ht, void *key)
{
	unsigned int i;
	if(ht == NULL)
	{
		fprintf(stderr, "Hashtable doesn't exist\n");
		return -1;
	}
	unsigned int index = ht->hash_function(key);
	index %= ht->hmax;
	ll_node_t *curr = ht->buckets[index]->head;
	for(i = 0; i < ht->buckets[index]->size; i++)
	{
		if(ht->compare_function(key, ((struct info *)curr->data)->key) == 0)
			return 1;
		curr = curr->next;
	}
	return 0;
}

/*
 * Procedura care elimina din hashtable intrarea asociata cheii key.
 * Atentie! Trebuie avuta grija la eliberarea intregii memorii folosite pentru o intrare din hashtable (adica memoria
 * pentru copia lui key --vezi observatia de la procedura put--, pentru structura info si pentru structura Node din
 * lista inlantuita).
 */
void
ht_remove_entry(server_memory* ht, void *key)
{
	unsigned int i;
	if(ht == NULL)
	{
		fprintf(stderr, "Hashtable doesn't exist\n");
		return;
	}
	unsigned int index = ht->hash_function(key);
	index %= ht->hmax;
	ll_node_t *curr = ht->buckets[index]->head;
	for(i = 0; i < ht->buckets[index]->size; i++)
	{
		if(ht->compare_function(key, ((struct info *)curr->data)->key) == 0){
			ll_node_t *entry = ll_remove_nth_node(ht->buckets[index], i);
			free(((struct info *)entry->data)->key);
			free(((struct info *)entry->data)->value);
			free(entry->data);
			free(entry);
			ht->size--;
			return;
		}
		curr = curr->next;
	}
}

/*
 * Procedura care elibereaza memoria folosita de toate intrarile din hashtable, dupa care elibereaza si memoria folosita
 * pentru a stoca structura hashtable.
 */
void
ht_free(server_memory* ht)
{
	unsigned int i;
	for(i = 0; i < ht->hmax; i++) {
		ll_node_t* curr;
		linked_list_t *pp_list = ht->buckets[i];
		while (ll_get_size(pp_list) > 0) {
			curr = ll_remove_nth_node(pp_list, 0);
			free(((struct info *)curr->data)->key);
			free(((struct info *)curr->data)->value);
			free(curr->data);
			free(curr);
		}
		free(pp_list);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int
ht_get_size(server_memory* ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int
ht_get_hmax(server_memory* ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
