#include "memory/mmu.h"

.global pgd_address
pgd_address:
    adrp x0, pgd
    ret