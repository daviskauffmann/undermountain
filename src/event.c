#include "event.h"

#include <stddef.h>
#include <stdio.h>

static struct event_listener *listeners;
static unsigned int dispatch_depth;

bool event_subscribe(struct event_listener *const listener)
{
    if (!listener || !listener->receive || dispatch_depth)
    {
        fprintf(stderr, "Cannot subscribe an invalid listener or change listeners during dispatch.\n");
        return false;
    }

    for (struct event_listener *current = listeners; current; current = current->next)
    {
        if (current == listener)
        {
            fprintf(stderr, "Event listener is already subscribed.\n");
            return false;
        }
    }

    listener->next = listeners;
    listeners = listener;

    return true;
}

bool event_unsubscribe(struct event_listener *const listener)
{
    if (!listener || dispatch_depth)
    {
        fprintf(stderr, "Cannot unsubscribe an invalid listener or change listeners during dispatch.\n");
        return false;
    }

    for (struct event_listener **current = &listeners; *current; current = &(*current)->next)
    {
        if (*current == listener)
        {
            *current = listener->next;
            listener->next = NULL;
            return true;
        }
    }

    fprintf(stderr, "Event listener is not subscribed.\n");
    return false;
}

void event_emit(const struct game_event *const event)
{
    if (!event || event->type < 0 || event->type >= NUM_EVENT_TYPES)
    {
        fprintf(stderr, "Cannot emit an invalid event.\n");
        return;
    }

    dispatch_depth++;
    for (struct event_listener *listener = listeners; listener; listener = listener->next)
    {
        listener->receive(event, listener->context);
    }
    dispatch_depth--;
}

bool event_reset(void)
{
    if (dispatch_depth)
    {
        fprintf(stderr, "Cannot reset event listeners during dispatch.\n");
        return false;
    }

    while (listeners)
    {
        struct event_listener *const listener = listeners;
        listeners = listener->next;
        listener->next = NULL;
    }

    return true;
}
