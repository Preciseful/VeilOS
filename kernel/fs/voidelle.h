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

/**
 * @brief Writes from a buffer to a void in the filesystem.
 *
 * @param voidom The voidom it resides in.
 * @param buf The buffer.
 * @param position The position of the void.
 * @param size The size of the buffer.
 * @return `true` if the size is less than VOID_SIZE, otherwise false.
 */
bool write_void(Voidom voidom, void *buf, uint64_t position, uint64_t size);

/**
 * @brief Reads to a buffer from a void in the filesystem.
 *
 * @param voidom The voidom it resides in.
 * @param buf The buffer.
 * @param position The position of the void.
 * @param size The size of the buffer.
 * @return `true` if the size is less than VOID_SIZE, otherwise false.
 */
bool read_void(Voidom voidom, void *buf, uint64_t position, uint64_t size);

/**
 * @brief Get a voidite from content at a certain index (not offset).
 *
 * @param voidom The voidom it resides in.
 * @param voidelle The voidelle.
 * @param[out] buf The voidite.
 * @param index The index.
 * @return `true` if it exists, otherwise `false`.
 */
bool get_content_voidite_at(Voidom voidom, Voidelle voidelle, Voidite *buf, unsigned long index);

/**
 * @brief Fill the contents links of a voidelle up until a certain amount.
 *
 * @param voidom The voidom it resides in.
 * @param voidelle The voidelle.
 * @param count The amount to fill.
 */
void fill_content_voidites(Voidom voidom, Voidelle *voidelle, unsigned long count);

/**
 * @brief Creates an entry.
 *
 * @param voidom The voidom it resides in.
 * @param[out] buf The entry.
 * @param name The name of the entry.
 * @param flags The flags of the entry.
 * @param owner_id The ID of the owner.
 * @param owner_perm The owner's permissions.
 * @param other_perm Other's permissions.
 * @return Possible errors.
 */
verror_t create_voidelle(Voidom voidom, Voidelle *buf, const char *name, enum Voidelle_Flags flags, uint64_t owner_id, uint8_t owner_perm, uint8_t other_perm);

/**
 * @brief Get an entry's name.
 *
 * @param voidom The voidom it resides in.
 * @param voidelle The entry.
 * @param[out] buf The buffer to put the name in.
 * @return Possible errors.
 */
verror_t get_voidelle_name(Voidom voidom, Voidelle voidelle, char *buf);

/**
 * @brief Reads a certain amount of content from the entry into a buffer, starting from an offset.
 *
 * @param voidom The voidom it resides in.
 * @param voidelle The entry.
 * @param offset The offset in file from which the write starts from.
 * @param[out] buf The buffer to read in.
 * @param size The size of the buffer.
 * @return The amount read.
 */
unsigned long read_voidelle(Voidom voidom, Voidelle voidelle, unsigned long offset, void *buf, unsigned long size);

/**
 * @brief Adds an entry to a parent.
 *
 * @param voidom The voidom it resides in.
 * @param parent The parent to add the entry to.
 * @param voidelle The entry.
 */
void add_voidelle(Voidom voidom, Voidelle *parent, Voidelle *voidelle);

/**
 * @brief Removes an entry from a parent.
 *
 * @param voidom The voidom it resides in.
 * @param parent The parent to add the entry to.
 * @param voidelle The entry.
 * @param invalidate Whether the position should be freed.
 * @return `true` if removing was successful, otherwise `false`.
 */
bool remove_voidelle(Voidom voidom, Voidelle *parent, Voidelle voidelle, bool invalidate);

/**
 * @brief Initialize the Voidelle filesystem from a partition.
 *
 * @param[out] voidom The voidom used to track the filesystem.
 * @param partition The partition the Voidelle filesystem resides in.
 */
void VoidelleFSInit(Voidom *voidom, Partition partition);