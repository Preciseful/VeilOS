#pragma once

extern void jump_high(unsigned long pgd, unsigned long high_pgd, unsigned long last_page);

void JumpToHigher(void *dtb);