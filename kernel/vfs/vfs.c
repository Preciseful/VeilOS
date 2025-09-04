#include <vfs/vfs.h>
#include <memory/memory.h>
#include <lib/string.h>
#include <lib/printf.h>
#include <memory/memory.h>

VFS *vfs;
List open_entries;

#define VFS_REQUEST_SEEK 0

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

VEntry *find_open_file(FileID id)
{
    if (id == -1)
        return 0;

    int i = 0;
    for (ListObject *obj = open_entries.first; obj; obj = obj->next, i++)
    {
        if (i != id)
            continue;

        return obj->value;
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

VEntry *get_distinct_entry(VEntry *root, const char *path)
{
    for (ListObject *obj = root->distinct_entries.first; obj; obj = obj->next)
    {
        VEntry *entry = GET_VALUE(obj, VEntry);
        unsigned entry_path_len = strlen(entry->path);

        if (memcmp(entry->path, path, entry_path_len) != 0)
            continue;
        if (path[entry_path_len + 1] != '/')
            continue;

        return entry;
    }

    return 0;
}

static PORTAL_READ_FUNCTION(read_portal_vfs)
{
    FileID id = *((FileID *)obj);
    return ReadFromFile(id, buf, length);
}

static PORTAL_WRITE_FUNCTION(write_portal_vfs)
{
    FileID id = *((FileID *)obj);
    return WriteInFile(id, buf, length);
}

static PORTAL_REQUEST_FUNCTION(request_portal_vfs)
{
    FileID id = *((FileID *)obj);

    switch (code)
    {
    case VFS_REQUEST_SEEK:
        SeekInFile(id, *((unsigned long *)data));
        return 0;
    }

    return 0;
}

void VFSInit()
{
    open_entries = CreateList(LIST_ARRAY);
    vfs = malloc(sizeof(VFS));
    memset(vfs->roots, 0, sizeof(VEntry) * VFS_MAX_ROOTS);

    RegisterPortal(PORTAL_VFS, vfs, read_portal_vfs, write_portal_vfs, request_portal_vfs);
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
        root.seek = 0;

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

bool AddDistinctEntry(const char *path, Portal filesystem)
{
    char root_char = path[0];
    VEntry *root = find_root(root_char);
    if (!root)
    {
        LOG("Root %c doesn't exist.\n", root_char);
        return false;
    }

    VEntry *entry = malloc(sizeof(VEntry));
    entry->name_position = 1;
    entry->portal = filesystem;
    entry->cached_entries = CreateList(LIST_LINKED);
    entry->distinct_entries = CreateList(LIST_LINKED);
    entry->seek = 0;
    entry->path = path;

    FSObject fsobject;
    fsobject.fs = entry->portal.object;
    fsobject.id = 0;
    fsobject.seek = 0;

    entry->request_id = entry->portal.request(&fsobject, FS_GET_ROOT_ID, 0);

    AddToList(&root->distinct_entries, entry);
    return true;
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

    // check cache first
    FileID cache = check_cache(root, path);
    if (cache != -1)
    {
        free(path);
        return cache;
    }

    unsigned long substring_skip = 1;
    VEntry *distinct = get_distinct_entry(root, path);
    if (distinct)
    {
        root = distinct;
        substring_skip = strlen(distinct->path) + 1;
    }

    List substrings = GetSubstrings(path + substring_skip, '/');

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
            FreeList(&substrings, true);
            return -1;
        }

        VEntry *entry = malloc(sizeof(VEntry));
        entry->name_position = current_total_length + 1;
        entry->portal = parent->portal;
        entry->cached_entries = CreateList(LIST_LINKED);
        entry->distinct_entries = CreateList(LIST_LINKED);
        entry->request_id = id;
        entry->seek = 0;

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
    FreeList(&substrings, true);
    return cache_id;
}

void SeekInFile(FileID id, unsigned long seek)
{
    VEntry *entry = find_open_file(id);
    if (!entry)
        return;

    entry->seek = seek;
}

unsigned long ReadFromFile(FileID id, void *buf, unsigned long size)
{
    VEntry *entry = find_open_file(id);
    if (!entry)
    {
        LOG("Reading from file %lu (%s) failed.", id, entry->path);
        return 0;
    }

    FSObject obj;
    obj.fs = entry->portal.object;
    obj.seek = entry->seek;
    obj.id = entry->request_id;

    return entry->portal.read(&obj, buf, size);
}

unsigned long WriteInFile(FileID id, void *buf, unsigned long size)
{
    VEntry *entry = find_open_file(id);
    if (!entry)
        return 0;

    FSObject obj;
    obj.fs = entry->portal.object;
    obj.seek = 0;
    obj.id = entry->request_id;

    return entry->portal.write(&obj, buf, size);
}

void CloseFile(FileID id)
{
    if (id == -1)
        return;

    int i = 0;
    for (ListObject *obj = open_entries.first; obj; obj = obj->next, i++)
    {
        if (i != id)
            continue;

        RemoveKnownFromList(&open_entries, 0, obj);
    }
}