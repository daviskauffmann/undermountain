#ifndef GAME_LIST_H
#define GAME_LIST_H

#include <stdbool.h>
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

bool list_contains(const struct list *list, const void *data);

size_t list_index_of(const struct list *list, const void *data);

void list_add(struct list *list, void *data);
void list_remove(struct list *list, const void *data);
void list_remove_at(struct list *list, size_t index);

void list_clear(struct list *list);

#endif
