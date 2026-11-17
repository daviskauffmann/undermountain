#include "list.h"

#include <malloc.h>

struct list *list_new()
{
    struct list *const list = malloc(sizeof(*list));

    list->data = NULL;
    list->size = 0;
    list->capacity = 0;

    return list;
}

void list_delete(struct list *list)
{
    free(list->data);
    free(list);
}

void *list_get(const struct list *const list, const size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }

    return list->data[index];
}

bool list_contains(const struct list *list, const void *const data)
{
    for (size_t index = 0; index < list->size; index++)
    {
        if (list->data[index] == data)
        {
            return true;
        }
    }

    return false;
}

size_t list_index_of(const struct list *list, const void *const data)
{
    for (size_t index = 0; index < list->size; index++)
    {
        if (list->data[index] == data)
        {
            return index;
        }
    }

    return (size_t)-1;
}

void list_add(struct list *const list, void *const data)
{
    if (list->size == list->capacity)
    {
        list->capacity = list->capacity ? list->capacity * 2 : 1;
        list->data = realloc(list->data, list->capacity * sizeof(*list->data));
    }

    list->data[list->size++] = data;
}

void list_remove(struct list *const list, const void *const data)
{
    for (size_t index = 0; index < list->size; index++)
    {
        if (list->data[index] == data)
        {
            list_remove_at(list, index);
            break;
        }
    }
}

void list_remove_at(struct list *const list, const size_t index)
{
    if (index >= list->size)
    {
        return;
    }

    for (size_t next_index = index; next_index < list->size - 1; next_index++)
    {
        list->data[next_index] = list->data[next_index + 1];
    }

    list->size--;
}

void list_clear(struct list *const list)
{
    list->size = 0;
}
