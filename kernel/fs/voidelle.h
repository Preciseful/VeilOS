#pragma once

#define VOIDITE_CONTENT_SIZE (512 - sizeof(unsigned long) * 2)

enum Voidelle_Entry_Flags
{
    VOIDELLE_DIRECTORY = 0b1,
    VOIDELLE_HIDDEN = 0b10,
    VOIDELLE_SYSTEM = 0b100,
    VOIDELLE_DELETED = 0b1000,
};

typedef struct voidlet
{
    unsigned char identifier[4];
    unsigned long voidelle_size;
    unsigned long voidmap_size;
    unsigned long voidmap;
} __attribute__((packed)) voidlet_t;

typedef struct voidite
{
    unsigned long pos;
    unsigned long next;
    unsigned char data[VOIDITE_CONTENT_SIZE];
} __attribute__((packed)) voidite_t;

typedef struct voidelle
{
    unsigned char velle[5];
    unsigned long flags;
    unsigned long name;
    unsigned long content;
    unsigned long content_size;
    unsigned long next;
    unsigned long pos;
    unsigned long create_year;
    // MONTH | DAY | HOUR | MINUTE | SECOND
    unsigned char create_date[5];
    unsigned long modify_year;
    unsigned char modify_date[5];
    unsigned long owner_id;
    unsigned char others_permission[3];
    unsigned char owner_permission[3];
} __attribute__((packed)) voidelle_t;

typedef struct voidom
{
    voidelle_t *root;
    voidlet_t *voidlet;
    unsigned long partition_seek;
} __attribute__((packed)) voidom_t;

voidom_t *voidelle_init();
voidelle_t *create_voidelle(voidom_t *vdom, unsigned long parent_pos, unsigned long flags, unsigned int *exit_code, char *name);
char *voidelle_read_at(voidom_t *vdom, voidelle_t velle, unsigned long pos);
void voidelle_write(voidom_t *vdom, voidelle_t *velle, char *write_buf, unsigned long write_buf_size);
unsigned long get_voidelle_entries(voidom_t *vdom, unsigned long parent_pos, voidelle_t **bnodes);
char *get_voidelle_name(voidom_t *vdom, unsigned long name_pos);