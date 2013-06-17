/*
filename:
	genlist.h

purpose:	
	generic linked list for data specific wrappers
*/

#ifndef _GENLIST_H_
#define _GENLIST_H_
#include <stdlib.h>

struct genlist_node {
	struct genlist_node *next;
	struct genlist_node *prev;
	void *data;
};

struct genlist {
	struct genlist_node *first;
	struct genlist_node *last;
};

struct genlist *genlist_new();
void genlist_free(struct genlist *list);

struct genlist_node *genlist_node_new();
void genlist_node_free(struct genlist_node *node);

struct genlist_node *genlist_add(struct genlist *list, 
						struct genlist_node *node);
struct genlist_node *genlist_remove(struct genlist *list,
						 struct genlist_node *node);

struct genlist_node *genlist_first(struct genlist *list);
struct genlist_node *genlist_last(struct genlist *list);
struct genlist_node *genlist_next(struct genlist_node *node);
struct genlist_node *genlist_prev(struct genlist_node *node);

#endif

