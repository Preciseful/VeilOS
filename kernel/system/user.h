#pragma once

#include <scheduler/task.h>

typedef unsigned long UID;

typedef struct OwnedProcess
{
    struct OwnedProcess *next;
    PID pid;
} OwnedProcess;

typedef struct User
{
    UID id;
    char *name;
    char *passline;
    OwnedProcess *owned;

    struct User *next;
} User;

void UsersInit();
bool GetUser(User *user, UID uid);

void UserOwnProcess(User user, PID pid);
bool CheckOwnsProcess(User user, PID pid);
