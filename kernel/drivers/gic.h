#pragma once

#include <boot/base.h>

#define GIC_BASE (PERIPHERAL_BASE + 0x1840000)
#define GICD_DIST_BASE (GIC_BASE + 0x00001000)
#define GICC_CPU_BASE (GIC_BASE + 0x00002000)
#define GICD_ENABLE_IRQ_BASE (GICD_DIST_BASE + 0x00000100)
#define GICC_IAR (GICC_CPU_BASE + 0x0000000C)
#define GICC_EOIR (GICC_CPU_BASE + 0x00000010)
#define GIC_IRQ_TARGET_BASE (GICD_DIST_BASE + 0x00000800)

#ifndef __ASSEMBLER__

void gic_allow(unsigned int interrupt, unsigned int core);

#endif