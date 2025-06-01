#pragma once

#define SYS_FREQ 54000000

#ifndef __ASSEMBLER__

extern void timer_init();
extern void refresh_cntp_tval(unsigned int val);

#endif