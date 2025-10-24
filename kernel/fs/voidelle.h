#pragma once

#include <interface/partition.h>
#include <stdbool.h>

#define VOID_SIZE 512
#define VOIDITE_CONTENT_SIZE (VOID_SIZE - sizeof(unsigned long) * 2)

enum Voidelle_Entry_Flags
{
    VOIDELLE_DIRECTORY = 0b1,
    VOIDELLE_HIDDEN = 0b10,
    VOIDELLE_SYSTEM = 0b100,
    VOIDELLE_DELETED = 0b1000,
};

typedef struct Voidlet
{
    unsigned char identifier[4];
    unsigned long void_size;
    unsigned long voidmap_size;
    unsigned long voidmap;
} __attribute__((aligned(VOID_SIZE))) __attribute__((packed)) Voidlet;

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
    unsigned long name_size;
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

bool VoidelleInit(Voidom *voidom, Partition partition);
void ReadVoid(Voidom voidom, void *void_section, unsigned long pos);
void UpdateVoidProperties(Voidom voidom, void *void_section, unsigned long pos);
Voidelle CreateVoidelle(Voidom voidom, Voidelle *parent, const char *name, unsigned long flags);
unsigned long WriteToVoidelle(Voidom voidom, Voidelle *voidelle, const void *data, unsigned long size);
void GetVoidelleName(Voidom voidom, Voidelle voidelle, char *buffer);
void RemoveVoidelle(Voidom voidom, Voidelle *parent, Voidelle *voidelle);
bool ReadVoidelleAt(Voidom voidom, Voidelle voidelle, Voidite *voidite, unsigned long index);
