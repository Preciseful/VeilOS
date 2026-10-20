#include <system/debug/update.h>
#include <drivers/power.h>
#include <system/debug/semihost.h>
#include <system/vfs.h>
#include <lib/printf.h>

#define READ_CHUNK (PAGE_SIZE * 2)

void ForceUpdate()
{
    // todo: make forced restart work properly, prolly some registers that need resetting
    LOG("Forcing an update!\n");

    FILEHANDLE kernel_handle = OpenFile(FILE_WRITE, "/firmware/kernel8.img");
    LOG("Kernel handle: %li\n", kernel_handle);

    DFILEHANDLE build_handle = DebugFileOpen("kernel8.img", 0);
    LOG("Host kernel handle: %li\n", build_handle);
    unsigned long offset = 0;

    while (true)
    {
        char *buf = malloc(READ_CHUNK);
        unsigned long value = DebugFileRead(build_handle, buf, READ_CHUNK);
        unsigned long read_amount = READ_CHUNK - value;
        LOG("Read: %lu\n", read_amount);

        if (read_amount == 0)
        {
            free(buf);
            break;
        }

        long write_value = WriteFile(kernel_handle, buf, read_amount, offset);
        LOG("Write: %li\n", write_value);

        if (read_amount < READ_CHUNK)
        {
            free(buf);
            break;
        }

        offset += READ_CHUNK;
    }

    CloseFile(kernel_handle);
    DebugFileClose(build_handle);

    LOG("Waiting for cold boot...\n");
    while (true)
        ;
}