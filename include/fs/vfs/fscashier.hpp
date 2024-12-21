#ifndef CASHIER_HPP
#define CASHIER_HPP

#include <lib/map.hpp>
#include <fs/vfs/vfsnode.hpp>

namespace veil::fs::vfs
{
    class FSCashier
    {
        static unsigned long hash(VFSNode *x);

        veil::std::Map<VFSNode *, unsigned long> expiration_dates;
        VFSNode *top_nodes[3];

        void addTops(VFSNode *dir);

    public:
        void Add(VFSNode *dir);
        void Cleanup();

        void RefreshExpiration(VFSNode *dir);

        static void CleanupTask(unsigned long target);
        static FSCashier *GetCashier();

        FSCashier();
    };
}

#endif