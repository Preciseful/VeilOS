#pragma once

#include <stdbool.h>

#define GET_VALUE(object, type) ((type *)object->value)
#define LIST_FOREACH(type, name, list)                                                                           \
    ListObject *list_foreach_object = list.first;                                                                \
    for (unsigned long index = 0; list_foreach_object; list_foreach_object = list_foreach_object->next, index++) \
        for (type *name = GET_VALUE(list_foreach_object, type); name; name = 0)

enum List_Type
{
    LIST_ARRAY,
    LIST_LINKED
};

typedef struct ListObject
{
    void *value;
    struct ListObject *next;
} ListObject;

typedef struct List
{
    enum List_Type type;
    ListObject *first;
} List;

List CreateList(enum List_Type type);
void AddToList(List *list, void *object_value);
void RemoveKnownFromList(List *list, ListObject *prev, ListObject *current);
void RemoveFromList(List *list, void *object_value);
void FreeList(List *list, bool free_values);
void *GetAtIndex(List list, unsigned long index);