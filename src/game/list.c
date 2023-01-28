#include "list.h"

#include <assert.h>
#include <malloc.h>

struct list *list_new()
{
    struct list *const list = malloc(sizeof(struct list));
    assert(list);

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

bool list_contains(const struct list *list, void *data)
{
    for (size_t i = 0; i < list->size; ++i)
    {
        if (list->data[i] == data)
        {
            return true;
        }
    }

    return false;
}

size_t list_index_of(const struct list *list, void *data)
{
    for (size_t i = 0; i < list->size; ++i)
    {
        if (list->data[i] == data)
        {
            return i;
        }
    }

    return (size_t)-1;
}

void list_add(struct list *const list, void *const data)
{
    if (list->size == list->capacity)
    {
        list->capacity = list->capacity ? list->capacity * 2 : 1;
        list->data = realloc(list->data, list->capacity * sizeof(void *));
        assert(list->data);
    }

    list->data[list->size++] = data;
}

void list_remove(struct list *const list, const void *const data)
{
    for (size_t i = 0; i < list->size; ++i)
    {
        if (list->data[i] == data)
        {
            list_remove_at(list, i);
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

    for (size_t i = index; i < list->size - 1; ++i)
    {
        list->data[i] = list->data[i + 1];
    }

    --list->size;
}

void list_clear(struct list *const list)
{
    list->size = 0;
}
