#include <interface/portal.h>
#include <memory/memory.h>

PortalOverseer overseer;

void PortalInit()
{
    overseer.portals.first = 0;
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

PortalID RegisterPortal(enum Portal_Category category, ReadFunction read, WriteFunction write, RequestFunction request)
{
    Portal *portal = malloc(sizeof(Portal));
    portal->category = category;
    portal->read = read;
    portal->write = write;
    portal->request = request;
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