/**
 * @file
 * @author Developful
 * @brief Interface for users and user-owned processes.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

/**
 * @brief Initializes users in memory.
 */
void UsersInit();

/**
 * @brief Gets a user from a UID.
 *
 * @param[out] user The user.
 * @param uid The UID.
 * @return `true` if found, otherwise `false`.
 */
bool GetUser(User *user, UID uid);

/**
 * @brief Own a process under a user.
 *
 * @param user The user.
 * @param pid The process id.
 */
void UserOwnProcess(User user, PID pid);

/**
 * @brief Check if a user owns a process.
 *
 * @param user The user.
 * @param pid The process id.
 * @return `true` if the user owns it, otherwise `false`.
 */
bool CheckOwnsProcess(User user, PID pid);
