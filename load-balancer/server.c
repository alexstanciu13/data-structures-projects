/* Copyright 2021 <Alex Stanciu> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "LinkedList.h"
#include "utils.h"
#include "Hashtable.h"

server_memory* init_server_memory() {
	server_memory *server = ht_create(200, hash_function_string,
		compare_function_strings);
	return server;
}

void server_store(server_memory* server, char* key, char* value) {
	ht_put(server, key, strlen(key) + 1, value, strlen(value) + 1);
}

void server_remove(server_memory* server, char* key) {
	ht_remove_entry(server, key);
}

char* server_retrieve(server_memory* server, char* key) {
	return ht_get(server, key);
}

void free_server_memory(server_memory* server) {
	ht_free(server);
}
