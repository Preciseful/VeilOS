#pragma once
#include <lib/list.h>
#include <stdbool.h>

#define PORTAL_WRITE_FUNCTION(name) unsigned long name(void *obj, unsigned char *buf, unsigned long length)
#define PORTAL_READ_FUNCTION(name) unsigned long name(void *obj, unsigned char *buf, unsigned long length)
#define PORTAL_REQUEST_FUNCTION(name) unsigned long name(void *obj, unsigned long code, const void *data)

typedef unsigned long (*WriteFunction)(void *obj, unsigned char *buf, unsigned long length);
typedef unsigned long (*ReadFunction)(void *obj, unsigned char *buf, unsigned long length);
typedef unsigned long (*RequestFunction)(void *obj, unsigned long code, const void *data);
typedef unsigned long PortalID;

enum Portal_Category
{
    PORTAL_UART,
    PORTAL_FILESYSTEM_DRIVER,
    PORTAL_VFS,
    PORTAL_CATEGORY_COUNT
};

typedef struct PortalOverseer
{
    List portals;
    unsigned long counts[PORTAL_CATEGORY_COUNT];
} PortalOverseer;

typedef struct Portal
{
    enum Portal_Category category;
    PortalID id;
    WriteFunction write;
    ReadFunction read;
    RequestFunction request;
    void *object;
} Portal;

void PortalsInit();
PortalID RegisterPortal(enum Portal_Category category, void *object, ReadFunction read, WriteFunction write, RequestFunction request);
void UnregisterPortal(enum Portal_Category name, PortalID);
bool GetPortal(enum Portal_Category category, PortalID id, Portal *portal);