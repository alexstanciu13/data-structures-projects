// Copyright 2021 Stanciu Alex

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CircularDoublyLinkedList.h"
#include "utils.h"

doubly_linked_list_t*
dll_create(unsigned int data_size)
{
	doubly_linked_list_t* list = calloc(1, sizeof(*list));
	DIE(list == NULL, "Failed to allocate list");
	list->head = NULL;
	list->size = 0;
	list->data_size = data_size;
	return list;
}

dll_node_t*
dll_get_nth_node(doubly_linked_list_t* list, unsigned int n)
{
	unsigned int i;
	dll_node_t *curr = NULL;
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return curr;
	}
	if (list->size == 0)
		n = 0;
	else
		n = n % list->size;
	curr = list->head;
	for (i = 0; i < n; i++)
		curr = curr->next;
	return curr;
}

void
dll_add_nth_node(doubly_linked_list_t* list, unsigned int n,
	const void* new_data)
{
	unsigned int i;
	dll_node_t* new_node;
	dll_node_t* curr;
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	}
	new_node = calloc(1, sizeof(*new_node));
	DIE(new_node == NULL, "Failed to allocate new node");
	new_node->data = calloc(1, list->data_size);
	DIE(new_node->data == NULL, "Failed to allocate data for new node");
	memcpy(new_node->data, new_data, list->data_size);
	if (n > list->size)
		n = list->size;
	if (list->head == NULL){
		list->head = new_node;
		list->head->next = list->head;
		list->head->prev = list->head;
	} else if (n == 0){
		list->head->prev->next = new_node;
		new_node->prev = list->head->prev;
		list->head->prev = new_node;
		new_node->next = list->head;
		list->head = new_node;
	} else {
		curr = list->head;
		for (i = 1; i < n; i++)
			curr = curr->next;
		new_node->next = curr->next;
		curr->next->prev = new_node;
		curr->next = new_node;
		new_node->prev = curr;
	}
	list->size++;
}

dll_node_t*
dll_remove_nth_node(doubly_linked_list_t* list, unsigned int n)
{
	unsigned int i;
	dll_node_t *curr = NULL;
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return curr;
	}
	curr = list->head;
	if (n == 0) {
		curr->next->prev = curr->prev;
		curr->prev->next = curr->next;
		list->head = curr->next;
		list->size--;
		if(list->size == 0) {
			list->head = NULL;
		}
		return curr;
	} else if (n > list->size - 1) {
        curr = list->head->prev;
        curr->next->prev = curr->prev;
        curr->prev->next = curr->next;
        list->size--;
        if (list->size == 0) {
            list->head = NULL;
        }
        return curr;
    } else {
		for (i = 0; i < n; i++)
			curr = curr->next;
		curr->next->prev = curr->prev;
		curr->prev->next = curr->next;
		list->size--;
		return curr;
	}
}

unsigned int
dll_get_size(doubly_linked_list_t* list)
{
	unsigned int count = 1;
	dll_node_t* curr;
	curr = list->head;
	if (curr == NULL)
		return 0;
	else
	{
		while (curr->next != list->head){
			count++;
			curr = curr->next;
		}
	}
	return count;
}

void
dll_free(doubly_linked_list_t** pp_list)
{
	unsigned int i;
	dll_node_t *curr = NULL;
	if ((*pp_list)->size == 0) {
		free(*pp_list);
		return;
	}
	if ((*pp_list)->size == 1) {
		free((*pp_list)->head->data);
		free((*pp_list)->head);
		free(*pp_list);
		return;
	}
	curr = (*pp_list)->head;
	for (i = 1; i < (*pp_list)->size; i++) {
		curr = curr->next;
		free(curr->prev->data);
		free(curr->prev);
	}
	free(curr->data);
	free(curr);
	free(*pp_list);
}

void
dll_print_int_list(doubly_linked_list_t* list)
{
	unsigned int i;
	dll_node_t *curr;
	if (list == NULL)
		return;
	curr = list->head;
	for (i = 0; i < list->size; i++){
		printf("%d ", * ((int*)curr->data));
		curr = curr->next;
	}
	printf("\n");
}

void
dll_print_string_list(doubly_linked_list_t* list)
{
	unsigned int i;
	dll_node_t *curr;
	if (list == NULL)
		return;
	curr = list->head;
	for (i = 0; i < list->size; i++){
		printf("%s ", (char*)curr->data);
		curr = curr->next;
	}
	printf("\n");
}

void
dll_print_ints_left_circular(dll_node_t* start)
{
	dll_node_t *curr;
	curr = start;
	printf("%d ", *((int *)curr->data));
	curr = curr->prev;
	while (curr != start){
		printf("%d ", *((int *)curr->data));
		curr = curr->prev;
	}
	printf("\n");
}

void
dll_print_ints_right_circular(dll_node_t* start)
{
	dll_node_t *curr;
	curr = start;
	printf("%d ", *((int *)curr->data));
	curr = curr->next;
	while (curr != start){
		printf("%d ", *((int *)curr->data));
		curr = curr->next;
	}
	printf("\n");
}
