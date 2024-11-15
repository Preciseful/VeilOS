#include <drivers/watchdog.h>
#include <lib/base.h>
#include <funcs.h>

#define PM_BASE (PERIPHERAL_BASE + 0x100000) /* Power Management, Reset controller and Watchdog registers */
#define PM_RSTC (PM_BASE + 0x1c)
#define PM_WDOG (PM_BASE + 0x24)
#define PM_WDOG_RESET 0000000000
#define PM_PASSWORD 0x5a000000
#define PM_WDOG_TIME_SET 0x000fffff
#define PM_RSTC_WRCFG_CLR 0xffffffcf
#define PM_RSTC_WRCFG_SET 0x00000030
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020
#define PM_RSTC_RESET 0x00000102

void watchdog_start(unsigned int timeout)
{
    unsigned int pm_rstc, pm_wdog;

    pm_rstc = get32(PM_RSTC);
    pm_wdog = PM_PASSWORD | (timeout & PM_WDOG_TIME_SET); // watchdog timer = timer clock / 16
                                                          // need password (31:16) + value (11:0)
    pm_rstc = PM_PASSWORD | (pm_rstc & PM_RSTC_WRCFG_CLR) | PM_RSTC_WRCFG_FULL_RESET;
    put32(PM_WDOG, pm_wdog);
    put32(PM_RSTC, pm_rstc);
}

void watchdog_stop()
{
    put32(PM_RSTC, PM_PASSWORD | PM_RSTC_RESET);
}

unsigned int watchdog_remaining()
{
    return (get32(PM_WDOG) & PM_WDOG_TIME_SET);
}