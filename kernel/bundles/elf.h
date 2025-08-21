#pragma once

#include <stdbool.h>
#include <scheduler/task.h>

Task *MakeELFProcess(const char *path, bool add, unsigned long offset);