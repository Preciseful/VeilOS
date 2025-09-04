#pragma once

#include <stdbool.h>

#define GET_VALUE(object, type) ((type *)object->value)

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