#include <lib/list.h>
#include <memory/memory.h>

void AddToList(List list, void *object_value)
{
    ListObject *object = malloc(sizeof(ListObject));
    object->value = object_value;
    object->next = 0;

    if (!list.object)
    {
        list.object = object;
        return;
    }

    ListObject *current = list.object;
    while (current->next)
        current = current->next;

    current->next = object;
}

void *RemoveFromList(List list, void *object_value)
{
    ListObject *last = 0;
    ListObject *current = list.object;

    while (current)
    {
        if (current->value == object_value)
        {
            if (last == 0)
                list.object = current->next;
            else
                last->next = current->next;

            free(current);
            return current->value;
        }

        last = current;
        current = current->next;
    }

    return 0;
}