/*
 * aarch64/dbgc.c
 *
 * Copyright (C) 2021 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @brief Architecture dependent parts of the mini debugger
 */

void Printf(char *fmt, ...);
extern unsigned long dbg_regs[37];
extern char dbg_cmd[256], dbg_running;

/**
 * Decode exception cause
 */
void dbg_decodeexc(unsigned long type)
{
    unsigned char cause = dbg_regs[33] >> 26;

    /* print out interruption type */
    switch (type)
    {
    case 0:
        Printf("Synchronous");
        break;
    case 1:
        Printf("IRQ");
        break;
    case 2:
        Printf("FIQ");
        break;
    case 3:
        Printf("SError");
        break;
    }
    Printf(": ");
    /* decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28) */
    switch (cause)
    {
    case 0b000000:
        Printf("Unknown");
        break;
    case 0b000001:
        Printf("Trapped WFI/WFE");
        break;
    case 0b001110:
        Printf("Illegal execution");
        break;
    case 0b010101:
        Printf("System call");
        break;
    case 0b100000:
        Printf("Instruction abort, lower EL");
        break;
    case 0b100001:
        Printf("Instruction abort, same EL");
        break;
    case 0b100010:
        Printf("Instruction alignment fault");
        break;
    case 0b100100:
        Printf("Data abort, lower EL");
        break;
    case 0b100101:
        Printf("Data abort, same EL");
        break;
    case 0b100110:
        Printf("Stack alignment fault");
        break;
    case 0b101100:
        Printf("Floating point");
        break;
    case 0b110000:
        Printf("Breakpoint, lower EL");
        break;
    case 0b110001:
        Printf("Breakpoint, same EL");
        break;
    case 0b111100:
        Printf("Breakpoint instruction");
        break;
    default:
        Printf("Unknown %x", cause);
        break;
    }
    /* decode data abort cause */
    if (cause == 0b100100 || cause == 0b100101)
    {
        Printf(", ");
        switch ((dbg_regs[33] >> 2) & 0x3)
        {
        case 0:
            Printf("Address size fault");
            break;
        case 1:
            Printf("Translation fault");
            break;
        case 2:
            Printf("Access flag fault");
            break;
        case 3:
            Printf("Permission fault");
            break;
        }
        switch (dbg_regs[33] & 0x3)
        {
        case 0:
            Printf(" at level 0");
            break;
        case 1:
            Printf(" at level 1");
            break;
        case 2:
            Printf(" at level 2");
            break;
        case 3:
            Printf(" at level 3");
            break;
        }
    }
    Printf("\n");
    /* if the exception happened in the debugger, we stop to avoid infinite loop */
    if (dbg_running)
    {
        Printf("Exception in mini debugger!\n"
               "  elr_el1: %x  spsr_el1: %x\n  esr_el1: %x  far_el1: %x\nsctlr_el1: %x  tcr_el1: %x\n",
               dbg_regs[31], dbg_regs[32], dbg_regs[33], dbg_regs[34], dbg_regs[35], dbg_regs[36]);
        while (1)
            ;
    }
}

/**
 * Parse register name
 */
int dbg_getreg(int i, int *reg)
{
    if (dbg_cmd[i] == 'x' || dbg_cmd[i] == 'r')
    {
        i++;
        if (dbg_cmd[i] >= '0' && dbg_cmd[i] <= '9')
        {
            *reg = dbg_cmd[i] - '0';
        }
        i++;
        if (dbg_cmd[i] >= '0' && dbg_cmd[i] <= '9')
        {
            *reg *= 10;
            *reg += dbg_cmd[i] - '0';
        }
    }
    return i;
}

/**
 * Dump registers
 */
void dbg_dumpreg()
{
    int i;
    /* general purpose registers x0-x30 */
    for (i = 0; i < 31; i++)
    {
        if (i && i % 3 == 0)
            Printf("\n");
        if (i < 10)
            Printf(" ");
        Printf("x%d: %16x  ", i, dbg_regs[i]);
    }
    /* some system registers */
    Printf("elr_el1: %x  spsr_el1: %x\n  esr_el1: %x  far_el1: %x\nsctlr_el1: %x  tcr_el1: %x\n",
           dbg_regs[31], dbg_regs[32], dbg_regs[33], dbg_regs[34], dbg_regs[35], dbg_regs[36]);
}
