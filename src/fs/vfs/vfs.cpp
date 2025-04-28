#include <fs/vfs/vfs.h>
#include <mm.h>
#include <lib/printf.h>
#include <fs/fat32.hpp>
#include <lib/fork.h>

#include <fs/vfs/directory.hpp>
#include <fs/vfs/city.hpp>

#include <lib/string.hpp>

using namespace veil::std;
using namespace veil;

Directory *root_directory = nullptr;
City *root_city = nullptr;

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

void cities_init(City *city, List<VFSNode *> nodes)
{
    for (unsigned long i = 0; i < nodes.Count(); i++)
    {
        if (nodes[i]->Attributes() & 0x10)
        {
            City *newcity = new City(rtrim(nodes[i]->Name()), DirectoryType, nodes[i]->GetEntry(), nodes[i]->GetFS());
            city->AddSubcity(newcity);
            printf("Added subcity %s to city %s\n", newcity->GetName(), city->GetName());
            cities_init(newcity, ((Directory *)nodes[i])->GetTotalNodes());
        }
        else
        {
            City *newcity = new City(rtrim(nodes[i]->Name()), FileType, nodes[i]->GetEntry(), nodes[i]->GetFS());
            city->AddSubcity(newcity);
            printf("Added subcity %s to city %s\n", newcity->GetName(), city->GetName());
        }
    }
}

void vfs_init()
{
    auto fs = new FatFS;

    if (!fs->succeded)
    {
        printf("FS failed.\n");
        return;
    }

    auto root_entries = fs->GetEntries(fs->root_cluster);
    auto root_entry = fs->GetEntry(fs->root_cluster);

    root_city = new City("/", DirectoryType, root_entry, fs);
    root_directory = new Directory(fs, root_entry, root_entries, "/");

    cities_init(root_city, root_directory->GetTotalNodes());
    printf("\n");
}