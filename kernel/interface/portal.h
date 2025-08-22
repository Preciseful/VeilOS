#pragma once
#include <lib/list.h>

typedef void (*WriteFunction)(unsigned char *bytes);
typedef void (*ReadFunction)(unsigned char *buf, unsigned long length);
typedef unsigned long (*RequestFunction)(unsigned long code, void *data);
typedef long PortalID;

enum Portal_Category
{
    PORTAL_UART,
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
} Portal;

void PortalInit();
PortalID RegisterPortal(enum Portal_Category category, ReadFunction read, WriteFunction write, RequestFunction request);
void UnregisterPortal(enum Portal_Category name, PortalID);