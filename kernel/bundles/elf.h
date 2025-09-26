#pragma once

#include <stdbool.h>
#include <scheduler/task.h>

Task *MakeElfProcess(const char *path, int argc, char **argv);