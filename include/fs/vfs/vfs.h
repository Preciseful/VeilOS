#ifndef VFS_HPP
#define VFS_HPP

#ifdef __cplusplus
#include <fs/vfs/directory.hpp>
extern veil::Directory *root_directory;

extern "C"
{
#endif

    void vfs_init();

#ifdef __cplusplus
}
#endif

#endif