#ifndef GAME_LIST_H
#define GAME_LIST_H

#include <stddef.h>

struct list
{
    void **data;
    size_t size;
    size_t capacity;
};

struct list *list_new();
void list_delete(struct list *list);

void *list_get(const struct list *list, size_t index);

void list_add(struct list *list, void *data);

#endif
