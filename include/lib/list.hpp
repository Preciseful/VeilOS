#ifndef LIST_HPP
#define LIST_HPP
#include <mm.h>
#include <lib/printf.h>
#include <lib/type_traits.hpp>

namespace veil::std
{
    template <typename T>
    class List
    {
        T *backing_data;
        unsigned long count;
        unsigned long capacity;

        T &get(unsigned long index)
        {
            if (index >= count)
                ERROR("bad indexo %lu\n", index);
            return backing_data[index];
        }

    public:
        List() : count(0), capacity(2)
        {
            backing_data = (T *)valloc(sizeof(T) * capacity);
        }

        List(unsigned long capacity) : count(0), capacity(capacity)
        {
            backing_data = (T *)valloc(sizeof(T) * capacity);
        }

        List(unsigned long count, unsigned long capacity) : count(count), capacity(capacity)
        {
            backing_data = (T *)valloc(sizeof(T) * capacity);
        }

        unsigned long Count()
        {
            return count;
        }

        unsigned long Capacity()
        {
            return capacity;
        }

        void Add(T item)
        {
            if (this->count >= this->capacity)
            {
                auto last = backing_data;
                backing_data = (T *)valloc(sizeof(T) * capacity);
                this->capacity *= 2;
                for (unsigned long i = 0; i < count; i++)
                    backing_data[i] = last[i];

                vfree(last);
            }

            backing_data[count] = item;
            count++;
        }

        void AddRange(List<T> items)
        {
            for (unsigned long i = 0; i < items.Count(); i++)
                Add(items[i]);
        }

        void Remove(T &item)
        {
            for (unsigned long i = 0; i < count; i++)
            {
                if (&backing_data[i] == item)
                {
                    RemoveAt(i);
                    break;
                }
            }
        }

        void RemoveAt(unsigned long index)
        {
            for (unsigned long i = index; i < count - 1; i++)
                backing_data[index] = backing_data[index + 1];

            count--;
        }

        void Clear()
        {
            for (unsigned int i = 0; i < count; i++)
                backing_data[i] = 0;

            count = 0;
        }

        T &operator[](unsigned long index)
        {
            return get(index);
        }

        template <typename U>
            requires is_convertible<T, U>::value
        List<U> Convert()
        {
            List<U> Us;
            for (unsigned long i = 0; i < count; i++)
            {
                Us.Add((U)get(i));
            }

            return Us;
        }
    };
}

#endif