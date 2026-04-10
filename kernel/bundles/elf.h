/**
 * @file
 * @author Developful
 * @brief Interface for handling ELF files.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <scheduler/task.h>

/**
 * @brief Creates a process from an ELF file.
 *
 * Reads an ELF file from a path and makes it into a running task,
 * automatically being added to scheduling.
 *
 * @param path The path of the ELF file.
 * @param kernel Whether the process is EL1 (kernel) or EL0 (user).
 * @param pid The process id of the task. Passing -1 sets the process id automatically (recommended).
 * @param argc The arguments count to be passed.
 * @param argv The arguments values to be passed.
 * @param envargc The environment counts to be passed. Must be a multiple of 2.
 * @param envargv The environment values to be passed. Must have a count multiple of 2.
 * @return Pointer to the process.
 */
Task *MakeElfProcess(const char *path, bool kernel, PID pid, int argc, char **argv, int envargc, char **envargv);