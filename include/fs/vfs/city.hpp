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
        DirectoryType = 0,
        FileType = 1
    };

    class City
    {
    private:
        veil::std::List<const unsigned char *> names;
        veil::std::List<City *> cities;
        const unsigned char *name;
        FAT32DirectoryEntry entry;
        FatFS *fs;

    public:
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
            return entry.cluster;
        }

        City(City *parent, const unsigned char *name, CityType type, FAT32DirectoryEntry entry, FatFS *fs) : name(name),
                                                                                                             entry(entry),
                                                                                                             fs(fs),
                                                                                                             Parent(parent),
                                                                                                             Attributes(type) {}
        City(City *parent, const char *name, CityType type, FAT32DirectoryEntry entry, FatFS *fs) : name((const unsigned char *)name),
                                                                                                    entry(entry),
                                                                                                    fs(fs),
                                                                                                    Parent(parent),
                                                                                                    Attributes(type) {}
    };
}

#endif