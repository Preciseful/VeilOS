#include <fs/vfs/city.hpp>
#include <fs/vfs/file.hpp>
#include <lib/string.h>

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