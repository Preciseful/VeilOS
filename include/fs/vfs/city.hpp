#ifndef CITY_HPP
#define CITY_HPP

#include <lib/list.hpp>
#include <fs/vfs/file.hpp>
#include <fs/vfs/directory.hpp>
#include <fs/fat32.hpp>

namespace veil
{
    enum CityType
    {
        DirectoryType = 1,
        FileType = 2
    };

    class City
    {
    private:
        veil::std::List<City *> cities;
        unsigned int cluster = 0;
        FAT32DirectoryEntry entry;

    public:
        const unsigned char *name;
        veil::std::List<const unsigned char *> names;
        FatFS *fs;
        City *Parent;
        unsigned char Attributes;

        City *GetSubcity(const unsigned char *name);
        void AddSubcity(City *city);
        void RemoveCity(const unsigned char *name);
        File *GetFile(const unsigned char *city);
        Directory *GetDirectory(const unsigned char *city_name);
        unsigned char *AbsolutePath();

        const unsigned char *GetName()
        {
            return name;
        }

        unsigned int GetCluster()
        {
            if (cluster == 0)
                return entry.cluster;
            return cluster;
        }

        void SetCluster(unsigned int cluster)
        {
            this->cluster = cluster;
        }

        City(City *parent, const unsigned char *name, CityType type, FAT32DirectoryEntry entry, FatFS *fs) : entry(entry),
                                                                                                             name(name),
                                                                                                             fs(fs),
                                                                                                             Parent(parent),
                                                                                                             Attributes(type) {}
        City(City *parent, const char *name, CityType type, FAT32DirectoryEntry entry, FatFS *fs) : entry(entry),
                                                                                                    name((const unsigned char *)name),
                                                                                                    fs(fs),
                                                                                                    Parent(parent),
                                                                                                    Attributes(type) {}
    };
}

#endif