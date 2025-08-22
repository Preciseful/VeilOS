#pragma once

#define GET_VALUE(object, type) ((type *)object->value)

typedef struct ListObject
{
    void *value;
    struct ListObject *next;
} ListObject;

typedef struct List
{
    ListObject *first;
} List;

void AddToList(List *list, void *object_value);
void RemoveKnownFromList(List *list, ListObject *prev, ListObject *current);
void RemoveFromList(List *list, void *object_value);