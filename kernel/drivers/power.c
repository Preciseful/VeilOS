#include <drivers/power.h>
#include <boot/base.h>
#include <drivers/gpio.h>
#include <drivers/emmc.h>

#define PM_BASE (PERIPHERAL_BASE + 0x100000)
#define PM_RSTC (PM_BASE + 0x1C)
#define PM_WATCHDOG (PM_BASE + 0x24)

#define PM_PASSWORD 0x5A000000

#define PM_WATCHDOG_TIME_SET 0x000fffff

#define PM_RSTC_WRCFG_CLR 0xFFFFFFCF
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020

void Restart()
{
    CommandEMMC(CTGoIdle, 0, 2000);

    unsigned long rstc = ReadMMIO(PM_RSTC);
    unsigned long watchdog = PM_PASSWORD | (0x120 & PM_WATCHDOG_TIME_SET);

    rstc = PM_PASSWORD | (rstc & PM_RSTC_WRCFG_CLR) | PM_RSTC_WRCFG_FULL_RESET;

    WriteToMMIO(PM_WATCHDOG, watchdog);
    WriteToMMIO(PM_RSTC, rstc);
}