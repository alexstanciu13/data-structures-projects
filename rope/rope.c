#include "./rope.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "utils.h"

#define EMPTY ""

char *strdup(const char *s);

RopeNode* makeRopeNode(const char* str) {
	RopeNode *node = malloc(sizeof(*node));
	DIE(node == NULL, "malloc failed");
	node->left = NULL;
	node->right = NULL;
	node->str = str;
	node->weight = strlen(str);
	return node;
}

RopeTree* makeRopeTree(RopeNode* root) {
	RopeTree *tree = malloc(sizeof(*tree));
	DIE(tree == NULL, "malloc failed");
	tree->root = root;
	return tree;
}

RopeNode* copyRopeNode1(RopeNode* rn) {
	if (!rn)
		return NULL;

	RopeNode* new_rn = makeRopeNode(strdup(rn->str));
	new_rn->weight = rn->weight;
	new_rn->left = copyRopeNode1(rn->left);
	new_rn->right = copyRopeNode1(rn->right);
	return new_rn;
}

RopeTree* copyRopeTree1(RopeTree* rt) {
	if (!rt)
		return NULL;

	RopeTree* new_rt = makeRopeTree(copyRopeNode1(rt->root));
	return new_rt;
}

void printRopeNode(RopeNode* rn) {
	if (!rn)
		return;

	if (!(rn->left) && !(rn->right)) {
		printf("%s", rn->str);
		return;
	}

	printRopeNode(rn->left);
	printRopeNode(rn->right);
}

void printRopeTree(RopeTree* rt) {
	if (rt && rt->root) {
		printRopeNode(rt->root);
		printf("%s", "\n");
	}
}

void debugRopeNode(RopeNode* rn, int indent) {
	if (!rn)
		return;

	for (int i = 0; i < indent; ++i)
		printf("%s", " ");

	if (!strcmp(rn->str, EMPTY))
		printf("# %d\n", rn->weight);
	else
		printf("%s %d\n", rn->str, rn->weight);

	debugRopeNode(rn->left, indent+2);
	debugRopeNode(rn->right, indent+2);
}



int getTotalWeight(RopeNode* rt) {
	if (!rt)
		return 0;

	return rt->weight + getTotalWeight(rt->right);
}


RopeTree* concat(RopeTree* rt1, RopeTree* rt2) {
	RopeNode *root = makeRopeNode(strdup(EMPTY));
	RopeTree *tree = makeRopeTree(root);
	root->weight = getTotalWeight(rt1->root);
	root->left = rt1->root;
	root->right = rt2->root;
	return tree;
}

char indexRope(RopeTree* rt, int idx) {
	RopeNode *node = rt->root;
	while (node->left != NULL || node->right != NULL){
		if (idx < node->weight){
			node = node->left;
		} else {
			idx -= node->weight;
			node = node->right;
		}
	}
	return node->str[idx];
}


char* search(RopeTree* rt, int start, int end) {
	char *str = malloc(end - start + 1);
	DIE(str == NULL, "malloc failed");
	for (int i = start; i < end; i++){
		str[i - start] = indexRope(rt, i);
	}
	str[end - start] = '\0';
	return str;
}

// functie care actualizeaza weight-urile dupa split
void repare(RopeNode *node){
	if (node == NULL)
		return;
	if (node->left == NULL && node->right == NULL)
		return;
	repare(node->left);
	repare(node->right);
	node->weight = getTotalWeight(node->left);
}

RopeTree *break_leaf(RopeTree *rtcopy, int idx) {
	RopeNode *node = rtcopy->root;
	// cautarea predecesorului frunzei care poate
	// fi taiata in functie de index-ul primit
	while (node->left != NULL || node->right != NULL){
		if (idx == node->weight){
			return rtcopy;
		}
		if (idx < node->weight){
			if (node->left->left == NULL && node->left->right == NULL){
				break;
			}
			node = node->left;
		} else {
			if (node->right->left == NULL && node->right->right == NULL){
				break;
			}
			idx -= node->weight;
			node = node->right;
		}
	}
	// se formeaza cele doua frunze prin impartirea celei
	// initiale si se concateneaza
	if (idx < node->weight){
		if (idx > 0){
			RopeNode *new_node_empty = makeRopeNode(strdup(EMPTY));
			new_node_empty->weight = idx;

			char *string1, *string2;
			string1 = malloc(idx + 1);
			DIE(string1 == NULL, "malloc failed");
			string2 = malloc(node->weight - idx + 1);
			DIE(string2 == NULL, "malloc failed");
			strncpy(string1, (char *)node->left->str, idx);
			string1[idx] = '\0';
			strcpy(string2, (char *)node->left->str + idx);

			RopeNode *new_node1 = makeRopeNode(strdup((const char *)string1));
			RopeNode *new_node2 = makeRopeNode(strdup((const char *)string2));

			new_node_empty->left = new_node1;
			new_node_empty->right = new_node2;

			free((void *)node->left->str);
			free(node->left);
			node->left = new_node_empty;
			free(string1);
			free(string2);
			return rtcopy;
		}
	} else {
		idx -= node->weight;
		RopeNode *new_node_empty = makeRopeNode(strdup(EMPTY));
		new_node_empty->weight = idx;

		char *string1, *string2;
		string1 = malloc(idx + 1);
		DIE(string1 == NULL, "malloc failed");
		string2 = malloc(strlen(node->right->str) - idx + 1);
		DIE(string2 == NULL, "malloc failed");
		strncpy(string1, (char *)node->right->str, idx);
		string1[idx] = '\0';
		strcpy(string2, (char *)node->right->str + idx);

		RopeNode *new_node1 = makeRopeNode(strdup((const char *)string1));
		RopeNode *new_node2 = makeRopeNode(strdup((const char *)string2));

		new_node_empty->left = new_node1;
		new_node_empty->right = new_node2;

		free((void *)node->right->str);
		free(node->right);
		node->right = new_node_empty;
		free(string1);
		free(string2);
		return rtcopy;
	}
	return NULL;
}

SplitPair split(RopeTree* rt, int idx) {
	SplitPair sp;

	// lucram pe o copie a rope-ului
	RopeTree *rtcopy = copyRopeTree1(rt);

	RopeNode *node = rtcopy->root;

	// cazul in care nu se face nicio taiere
	if (idx == 0){
		sp.left = makeRopeNode(strdup(EMPTY));
		sp.right = rtcopy->root;
		free(rtcopy);
		return sp;
	}
	// cazul in care se face o singura taiere,
	// la root->right
	if (node->weight == idx){
		sp.right = node->right;
		node->right = NULL;
		sp.left = node;
		free(rtcopy);
		return sp;
	}
	// verificam intai daca trebuie impartita o frunza
	rtcopy = break_leaf(rtcopy, idx);
	node = rtcopy->root;

	// vector care stocheaza nodurile ce vor fi concatenate
	// pentru a forma rope-ul din sp.right

	RopeNode **vector = (RopeNode **)malloc(100 * sizeof(RopeNode*));
	DIE(vector == NULL, "malloc failed");
	int i = 0;
	while (node->left != NULL || node->right != NULL){
		// daca se taie tot subarborele drept din node si se continua
		// parcurgerea sau nu
		if (idx <= node->weight){
			if (node->right != NULL){
				RopeNode *new_node = makeRopeNode(strdup(node->right->str));
				new_node->weight = node->right->weight;
				new_node->left = node->right->left;
				new_node->right = node->right->right;
				vector[i++] = new_node;
				free((void *)node->right->str);
				free(node->right);
				node->right = NULL;
			}
			if(idx < node->weight)
				node = node->left;
			else
				break;
		} else {
			idx -= node->weight;
			node = node->right;
		}
	}
	// cazul in care avem un singur nod in vector
	int nrconcat = i;
	if (nrconcat == 1){
		RopeNode *new_root = makeRopeNode(strdup(EMPTY));
		new_root->weight = vector[0]->weight;
		new_root->left = vector[0];
		RopeTree *tree1 = makeRopeTree(new_root);
		sp.left = rtcopy->root;
		sp.right = tree1->root;
		free(rtcopy);
		free(tree1);
		free(vector);
		return sp;
	}

	RopeTree *tree1  = makeRopeTree(vector[nrconcat - 1]);
	RopeTree *tree1copy = tree1;
	RopeTree *tree2;
	// concatenarea rope-urilor din vector
	for (int j = nrconcat - 2; j >= 0; j--){
		tree2 = makeRopeTree(vector[j]);
		tree1copy = tree1;
		tree1 = concat(tree1, tree2);
		free(tree2);
		free(tree1copy);
	}
	// actualizarea weight-urilor celor doua rope-uri
	repare(rtcopy->root);
	repare(tree1->root);

	sp.right = tree1->root;
	free(tree1);
	sp.left = rtcopy->root;
	free(rtcopy);
	free(vector);
	return sp;
}

RopeTree* insert(RopeTree* rt, int idx, const char* str) {
	SplitPair sp = split(rt, idx);
	RopeTree *tree1 = makeRopeTree(sp.left);
	RopeTree *tree2 = makeRopeTree(sp.right);
	RopeNode *new_node = makeRopeNode(str);
	RopeTree *tree3 = makeRopeTree(new_node);
	RopeTree *tree4 = concat(tree1, tree3);
	RopeTree *tree5 = concat(tree4, tree2);
	free(tree1);
	free(tree2);
	free(tree3);
	free(tree4);
	return tree5;
}

RopeTree* delete(RopeTree* rt, int start, int len) {
// daca start+len iese din rope, scadem len pana ajunge la
// indexul maxim posibil
	while (start + len > getTotalWeight(rt->root))
		len--;
	SplitPair sp = split(rt, start);
	RopeTree *tree1 = makeRopeTree(sp.left);
	RopeTree *tree2 = makeRopeTree(sp.right);
	SplitPair sp1 = split(tree2, len);
	RopeTree *tree3 = makeRopeTree(sp1.right);
	RopeTree *tree4 = concat(tree1, tree3);
	free(tree1);
	free(tree2);
	free(tree3);
	return tree4;
}
