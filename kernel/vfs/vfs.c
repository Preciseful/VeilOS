#include <vfs/vfs.h>
#include <memory/memory.h>
#include <lib/string.h>
#include <lib/printf.h>
#include <memory/memory.h>

VFS *vfs;
List open_entries;

void VFSInit()
{
    open_entries = CreateList(LIST_ARRAY);
    vfs = malloc(sizeof(VFS));
    memset(vfs->roots, 0, sizeof(VEntry) * VFS_MAX_ROOTS);
}

bool AddRoot(const char *path, Portal filesystem)
{
    // check length
    if (!path[0])
        return false;
    if (path[1])
        return false;

    for (unsigned long i = 0; i < VFS_MAX_ROOTS; i++)
    {
        // we use path to detect whether a root is not present
        if (vfs->roots[i].path)
            continue;

        VEntry root;

        root.name_position = 0;
        root.path = path;
        root.portal = filesystem;
        root.cached_entries = CreateList(LIST_LINKED);
        root.distinct_entries = CreateList(LIST_LINKED);

        FSObject fsobject;
        fsobject.fs = root.portal.object;
        fsobject.id = 0;
        fsobject.seek = 0;

        root.request_id = root.portal.request(&fsobject, FS_GET_ROOT_ID, 0);

        vfs->roots[i] = root;
        return true;
    }

    return false;
}

VEntry *find_root(char root_char)
{
    if (root_char == 0)
        return 0;

    for (unsigned long i = 0; i < VFS_MAX_ROOTS; i++)
    {
        if (vfs->roots[i].path[0] == root_char)
            return vfs->roots + i;
    }

    return 0;
}

FileID find_open_file_id(VEntry *entry)
{
    int i = 0;
    for (ListObject *obj = open_entries.first; obj; obj = obj->next, i++)
    {
        if (obj->value == entry)
            return i;
    }

    return -1;
}

FileID check_cache(VEntry *entry, const char *path)
{
    for (ListObject *obj = entry->cached_entries.first; obj; obj = obj->next)
    {
        VEntry *entry = GET_VALUE(obj, VEntry);
        if (strcmp(entry->path, path) != 0)
            continue;

        AddToList(&open_entries, entry);
        return find_open_file_id(entry);
    }

    return -1;
}

FileID OpenFile(const char *const_path)
{
    char root_char = const_path[0];
    VEntry *root = find_root(root_char);
    if (!root)
    {
        LOG("Root %c doesn't exist.\n", root_char);
        return -1;
    }

    unsigned long path_len = strlen(const_path);
    char *path = malloc(path_len + 1);
    memcpy(path, const_path, path_len);
    path[path_len] = 0;

    // TODO: check distinct entries too
    // check cache first
    FileID cache = check_cache(root, path);
    if (cache != -1)
    {
        free(path);
        return cache;
    }

    List substrings = GetSubstrings(path + 1, '/');

    // get entries starting at root
    VEntry *parent = root;
    unsigned long current_total_length = 0;

    for (ListObject *object = substrings.first; object; object = object->next)
    {
        char *substring = GET_VALUE(object, char);
        unsigned long sub_len = strlen(substring);

        FSObject fsobject;
        fsobject.fs = parent->portal.object;
        fsobject.id = parent->request_id;
        fsobject.seek = 0;

        FileRequestID id = parent->portal.request(&fsobject, FS_REQUEST_FILE, substring);
        if (id == 0)
        {
            LOG("File '%s' doesn't exist.\n", substring);
            free(path);
            return -1;
        }

        VEntry *entry = malloc(sizeof(VEntry));
        entry->name_position = current_total_length + 1;
        entry->portal = parent->portal;
        entry->cached_entries.first = 0;
        entry->distinct_entries.first = 0;
        entry->request_id = id;

        current_total_length += sub_len + 1;

        char *entry_path = malloc(current_total_length + 1);
        entry_path[current_total_length] = 0;
        memcpy(entry_path, path, current_total_length);
        entry->path = entry_path;

        parent = entry;
        AddToList(&root->cached_entries, entry);
    }

    FileID cache_id = check_cache(root, path);
    free(path);
    return cache_id;
}