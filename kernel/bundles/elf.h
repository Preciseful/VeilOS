#pragma once

#include <stdbool.h>
#include <scheduler/task.h>

Task *MakeElfProcess(const char *path, bool kernel, int argc, char **argv, char **environment, long pid);