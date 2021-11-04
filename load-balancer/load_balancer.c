/* Copyright 2021 <Alex Stanciu> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"

struct server_ids {
	int server_id;
	server_memory *server_address;
};

struct load_balancer {
	unsigned int *hashring;
	server_ids *servers;
	int nrservers;
};

unsigned int hash_function_servers(void *a) {
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *) a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}


load_balancer* init_load_balancer() {
	load_balancer *main;
	main = malloc(sizeof(*main));
	main->hashring = malloc(300000 * sizeof(unsigned int));
	DIE(main->hashring == NULL, "Failed malloc");
	main->servers = malloc(100000 * sizeof(main->servers));
	DIE(main->servers == NULL, "Failed malloc");
	main->nrservers = 0;
	return main;
}

void loader_store(load_balancer* main, char* key, char* value, int* server_id) {
	int pos = 0;  // in cazul in care hash-ul e mai mare decat ultimul server
	// cheia sa se stocheze pe primul server
	for (int i = 0; i < 3 * main->nrservers; i++){
		if (hash_function_key(key) <
			hash_function_servers(&main->hashring[i])){
			pos = i;
			break;
		}
	}
	*server_id = main->hashring[pos] % 100000;
	for (int i = 0; i < main->nrservers; i++){
		if (main->servers[i].server_id == *server_id){
			server_store(main->servers[i].server_address, key, value);
			break;
		}
	}
}


char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	char *value;
	int pos = 0;
	for (int i = 0; i < 3 * main->nrservers; i++){
		if (hash_function_key(key) <
			hash_function_servers(&main->hashring[i])){
			pos = i;
			break;
		}
	}
	*server_id = main->hashring[pos] % 100000;
	for (int i = 0; i < main->nrservers; i++){
		if (main->servers[i].server_id == *server_id){
			value = server_retrieve(main->servers[i].server_address, key);
			break;
		}
	}
	return value;
}

void balance(load_balancer* main, unsigned int r, int size){
	for (int i = 0; i < size; i++){
		if (main->hashring[i] == r){
			int posserver;  // pozitia server-ului din care scot
			for (int j = 0; j <= main->nrservers; j++){
				if (main->servers[j].server_id ==
					(int)main->hashring[(i + 1) % size] % 100000){
					posserver = j;
					break;
				}
			}
			int posserverring = (i + 1) % (size);
			int idposserver[3];
			idposserver[0] = main->servers[posserver].server_id;
			idposserver[1] = main->servers[posserver].server_id + 100000;
			idposserver[2] = main->servers[posserver].server_id + 200000;
			for (int j = 0; j < 2; j++){
				for (int k = j + 1; k < 3; k++){
					if (hash_function_servers(&idposserver[j]) >
						hash_function_servers(&idposserver[k])){
						int temp = idposserver[j];
						idposserver[j] = idposserver[k];
						idposserver[k] = temp;
					}
				}
			}
			// sa nu mutam pe aceeasi replica
			if ((int)main->hashring[i] % 100000 ==
				main->servers[posserver].server_id)
				return;
			for (int k = 0; k < 200; k++){
				// parcurgere buckets din hashtable
				ll_node_t *curr =
				main->servers[posserver].server_address->buckets[k]->head;
				while (curr != NULL){
					int p = 0;
					for (int j = 0; j < 3; j++){
						if (hash_function_key(((struct info *)curr->data)->key) <
							hash_function_servers(&idposserver[j])){
							p = j;
							break;
						}
					}
					if ((int)main->hashring[posserverring] !=
						idposserver[p]){
						curr = curr->next;
						continue;
					}
					// cele 3 cazuri pentru pozitiile server-ului
					if (i == 0 && (hash_function_key(((struct info *)curr->data)->key) <
						hash_function_servers(&main->hashring[i]) ||
						hash_function_key(((struct info *)curr->data)->key) >
						hash_function_servers(&main->hashring[size - 1]))){
						char* key2 = ((struct info*)curr->data)->key;
						char* value2 = ((struct info*)curr->data)->value;
						server_store(main->servers[main->nrservers].server_address,
							key2, value2);
						curr = curr->next;
						server_remove(main->servers[posserver].server_address, key2);
					} else if (i == size - 1 &&
						(hash_function_key(((struct info *)curr->data)->key) <
							hash_function_servers(&main->hashring[i]) &&
							hash_function_key(((struct info *)curr->data)->key) >
							hash_function_servers(&main->hashring[0]))){
						char* key2 = ((struct info*)curr->data)->key;
						char* value2 = ((struct info*)curr->data)->value;
						server_store(main->servers[main->nrservers].server_address,
							key2, value2);
						curr = curr->next;
						server_remove(main->servers[posserver].server_address, key2);
					} else if ((i != 0 && i != size - 1) &&
						hash_function_key(((struct info *)curr->data)->key) <
						hash_function_servers(&main->hashring[i])){
						char* key2 = ((struct info*)curr->data)->key;
						char* value2 = ((struct info*)curr->data)->value;
						server_store(main->servers[main->nrservers].server_address,
							key2, value2);
						curr = curr->next;
						server_remove(main->servers[posserver].server_address, key2);
					} else {
							curr = curr->next;
					}
				}
			}
		}
	}
}

void loader_add_server(load_balancer* main, int server_id) {
	unsigned int r1, r2, r3;
	int pos1 = -1, pos2 = -1, pos3 = -1;
	// cele 3 replici
	r1 = server_id;
	r2 = 100000 + server_id;
	r3 = 200000 + server_id;
	// adaugarea primul server
	if (main->nrservers == 0){
		main->hashring[0] = r1;
		main->hashring[1] = r2;
		main->hashring[2] = r3;
		for (int i = 0; i < 2; i++)
			for (int j = i + 1; j < 3; j++)
				if (hash_function_servers(&main->hashring[i]) >
					hash_function_servers(&main->hashring[j])){
					unsigned int temp = main->hashring[i];
					main->hashring[i] = main->hashring[j];
					main->hashring[j] = temp;
				}
		main->nrservers++;
		main->servers[main->nrservers - 1].server_id = server_id;
		main->servers[main->nrservers - 1].server_address = init_server_memory();
		return;
	} else {
		main->servers[main->nrservers].server_id = server_id;
		main->servers[main->nrservers].server_address = init_server_memory();
		// pentru fiecare replica adaugam pe hashring si redistribuim obiectele
		for (int i = 0; i < 3 * main->nrservers; i++){
			if (hash_function_servers(&r1) <
				hash_function_servers(&main->hashring[i])){
				pos1 = i;
				break;
			}
		}
		if (pos1 == -1)
			pos1 = 3 * main->nrservers;
		for (int i = 3 * main->nrservers - 1; i >= pos1; i--){
			main->hashring[i + 1] = main->hashring[i];
		}
		main->hashring[pos1] = r1;
		balance(main, r1, 3 * main->nrservers + 1);

		for (int i = 0; i < 3 * main->nrservers + 1; i++){
			if (hash_function_servers(&r2) <
				hash_function_servers(&main->hashring[i])){
				pos2 = i;
				break;
			}
		}
		if (pos2 == -1)
			pos2 = 3 * main->nrservers + 1;
		for (int i = 3 * main->nrservers; i >= pos2; i--){
			main->hashring[i + 1] = main->hashring[i];
		}
		main->hashring[pos2] = r2;
		balance(main, r2, 3 * main->nrservers + 2);

		for (int i = 0; i < 3 * main->nrservers + 2; i++){
			if (hash_function_servers(&r3) <
				hash_function_servers(&main->hashring[i])){
				pos3 = i;
				break;
			}
		}
		if (pos3 == -1)
			pos3 = 3 * main->nrservers + 2;
		for (int i = 3 * main->nrservers + 1; i >= pos3; i--){
			main->hashring[i + 1] = main->hashring[i];
		}
		main->hashring[pos3] = r3;
		balance(main, r3, 3 * main->nrservers + 3);

		main->nrservers++;
	}
}


void loader_remove_server(load_balancer* main, int server_id) {
	int posserver, s1 = -1, s2 = -1, s3 = -1;
	for (int i = 0; i < main->nrservers; i++){
		if (main->servers[i].server_id == server_id){
			posserver = i;
			break;
		}
	}
	// cele 3 servere unde vom face redistribuirea
	for (int i = 0; i < 3 * main->nrservers; i++){
		if ((int)main->hashring[i] % 100000 ==
			server_id){
			if (s1 == -1)
				s1 = i;
			else if (s1 != -1 && s2 == -1)
				s2 = i;
			else if (s2 != -1 && s3 == -1)
				s3 = i;
		}
	}
	for (int i = s3; i < 3 * main->nrservers; i++)
		main->hashring[i] = main->hashring[i + 1];
	for (int i = s2; i < 3 * main->nrservers - 1; i++)
		main->hashring[i] = main->hashring[i + 1];
	for (int i = s1; i < 3 * main->nrservers - 2; i++)
		main->hashring[i] = main->hashring[i + 1];
	main->nrservers--;
	server_ids tmp = main->servers[posserver];
	for (int i = posserver; i < main->nrservers; i++){
		main->servers[i] = main->servers[i + 1];
	}
	for (int k = 0; k < 200; k++){
		ll_node_t *curr = tmp.server_address->buckets[k]->head;
		while (curr != NULL){
			int id2 = 0;
			char* key2 = ((struct info*)curr->data)->key;
			char* value2 = ((struct info*)curr->data)->value;
			// stocam obiectul
			loader_store(main, key2, value2, &id2);
			curr = curr->next;
		}
	}
	free_server_memory(tmp.server_address);
}

void free_load_balancer(load_balancer* main) {
	for (int i = 0; i < main->nrservers; i++){
		free_server_memory(main->servers[i].server_address);
	}
	free(main->servers);
	free(main->hashring);
	free(main);
}
