#include <bundles/vex.h>
#include <bundles/elf.h>
#include <vfs/vnode.h>
#include <scheduler/scheduler.h>
#include <lib/printf.h>

bool verify_perms(VexPermissions perms)
{
    if (perms.portal > 255 || perms.portal < -1)
    {
        LOG("Permission's portal is malformed: '%d'.\n", perms.portal);
        return false;
    }

    return true;
}

Vex *MakeVexProcess(const char *path)
{
    Vex *vex = malloc(sizeof(Vex));
    VexPermissions permissions;

    VNode *vnode = OpenFile(path);
    SeekInFile(vnode, 0, SEEK_SET);
    ReadFile(&permissions, sizeof(permissions), vnode);
    CloseFile(vnode);

    vex->path = path;
    vex->permissions = permissions;

    if (!verify_perms(vex->permissions))
        return 0;

    Task *task = MakeELFProcess(path, false, sizeof(VexPermissions));
    if (!task)
        return 0;

    task->bundle = vex;

    AddTask(task);

    return vex;
}