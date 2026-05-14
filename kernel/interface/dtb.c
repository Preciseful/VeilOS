#include <interface/dtb.h>
#include <lib/string.h>
#include <stdbool.h>

#define FDT_MAGIC 0xD00DFEED
#define FDT_BEGIN_NODE 0x1
#define FDT_END_NODE 0x2
#define FDT_PROP 0x3
#define FDT_NOP 0x4
#define FDT_END 0x9

typedef struct FDTHeader
{
    unsigned int magic;
    unsigned int totalsize;
    unsigned int off_dt_struct;
    unsigned int off_dt_strings;
    unsigned int off_mem_rsvmap;
    unsigned int version;
    unsigned int last_comp_version;
    unsigned int boot_cpuid_phys;
    unsigned int size_dt_strings;
    unsigned int size_dt_struct;
} FDTHeader;

int pathcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 != '/' && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }

    char s1v = *s1, s2v = *s2;

    if (s1v == '/')
        s1v = 0;
    if (s2v == '/')
        s2v = 0;

    return (s1v > s2v) - (s2v > s1v);
}

const char *get_path_component(const char *path, int depth)
{
    int counted_depth = 0;
    while (*path)
    {
        if (*path == '/')
            counted_depth++;

        if (counted_depth == depth)
            return path + 1;

        path++;
    }

    return 0;
}

unsigned int GetDTBSize(void *dtb)
{
    FDTHeader *header = (FDTHeader *)dtb;
    if (__builtin_bswap32(header->magic) != FDT_MAGIC)
        return 0;

    return __builtin_bswap32(header->totalsize);
}

unsigned int ParseDTB(void *dtb, const char *path, void **data)
{
    FDTHeader *header = (FDTHeader *)dtb;

    if (__builtin_bswap32(header->magic) != FDT_MAGIC)
        return 0;

    unsigned int *ptr = (unsigned int *)(dtb + __builtin_bswap32(header->off_dt_struct));
    const char *strtab = (const char *)(dtb + __builtin_bswap32(header->off_dt_strings));
    int depth = 0;
    int component_depth = 0;
    bool active = false;

    while (1)
    {
        unsigned int token = __builtin_bswap32(*ptr++);
        if (depth != component_depth)
            active = false;

        switch (token)
        {
        case FDT_BEGIN_NODE:
            const char *node_name = (const char *)ptr;
            unsigned long name_len = strlen(node_name);
            ptr += ((name_len + 4) & ~3) / 4;

            const char *component = get_path_component(path, depth);
            if (component && pathcmp(component, node_name) == 0)
            {
                component_depth = depth + 1;
                active = true;
            }

            depth++;
            break;

        case FDT_PROP:
            unsigned int len = __builtin_bswap32(*ptr++);
            unsigned int nameoff = __builtin_bswap32(*ptr++);
            const char *propname = strtab + nameoff;

            if (active)
            {
                const char *target_propname = get_path_component(path, depth);
                if (target_propname && pathcmp(target_propname, propname) == 0)
                {
                    *data = (void *)ptr;
                    return len;
                }
            }

            ptr += (len + 3) / 4;
            break;

        case FDT_END_NODE:
            depth--;
            break;

        case FDT_END:
            return 0;
        }
    }
}