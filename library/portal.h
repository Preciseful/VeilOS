#pragma once

#define PORTAL_UART 0
#define PORTAL_VFS 2

#define PORTAL_READ 0
#define PORTAL_WRITE 1
#define PORTAL_REQUEST 2

#define PORTAL_SVC 0

unsigned long portal_write(unsigned long portal, unsigned long portal_id, void *obj, void *buf, unsigned long size);
unsigned long portal_read(unsigned long portal, unsigned long portal_id, void *obj, void *buf, unsigned long size);
unsigned long portal_request(unsigned long portal, unsigned long portal_id, void *obj, unsigned long code, const void *data);