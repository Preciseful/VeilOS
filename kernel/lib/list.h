#pragma once

#define GET_VALUE(list, type) ((type *)list->value)

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
void RemoveFromList(List *list, void *object_value);