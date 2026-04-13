/**
 * @file
 * @author Developful
 * @brief Interface for users and user-owned processes.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

typedef unsigned long UID;

#include <scheduler/task.h>

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
 * @return Gets the current user ID.
 */
UID GetCurrentUser();

/**
 * @brief Gets a user from a process ID.
 *
 * @param pid The process id.
 * @param[out] user The user.
 * @return `true` if the process and user are found, otherwise `false`.
 */
bool GetUserFromProcess(PID pid, User *user);

/**
 * @brief Gets a user from a user ID.
 *
 * @param uid The user ID.
 * @param[out] user The user.
 * @return `true` if found, otherwise `false`.
 */
bool GetUser(UID uid, User *user);

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
