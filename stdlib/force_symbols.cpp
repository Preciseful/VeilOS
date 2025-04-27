#include "file.hpp"

namespace veil
{
    extern "C" __attribute__((visibility("default"))) void *veil_File_Find = (void *)&File::Find;
}