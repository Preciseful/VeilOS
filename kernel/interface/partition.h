/**
 * @file
 * @author Developful
 * @brief Interfaces for partitions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <fs/mbr.h>

typedef struct Partition
{
    char *name;
    /// @brief The offset in the SD card of the partition.
    unsigned long offset;
    PartitionEntry backing_data;
    MasterBootRecord mbr;
} Partition;

/**
 * @brief Initialize partitions from the SD card.
 *
 * @return Array of partitions.
 */
Partition *PartitionsInit();