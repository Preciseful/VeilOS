#ifndef VFS_HPP
#define VFS_HPP

#ifdef __cplusplus
#include <fs/vfs/directory.hpp>
#include <fs/vfs/city.hpp>
extern veil::Directory *root_directory;
extern veil::City *root_city;

extern "C"
{
#endif

    void vfs_init();

#ifdef __cplusplus
}
#endif

#endif