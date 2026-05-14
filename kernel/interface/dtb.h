#pragma once

unsigned int GetDTBSize(void *dtb);
unsigned int ParseDTB(void *dtb, const char *path, void **data);