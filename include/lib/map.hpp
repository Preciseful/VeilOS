#ifndef MAP_HPP
#define MAP_HPP

#include <lib/list.hpp>

// this is slow
namespace veil::std
{
    template <typename T, typename U>
    class Map
    {
        unsigned long (*get_hash)(T x);
        veil::std::List<unsigned long> hashes;

    public:
        veil::std::List<T> keys;
        veil::std::List<U> values;

        Map(unsigned long (*hash)(T x))
        {
            this->get_hash = hash;
        }

        void Add(T key, U value)
        {
            unsigned long hash = get_hash(key);
            for (unsigned long i = 0; i < keys.Count(); i++)
            {
                if (hashes[i] != hash)
                    continue;

                printf("KEYS COLLIDE!\n");
                return;
            }

            keys.Add(key);
            hashes.Add(hash);
            values.Add(value);
        }

        void Remove(T key)
        {
            unsigned long hash = get_hash(key);
            for (int i = 0; i < keys.Count(); i++)
            {
                if (keys[i] != hash)
                    continue;

                this->RemoveAt(i);
                break;
            }
        }

        void RemoveAt(unsigned long i)
        {
            keys.RemoveAt(i);
            values.RemoveAt(i);
            hashes.RemoveAt(i);
        }

        bool Get(T key, U &value)
        {
            unsigned long hash = get_hash(key);
            for (int i = 0; i < keys.Count(); i++)
            {
                if (hashes[i] != hash)
                    continue;

                value = values[i];
                return true;
            }

            return false;
        }

        bool Set(T key, U value)
        {
            unsigned long hash = get_hash(key);
            for (unsigned long i = 0; i < keys.Count(); i++)
            {
                if (hashes[i] != hash)
                    continue;

                values[i] = value;
                return true;
            }

            return false;
        }
    };
}

#endif