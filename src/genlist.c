/* genlist.c */

#include "genlist.h"
#include <stdio.h>


/* create new empty list, return pointer to list */
struct genlist *genlist_new()
{
	struct genlist *p = (struct genlist*)malloc(sizeof(struct genlist));
	if ( p != NULL) {
		p->first = NULL;
		p->last = NULL;
	}
	return p;
}

/* free list pointer, for nodes use genlist_node_free */
void genlist_free(struct genlist *list)
{
	if (list != NULL) {
		free(list);
	}
}

struct genlist_node *genlist_node_new(void *data)
{
	struct genlist_node *p = (struct genlist_node *)malloc(
						sizeof((struct genlist_node));
	if (p != NULL) {
		p->next = NULL;
		p->prev = NULL;
		p->data = data;
	}

	return p;
}

void *genlist_node_free(struct genlist_node *node)
{
	void *data;
	if (node == NULL) {
		return NULL;
	}
	data = node->data;
	free(node);

	return data;
}

/* add node, return pointer to new node */
struct genlist_node *genlist_add(struct genlist *list,
						 struct genlist_node *node)
{
	if ((list == NULL)||(node == NULL)) {
		return NULL;
	}
	if (list->first == NULL) {
		list->first = node;
		list->last = node;
		node->prev = NULL;
		node->next = NULL;
	} else {
		node->next = NULL;
		node->prev = list->last;
		list->last->next = node;
		list->last = node;
	}
}

/* delete node, return pointer to data */
struct genlist_node *genlist_remove(struct genlist *list,
						 struct genlist_node *node)
{
	if ((list == NULL)||(node == NULL)) {
		return NULL;
	}
		
	if (list->first = node ) {
		list->first = node->next;
	}
	if (list->last = node) {
		list->last = node->prev;
	}
	if (node->prev != NULL) {
		node->prev->next = node->next;
	}
	if (node->next != NULL) {
		node->next->prev = node->prev;
	}
	node->next = NULL;
	node->prev = NULL;
	return node;
}

/* navigation functions for NULL pointer checking */
struct genlist_node *genlist_first(struct genlist *list)
{
	if (list == NULL) {
		return NULL;
	}
	return list->first;
}

struct genlist_node *genlist_last(struct genlist *list)
{
	if (list == NULL) {
		return NULL;
	}
	return list->last;
}

struct genlist_node *genlist_node_next(struct genlist_node *node)
{
	if (node == NULL) {
		return NULL;
	}
	return node->prev;
}

struct genlist_node *genlist_node_prev(struct genlist_node *node)
{
	if (node == NULL) {
		return NULL;
	}
	return node->prev;
}

