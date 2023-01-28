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

void *list_get(const struct list *const list, const size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }

    return list->data[index];
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

void list_delete(struct list *list)
{
    free(list->data);
    free(list);
}
