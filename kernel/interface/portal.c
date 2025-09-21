#include <interface/portal.h>
#include <memory/memory.h>

PortalOverseer overseer;

void PortalsInit()
{
    overseer.portals = CreateList(LIST_ARRAY);
    memset(overseer.counts, 0, PORTAL_CATEGORY_COUNT * sizeof(unsigned long));
}

void SetID(Portal *portal)
{
    unsigned long count = overseer.counts[portal->category];
    unsigned char *used = malloc(count);
    memset(used, 0, count);

    for (ListObject *object = overseer.portals.first; object; object = object->next)
    {
        Portal *current_portal = GET_VALUE(object, Portal);
        if (current_portal->category == portal->category)
            used[current_portal->id] = true;
    }

    unsigned long id = 0;
    while (used[id])
        id++;

    free(used);
}

PortalID RegisterPortal(enum Portal_Category category, void *object, ReadFunction read, WriteFunction write, RequestFunction request)
{
    Portal *portal = malloc(sizeof(Portal));
    portal->category = category;
    portal->read = read;
    portal->write = write;
    portal->request = request;
    portal->object = object;
    SetID(portal);

    AddToList(&overseer.portals, portal);

    return portal->id;
}

void UnregisterPortal(enum Portal_Category category, PortalID id)
{
    Portal *portal = 0;

    for (ListObject *object = overseer.portals.first, *last = 0; object; last = object, object = object->next)
    {
        Portal *current_portal = GET_VALUE(object, Portal);
        if (current_portal->category == category && current_portal->id == id)
        {
            portal = current_portal;
            RemoveKnownFromList(&overseer.portals, last, object);
            break;
        }
    }

    free(portal);
}

bool GetPortal(enum Portal_Category category, PortalID id, Portal *portal)
{
    for (ListObject *object = overseer.portals.first; object; object = object->next)
    {
        Portal *current_portal = GET_VALUE(object, Portal);
        if (current_portal->category == category && current_portal->id == id)
        {
            *portal = *current_portal;
            return true;
        }
    }

    return false;
}

SYSCALL_HANDLER(portal)
{
    unsigned long category = sp[0];
    unsigned long id = sp[1];

    Portal portal;
    if (!GetPortal(category, id, &portal))
        return -1;

    switch (sp[2])
    {
        // read
    case 0:
        if (portal.read)
            return portal.read((void *)sp[3], (unsigned char *)sp[4], sp[5]);
        else
            return 0;
        break;

        // write
    case 1:
        if (portal.write)
            return portal.write((void *)sp[3], (unsigned char *)sp[4], sp[5]);
        else
            return 0;

        // request
    case 2:
        if (portal.request)
            return portal.request((void *)sp[3], sp[4], (void *)sp[5]);
        else
            return 0;
    }

    return -1;
}