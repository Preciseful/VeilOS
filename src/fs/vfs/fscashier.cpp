#include <fs/vfs/fscashier.hpp>
#include <lib/list.hpp>
#include <mm.h>
#include <drivers/timer.h>
#include <scheduler.h>
#include <stdbool.h>

using namespace veil;

FSCashier *cashier = 0;

unsigned long FSCashier::hash(VFSNode *x)
{
    return (unsigned long)x;
}

FSCashier::FSCashier() : expiration_dates(hash)
{
    if (cashier != nullptr)
        printf("CASHIER ALREADY EXISTS. DO NOT MAKE MULTIPLES.\n");
    cashier = this;
}

FSCashier *FSCashier::GetCashier()
{
    return cashier;
}

void FSCashier::Add(VFSNode *dir)
{
    this->expiration_dates.Add(dir, timer_get_ticks() + 180 * CLOCKHZ);
}

void FSCashier::RefreshExpiration(VFSNode *dir)
{
    unsigned long val = timer_get_ticks() + 180 * CLOCKHZ;
    this->expiration_dates.Set(dir, val);
}

void FSCashier::CleanupTask(unsigned long target)
{
    FSCashier *cashier = (FSCashier *)target;
    cashier->Cleanup();
}

void FSCashier::addTops(VFSNode *dir)
{
    for (int i = 0; i < 3; i++)
    {
        if (top_nodes[i] && top_nodes[i]->Interactions() < dir->Interactions())
        {
            for (int j = 2; j > i; j--)
                top_nodes[j] = top_nodes[j - 1];

            top_nodes[i] = dir;
            break;
        }

        else if (!top_nodes[i])
        {
            top_nodes[i] = dir;
            break;
        }
    }
}

void FSCashier::Cleanup()
{
    preempt_disable();

    for (unsigned long i = 0; i < this->expiration_dates.keys.Count(); i++)
        addTops(this->expiration_dates.keys[i]);

    for (unsigned long i = 0; i < this->expiration_dates.keys.Count(); i++)
    {
        VFSNode *dir = expiration_dates.keys[i];
        unsigned long expiration = expiration_dates.values[i];

        if (dir->IsPreserved())
            continue;
        if (expiration > timer_get_ticks())
            continue;
        if (top_nodes[0] == dir || top_nodes[1] == dir || top_nodes[2] == dir)
            continue;

        expiration_dates.RemoveAt(i);

        delete dir;
    }

    preempt_enable();
}