/**
 * @file
 * @author Developful
 * @brief ARM Semihost communication helpers.
 * @date 2026-09-18
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

typedef long DFILEHANDLE;

enum Debug_Operations
{
    Debug_Clock = 0x10,
    Debug_Close = 0x02,
    Debug_Elapsed = 0x30,
    Debug_Errno = 0x13,
    Debug_Exit = 0x18,
    Debug_Exit_Extended = 0x20,
    Debug_Flen = 0x0c,
    Debug_Get_Cmdline = 0x15,
    Debug_HeapInfo = 0x16,
    Debug_IsError = 0x08,
    Debug_IsTty = 0x09,
    Debug_Open = 0x01,
    Debug_Read = 0x06,
    Debug_Readc = 0x07,
    Debug_Remove = 0x0e,
    Debug_Rename = 0x0f,
    Debug_Seek = 0x0a,
    Debug_System = 0x12,
    Debug_TickFreq = 0x31,
    Debug_Time = 0x11,
    Debug_Tmpnam = 0x0d,
    Debug_Write = 0x05,
    Debug_Writec = 0x03,
    Debug_Write0 = 0x04,
};

extern unsigned long tell_debug_host(enum Debug_Operations operation, void *value);

/**
 * @brief Prints formatted output through the host.
 */
void DebugPrintf(char *fmt, ...);

DFILEHANDLE DebugFileOpen(char *path, unsigned long mode);
void DebugFileSeek(DFILEHANDLE handle, unsigned long seek);
unsigned long DebugFileWrite(DFILEHANDLE handle, char *data);
unsigned long DebugFileRead(DFILEHANDLE handle, char *buf, unsigned long len);
void DebugFileClose(DFILEHANDLE handle);