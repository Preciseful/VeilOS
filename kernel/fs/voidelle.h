#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <interface/partition.h>

#define VOID_SIZE 512
#define VOIDITE_CONTENT_SIZE (VOID_SIZE - sizeof(uint64_t) * 2)

// All void positions are partition-relative

typedef uint64_t verror_t;
// disk_t should be changed to your proper disk struct
typedef Partition disk_t;

enum Voidelle_Errors
{
    SUCCESS,
    INVALID_DISK,
    UNKNOWN_ERROR,
    FILE_IS_DIRECTORY,
};

enum Voidelle_Flags
{
    VOIDELLE_DIRECTORY = 0x1,
    VOIDELLE_HIDDEN = 0x2,
    VOIDELLE_SYSTEM = 0x4,
    // not used, but specified
    VOIDELLE_INVALID = 0x8,
};

enum Voidelle_Permission_Flags
{
    VOIDELLE_PERMISSION_EXECUTE = 0x1,
    VOIDELLE_PERMISSION_READ = 0x2,
    VOIDELLE_PERMISSION_WRITE = 0x4,
    VOIDELLE_PERMISSION_ALL = 0x7,
};

typedef struct __attribute__((packed)) Voidelle
{
    // Check for 'VELLE' to ensure integrity
    uint8_t header[5];
    uint64_t flags;
    uint64_t name_voidelle, name_voidelle_size;
    uint64_t content_voidelle, content_voidelle_size;
    uint64_t next_voidelle;
    uint64_t position;
    uint64_t creation_seconds;
    uint64_t modification_seconds;
    uint64_t access_seconds;
    uint64_t owner_id;
    uint8_t other_permission;
    uint8_t owner_permission;
} Voidelle;

_Static_assert(sizeof(Voidelle) <= VOID_SIZE, "Void size must be greater than a voidelle's size.");

typedef struct __attribute__((packed)) Voidlet
{
    // Check for 'VOID' to ensure integrity or recognize filesystem
    uint8_t header[4];
    uint64_t void_size;
    uint64_t voidmap_size;
    uint64_t voidmap;
} Voidlet;

typedef struct __attribute__((packed)) Voidite
{
    uint64_t position;
    uint64_t next_voidite;
    uint8_t data[VOIDITE_CONTENT_SIZE];
} Voidite;

// Simply a helper struct of useful metadata, it is not placed on the disk
typedef struct __attribute__((packed)) Voidom
{
    disk_t disk;
    Voidlet voidlet;
    Voidelle root;
} Voidom;

bool write_void(Voidom voidom, void *buf, uint64_t position, uint64_t size);
bool read_void(Voidom voidom, void *buf, uint64_t position, uint64_t size);
uint64_t populate_voidite_data(Voidom voidom, Voidite *first_voidite_buf, const void *data, uint64_t size);
void clear_voidites_after(Voidom voidom, Voidite *start);
void clear_voidelle_content(Voidom voidom, Voidelle *voidelle);
void clear_voidelle_name(Voidom voidom, Voidelle *voidelle);
void fill_content_voidites(Voidom voidom, Voidelle *voidelle, unsigned long count);
void fill_name_voidites(Voidom voidom, Voidelle *voidelle, unsigned long count);
uint64_t get_free_void(Voidom voidom);
verror_t create_voidlet(Voidom *voidom);
verror_t create_voidelle(Voidom voidom, Voidelle *buf, const char *name, enum Voidelle_Flags flags, uint64_t owner_id, uint8_t owner_perm, uint8_t other_perm);
verror_t get_voidelle_name(Voidom voidom, Voidelle voidelle, char *buf);
bool get_content_voidite_at(Voidom voidom, Voidelle voidelle, Voidite *buf, unsigned long index);
bool get_name_voidite_at(Voidom voidom, Voidelle voidelle, Voidite *buf, unsigned long index);
unsigned long read_voidelle(Voidom voidom, Voidelle voidelle, unsigned long seek, void *buf, unsigned long size);
void add_voidelle(Voidom voidom, Voidelle *parent, Voidelle *voidelle);
void add_voidelle_with_check(Voidom voidom, Voidelle *parent, Voidelle voidelle);

// swaps = 0x0, do not swap anything but the voidelles
// swaps = 0x1, swap names
// swaps = 0x2, swap contents
// swaps = 0x3, swap both
void swap_voidelles(Voidom voidom, Voidelle *first, Voidelle *second, int swaps);

bool remove_voidelle(Voidom voidom, Voidelle *parent, Voidelle voidelle, bool invalidate);

void VoidelleFSInit(Voidom *voidom, Partition partition);