#pragma once

#include <stdbool.h>
#include <interface/partition.h>

#define VOIDELLE_ROOT_CHARACTER '/'
#define VOID_SIZE 512
#define VOIDITE_CONTENT_SIZE (VOID_SIZE - sizeof(unsigned long) * 2)

enum Voidelle_Entry_Flags
{
    VOIDELLE_DIRECTORY = 0b1,
    VOIDELLE_HIDDEN = 0b10,
    VOIDELLE_SYSTEM = 0b100,
    VOIDELLE_DELETED = 0b1000,
};

typedef struct __attribute__((aligned(VOID_SIZE))) __attribute__((packed)) Voidlet
{
    unsigned char identifier[4];
    unsigned long void_size;
    unsigned long voidmap_size;
    unsigned long voidmap;
} Voidlet;

typedef struct __attribute__((packed)) Voidite
{
    unsigned long pos;
    unsigned long next;
    unsigned char data[VOIDITE_CONTENT_SIZE];
} Voidite;

typedef struct __attribute__((aligned(VOID_SIZE))) __attribute__((packed)) Voidelle
{
    unsigned char velle[5];
    unsigned long flags;
    unsigned long name;
    unsigned long content;
    unsigned long content_size;
    unsigned long next;
    unsigned long pos;
    unsigned long create_year;
    unsigned char create_date[5];
    unsigned long modify_year;
    unsigned char modify_date[5];
    unsigned long owner_id;
    unsigned char others_permission;
    unsigned char owner_permission;
} Voidelle;

typedef struct Voidom
{
    Partition partition;
    Voidlet voidlet;
    Voidelle root;
} Voidom;

Voidom *VoidelleInit(Partition partition);
bool GetVoidelleFromPath(Voidom *voidom, const char *path, Voidelle *b_voidelle);
bool MakeVoidelle(Voidom *voidom, const char *path, unsigned long flags, bool recursive);
// we maintain char *path here as it just extracts from the path directly
// for better perf
const char *GetVoidelleName(Voidom *voidom, const char *path);
// we also mantain char *path for remove voidelle as it has the same perf impact
// as if we would pass the voidelle directly
bool RemoveVoidelle(Voidom *voidom, const char *path, bool recursive);
bool WriteToVoidelle(Voidom *voidom, Voidelle *voidelle, char *data, unsigned long size);
bool ReadVoidelleAt(Voidom *voidom, Voidelle voidelle, unsigned long index, Voidite *buf);