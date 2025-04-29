#include <fs/vfs/city.hpp>
#include <fs/vfs/file.hpp>
#include <lib/string.h>
#include <lib/list.hpp>

using namespace veil;

City *City::GetSubcity(const unsigned char *name)
{
    for (unsigned long i = 0; i < names.Count(); i++)
    {
        if (strcmp(names[i], name) == 0)
            return cities[i];
    }

    return nullptr;
}

void City::AddSubcity(City *city)
{
    names.Add(city->GetName());
    cities.Add(city);
}

void City::RemoveCity(const unsigned char *name)
{
    for (unsigned long i = 0; i < names.Count(); i++)
    {
        if (strcmp(name, names[i]) != 0)
        {
            names.RemoveAt(i);
            cities.RemoveAt(i);
            return;
        }
    }
}

File *City::GetFile(const unsigned char *city_name)
{
    auto filecity = this->GetSubcity(city_name);
    if (!filecity)
        return nullptr;

    return new File(filecity->fs, filecity->entry, filecity->name);
}

Directory *City::GetDirectory(const unsigned char *city_name)
{
    auto dircity = this->GetSubcity(city_name);
    if (!dircity)
        return nullptr;

    return new Directory(dircity->fs, dircity->entry, fs->GetEntries(dircity->GetCluster()), dircity->name);
}

unsigned char *City::AbsolutePath()
{
    veil::std::List<City *> cities;
    City *city = this->Parent;
    unsigned long size = 0;
    while (city)
    {
        cities.Add(city);
        size += strlen(city->name) + 1;
        city = city->Parent;
    }

    unsigned char *dir = (unsigned char *)valloc(size - 1);
    unsigned long len = 0;

    for (unsigned long i = cities.Count(); i-- > 0;)
    {
        len += strlen(cities[i]->name) - 1;
        memcpy(dir, cities[i]->name, len);
        if (i != cities.Count() - 1)
        {
            dir[len] = '/';
            len++;
        }
    }

    return dir;
}