// Copyright 2021 Stanciu Alex

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CircularDoublyLinkedList.h"
#include "utils.h"

void
add_planet(doubly_linked_list_t* list, char name[],
	unsigned int index, unsigned int nrshields)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	} else if (index > list->size){
		printf("Planet out of bounds!\n");
		return;
	}
	planetdata *planetd = calloc(1, sizeof(planetdata));
	DIE(planetd == NULL, "Failed calloc");
	strcpy(planetd->name, name);
	planetd->destroyedplanets = 0;
	planetd->nrshields = nrshields;
	planetd->shields = dll_create(sizeof(int));
	// adaugam scuturile cu valoarea 1
	int nr = 1;
	for(unsigned int i = 0; i < nrshields; i++)
		dll_add_nth_node(planetd->shields, 0, &nr);
	// aici se adauga planeta la lista galaxiei
	dll_add_nth_node(list, index, planetd);
	printf("The planet %s has joined the galaxy.\n", name);
	free(planetd);
}

void showplanet(doubly_linked_list_t* list, unsigned int index)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	} else if (index >= list->size){
		printf("Planet out of bounds!\n");
		return;
	}
	// curr va fi nodul corespunzator planetei de la indicele index
	dll_node_t *curr = dll_get_nth_node(list, index);
	printf("NAME: %s\n", ((planetdata *)curr->data)->name);
	printf("CLOSEST: ");
	if (list->size == 1){
		printf("none\n");
	} else if (list->size == 2){
		printf("%s\n", ((planetdata *)curr->next->data)->name);
	} else {
		printf("%s and ", ((planetdata *)curr->prev->data)->name);
		printf("%s\n", ((planetdata *)curr->next->data)->name);
	}
	printf("SHIELDS: ");
	dll_print_int_list(((planetdata *)curr->data)->shields);
	printf("KILLED: %d\n", ((planetdata *)curr->data)->destroyedplanets);
}

void upgrade(doubly_linked_list_t* list, unsigned int index,
	unsigned int indshield, int val)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	} else if (index >= list->size){
		printf("Planet out of bounds!\n");
		return;
	}
	dll_node_t *curr = dll_get_nth_node(list, index);
	if (indshield >= ((planetdata *)curr->data)->shields->size){
		printf("Shield out of bounds!\n");
		return;
	}
	// nodul corespunzator scutului de care avem nevoie
	dll_node_t *currshield = dll_get_nth_node(((planetdata *)curr->data)->shields,
		indshield);
	*((int*)currshield->data) += val;
}

void expand(doubly_linked_list_t* list, unsigned int index, unsigned int val)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	} else if (index >= list->size){
		printf("Planet out of bounds!\n");
		return;
	}
	dll_node_t *curr = dll_get_nth_node(list, index);
	// indicele la care trebuie sa adaugam scutul adica finalul listei
	unsigned int last = ((planetdata *)curr->data)->shields->size;
	dll_add_nth_node(((planetdata *)curr->data)->shields, last, &val);
}

void rmvshield(doubly_linked_list_t* list,
	unsigned int index, unsigned int indshield)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	} else if (index >= list->size){
		printf("Planet out of bounds!\n");
		return;
	}
	dll_node_t *curr = dll_get_nth_node(list, index);
	if (indshield >= ((planetdata *)curr->data)->shields->size){
		printf("Shield out of bounds!\n");
		return;
	} else if (((planetdata *)curr->data)->shields->size == 4){
		printf("A planet cannot have less than 4 shields!\n");
		return;
	}
	dll_node_t *rmv = dll_remove_nth_node(((planetdata *)curr->data)->shields,
		indshield);
	free(rmv->data);
	free(rmv);
}

void rotate(doubly_linked_list_t* list, unsigned int index,
	char s, unsigned int units)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	} else if (index >= list->size){
		printf("Planet out of bounds!\n");
		return;
	} else if (s != 'c' && s != 't'){
		printf("Not a valid direction!\n");
		return;
	}
	dll_node_t *curr = dll_get_nth_node(list, index);
	dll_node_t *currshield = ((planetdata *)curr->data)->shields->head;
	unsigned int n = units % ((planetdata *)curr->data)->shields->size;
	// schimbam head-ul listei units % size pozitii in sens t sau c
	if (s == 't'){
		for (unsigned int i = 0; i < n; i++)
			currshield = currshield->next;
	} else {
		for (unsigned int i = 0; i < n; i++)
			currshield = currshield->prev;
	}
	((planetdata *)curr->data)->shields->head = currshield;
}

void blh(doubly_linked_list_t* list, unsigned int index)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	}
	dll_node_t *curr = dll_remove_nth_node(list, index);
	doubly_linked_list_t *listshields = ((planetdata *)curr->data)->shields;
	dll_free(&listshields);
	free((planetdata *)curr->data);
	free(curr);
}

void col(doubly_linked_list_t* list, unsigned int index1, unsigned int index2)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	} else if (index1 >= list->size || index2 >= list->size){
		printf("Planet out of bounds!\n");
		return;
	}
	dll_node_t *curr1 = dll_get_nth_node(list, index1);
	unsigned int indshield1 = ((planetdata *)curr1->data)->shields->size / 4;
	// folosirea convenienta a functiei upgrade
	upgrade(list, index1, indshield1, -1);
	dll_node_t *curr2 = dll_get_nth_node(list, index2);
	unsigned int indshield2 = (((planetdata *)curr2->data)->shields->size / 4) * 3;
	upgrade(list, index2, indshield2, -1);
	dll_node_t *currshield1 = dll_get_nth_node(((planetdata*)curr1->data)->shields,
		indshield1);
	dll_node_t *currshield2 = dll_get_nth_node(((planetdata*)curr2->data)->shields,
		indshield2);
	int ok = 1;
	char nume[21];
	if (*((int*)currshield1->data) == -1){
		strcpy(nume, ((planetdata *)curr1->data)->name);
		// folosirea convenienta a functiei blh pentru implosion
		blh(list, index1);
		ok = 0;
		printf("The planet %s has imploded.\n", nume);
		((planetdata *)curr2->data)->destroyedplanets++;
	}
	if (*((int*)currshield2->data) == -1){
		strcpy(nume, ((planetdata *)curr2->data)->name);
		printf("The planet %s has imploded.\n", nume);
		if (ok == 1){
			((planetdata *)curr1->data)->destroyedplanets++;
			blh(list, index2);
		} else {
			blh(list, index2 - 1);
		}
	}
}

void freelist(doubly_linked_list_t* list)
{
	if (list == NULL){
		fprintf(stderr, "Error: list doesn't exist\n");
		return;
	}
	while(list->size != 0)
		blh(list, 0);
}

int main(void)
{
	doubly_linked_list_t *list = dll_create(sizeof(planetdata));
	int M;
	scanf("%d", &M);
	char cmd[4];
	for (int i = 0; i < M; i++){
		scanf("%s", cmd);
		if (strcmp("ADD", cmd) == 0){
			char nume[21];
			unsigned int index, nrscuturi;
			scanf("%s%d%d", nume, &index, &nrscuturi);
			add_planet(list, nume, index, nrscuturi);
		} else if (strcmp("BLH", cmd) == 0){
			unsigned int index;
			scanf("%d", &index);
			if (index >= list->size){
				printf("Planet out of bounds!\n");
			} else {
				char nume[21];
				dll_node_t *curr = dll_get_nth_node(list, index);
				strcpy(nume, ((planetdata *)curr->data)->name);
				blh(list, index);
				printf("The planet %s has been eaten by the vortex.\n", nume);
			}
		} else if (strcmp("UPG", cmd) == 0){
			unsigned int index, indexshield;
			int val;
			scanf("%d%d%d", &index, &indexshield, &val);
			upgrade(list, index, indexshield, val);
		} else if (strcmp("EXP", cmd) == 0){
			unsigned int index, val;
			scanf("%d%d", &index, &val);
			expand(list, index, val);
		} else if (strcmp("RMV", cmd) == 0){
			unsigned int index, indexshield;
			scanf("%d%d", &index, &indexshield);
			rmvshield(list, index, indexshield);
		} else if (strcmp("COL", cmd) == 0){
			unsigned int index1, index2;
			scanf("%d%d", &index1, &index2);
			col(list, index1, index2);
		} else if (strcmp("ROT", cmd) == 0){
			unsigned int index, units;
			char s;
			scanf("%d %c %d", &index, &s, &units);
			rotate(list, index, s, units);
		} else if (strcmp("SHW", cmd) == 0){
			unsigned int index;
			scanf("%d", &index);
			showplanet(list, index);
		}
	}
	freelist(list);
	free(list);
	return 0;
}
