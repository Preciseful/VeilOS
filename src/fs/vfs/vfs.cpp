#include <fs/vfs/vfs.h>
#include <mm.h>
#include <lib/printf.h>
#include <fs/fat32.hpp>
#include <lib/fork.h>

#include <fs/vfs/directory.hpp>
#include <fs/vfs/fscashier.hpp>

using namespace veil::std;
using namespace veil::fs;
using namespace veil::fs::vfs;

Directory *root = nullptr;

void list_dirs(List<VFSNode *> nodes, bool read, unsigned int level)
{
    for (unsigned long i = 0; i < nodes.Count(); i++)
    {
        for (unsigned int j = 0; j < level; j++)
            printf("| ");

        printf("\"%s\"", nodes[i]->Name());

        if (nodes[i]->Attributes() & 0x10)
        {
            printf("\n");
            Directory *dir = (Directory *)nodes[i];
            list_dirs(dir->GetTotalNodes(), read, level + 1);
        }
        else
        {
            File *file = (File *)nodes[i];

            if (read)
            {
                auto content = file->GetContent();
                printf(" * Reading: \n");
                for (int i = 0; i < 5; i++)
                {
                    printf("%c", content[i]);
                }
            }
            printf("\n");
        }
    }
}

void vfs_init()
{
    FSCashier *cashier = new FSCashier;

    fork((unsigned long)FSCashier::CleanupTask, (unsigned long)cashier);
    FatFS *fs = new FatFS;

    if (!fs->succeded)
    {
        printf("FS failed.\n");
        return;
    }

    auto root_entry = fs->GetEntries(fs->root_cluster);
    Directory *root = new Directory(fs, fs->GetEntry(fs->root_cluster), root_entry, "/");
    root->Preserve();
}