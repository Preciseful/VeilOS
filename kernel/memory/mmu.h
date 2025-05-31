#pragma once

#define MT_DEVICE_nGnRnE_INDEX 0
#define MT_NORMAL_NC_INDEX 1
#define MT_DEVICE_nGnRnE 0x0
#define MT_NORMAL_NC 0x1
#define MT_DEVICE_nGnRnE_FLAGS 0x00
#define MT_NORMAL_NC_FLAGS 0x44
#define MAIR_VALUE (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC))

#define TCR_TG1_4K (2 << 30)
#define TCR_T1SZ ((64 - 48) << 16)
#define TCR_TG0_4K (0 << 14)
#define TCR_T0SZ (64 - 48)
#define TCR_VALUE (TCR_TG1_4K | TCR_T1SZ | TCR_TG0_4K | TCR_T0SZ)

#define CPACR_FPEN (1 << 21) | (1 << 20)
#define CPACR_ZEN (1 << 17) | (1 << 16)
#define CPACR_VALUE (CPACR_FPEN | CPACR_ZEN)