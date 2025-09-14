#include "portal.h"
#include "svc.h"

unsigned long portal_write(unsigned long portal, unsigned long portal_id, void *obj, void *buf, unsigned long size)
{
    return svc_call(portal, portal_id, PORTAL_WRITE, (unsigned long)obj, (unsigned long)buf, size, 0, 0, PORTAL_SVC);
}

unsigned long portal_read(unsigned long portal, unsigned long portal_id, void *obj, void *buf, unsigned long size)
{
    return svc_call(portal, portal_id, PORTAL_READ, (unsigned long)obj, (unsigned long)buf, size, 0, 0, PORTAL_SVC);
}

unsigned long portal_request(unsigned long portal, unsigned long portal_id, void *obj, unsigned long code, const void *data)
{
    return svc_call(portal, portal_id, PORTAL_REQUEST, (unsigned long)obj, code, (unsigned long)data, 0, 0, PORTAL_SVC);
}