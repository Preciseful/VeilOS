#include <fs/vfs/vfs.h>
#include <mm.h>
#include <lib/printf.h>
#include <fs/fat32.hpp>
#include <lib/fork.h>

#include <fs/vfs/directory.hpp>
#include <fs/vfs/city.hpp>

#include <lib/string.h>

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

void cities_init(City *parent_city, List<VFSNode *> nodes, unsigned int parent_cluster)
{
    for (unsigned long i = 0; i < nodes.Count(); i++)
    {
        nodes[i]->entry.parent_cluster = parent_cluster;
        if (nodes[i]->Attributes() & 0x10)
        {
            City *newcity = new City(parent_city, rtrim(nodes[i]->Name()), DirectoryType, nodes[i]->entry, nodes[i]->GetFS());
            parent_city->AddSubcity(newcity);
            nodes[i]->OwnCity = newcity;
            INFO("Added subcity %s to city %s\n", newcity->GetName(), parent_city->GetName());
            cities_init(newcity, ((Directory *)nodes[i])->GetTotalNodes(), nodes[i]->entry.cluster);
        }
        else
        {
            City *newcity = new City(parent_city, rtrim(nodes[i]->Name()), FileType, nodes[i]->entry, nodes[i]->GetFS());
            parent_city->AddSubcity(newcity);
            nodes[i]->OwnCity = newcity;
            INFO("Added subcity %s to city %s\n", newcity->GetName(), parent_city->GetName());
        }
    }
}

void vfs_init()
{
    auto fs = new FatFS;

    if (!fs->succeded)
    {
        ERROR("FS failed.\n");
        return;
    }

    auto root_entries = fs->GetEntries(fs->root_cluster);
    auto root_entry = fs->GetEntry(fs->root_cluster);

    root_city = new City(nullptr, "/", DirectoryType, root_entry, fs);
    root_city->SetCluster(fs->root_cluster);
    root_directory = new Directory(fs, root_entry, root_entries, "/");
    root_directory->OwnCity = root_city;

    cities_init(root_city, root_directory->GetTotalNodes(), fs->root_cluster);

    printf("\n");
}