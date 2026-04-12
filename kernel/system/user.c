/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <system/user.h>
#include <lib/string.h>
#include <lib/printf.h>
#include <lib/crypt/sha256.h>
#include <drivers/rng.h>

User *admin;
User *logged;

UID get_uid()
{
    UID max = 0;

    User *last = admin;
    while (last)
    {
        if (last->id > max)
            max = last->id;

        last = last->next;
    }

    return max + 1;
}

void register_user(const char *name, const char *password)
{
    char hash[65];
    hash[64] = 0;
    char salt[21];
    SPrintf(salt, "%lx", GetRandom64());

    char *salted_pass = malloc(strlen(salt) + strlen(password) + 1);
    SPrintf(salted_pass, "%s%s", salt, password);

    sha256_easy_hash_hex(salted_pass, strlen(salted_pass), hash);

    User user;
    user.name = malloc(strlen(name) + 1);
    strcpy(user.name, name);
    user.id = get_uid();
    user.next = 0;
    user.owned = 0;
    user.passline = malloc(strlen(salt) + 1 + 64 + 1);
    SPrintf(user.passline, "%s;%s", salt, hash);

    LOG("Registered user: %s, pass: %s\n", name, user.passline);

    if (admin == 0)
    {
        admin = malloc(sizeof(User));
        memcpy(admin, &user, sizeof(User));
    }
    else
    {
        User *last = admin;
        while (last->next)
            last = last->next;

        last->next = malloc(sizeof(User));
        memcpy(last->next, &user, sizeof(User));
    }
}

void UsersInit()
{
    char admin_pass[21];
    SPrintf(admin_pass, "%lx", GetRandom64());

    register_user("admin", admin_pass);
    register_user("guest", "");

    logged = admin->next;
}

UID GetCurrentUser()
{
    return logged->id;
}

bool GetUser(User *user, UID uid)
{
    User *last = admin;
    while (last)
    {
        if (last->id == uid)
        {
            memcpy(user, last, sizeof(User));
            return true;
        }

        last = last->next;
    }

    return false;
}

void UserOwnProcess(User user, PID pid)
{
    if (user.owned == 0)
    {
        user.owned = malloc(sizeof(OwnedProcess));
        user.owned->next = 0;
        user.owned->pid = pid;
        return;
    }

    OwnedProcess *last = user.owned;
    while (last->next)
        last = last->next;

    last->next = malloc(sizeof(OwnedProcess));
    last->next->pid = pid;
    last->next->next = 0;
}

bool CheckOwnsProcess(User user, PID pid)
{
    OwnedProcess *last = user.owned;
    while (last)
    {
        if (last->pid == pid)
            return true;

        last = last->next;
    }

    return false;
}