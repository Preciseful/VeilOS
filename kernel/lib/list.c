#include <lib/list.h>
#include <memory/memory.h>

List CreateList(enum List_Type type)
{
    List list;
    list.first = 0;
    list.type = type;
    return list;
}

void AddToList(List *list, void *object_value)
{
    ListObject *object = malloc(sizeof(ListObject));
    object->value = object_value;
    object->next = 0;

    if (!list->first)
    {
        list->first = object;
        return;
    }

    if (list->type == LIST_LINKED)
    {
        ListObject *current = list->first;
        while (current->next)
            current = current->next;

        current->next = object;
    }
    else if (list->type == LIST_ARRAY)
    {
        ListObject *current = list->first;
        while (current->value)
        {
            if (current->next == 0)
                break;
            current = current->next;
        }

        if (!current->value)
        {
            free(object);
            current->value = object_value;
        }
        else
            current->next = object;
    }
}

void RemoveKnownFromList(List *list, ListObject *prev, ListObject *current)
{
    if (list->type == LIST_ARRAY)
    {
        current->value = 0;
        return;
    }

    if (prev == 0)
        list->first = current->next;
    else
        prev->next = current->next;

    free(current);
}

void RemoveFromList(List *list, void *object_value)
{
    ListObject *last = 0;
    ListObject *current = list->first;

    while (current)
    {
        if (current->value == object_value)
        {
            RemoveKnownFromList(list, last, current);
            return;
        }

        last = current;
        current = current->next;
    }
}

void FreeList(List *list, bool free_values)
{
    while (list->first)
    {
        if (free_values)
            free(list->first->value);

        if (list->type == LIST_ARRAY)
        {
            free(list->first);
            list->first = list->first->next;
        }
        else
            RemoveKnownFromList(list, 0, list->first);
    }
}

void *GetAtIndex(List list, unsigned long index)
{
    unsigned long i = 0;
    for (ListObject *object = list.first; object; object = object->next, i++)
    {
        if (object == 0)
            return 0;

        if (i == index)
            return object->value;
    }

    return 0;
}