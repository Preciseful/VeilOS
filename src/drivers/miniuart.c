#include <drivers/miniuart.h>
#include <drivers/gpio.h>
#include <drivers/framebuffer.h>
#include <mm.h>
#include <scheduler.h>

unsigned char uart_output_queue[UART_MAX_QUEUE];
unsigned int uart_output_queue_write = 0;
unsigned int uart_output_queue_read = 0;

bool printf_use_framebuffer = false;

void uart_init()
{
    mmio_write(AUX_ENABLES, 1); // Enable UART1
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3); // 8 bits
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_IIR_REG, 0xC6); // Disable interrupts
    mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));
    gpio_setAlt5(14);
    gpio_setAlt5(15);
    mmio_write(AUX_MU_CNTL_REG, 3); // Enable RX/TX
}

bool uart_outputQueueEmpty()
{
    return uart_output_queue_read == uart_output_queue_write;
}

bool uart_readingReady()
{
    return mmio_read(AUX_MU_LSR_REG) & 0x01;
}

bool uart_writingReady()
{
    return mmio_read(AUX_MU_LSR_REG) & 0x20;
}

unsigned char uart_readByte()
{
    while (!uart_readingReady())
        ;

    return (unsigned char)mmio_read(AUX_MU_IO_REG);
}

void uart_writeCurrentByte(unsigned char character)
{
    while (!uart_writingReady())
        ;

    mmio_write(AUX_MU_IO_REG, (unsigned int)character);
}

void uart_loadOutput()
{
    while (!uart_outputQueueEmpty() && uart_writingReady())
    {
        uart_writeCurrentByte(uart_output_queue[uart_output_queue_read]);
        uart_output_queue_read = (uart_output_queue_read + 1) & (UART_MAX_QUEUE - 1); // Don't overrun
    }
}

void uart_writeNextByte(unsigned char character)
{
    unsigned int next = (uart_output_queue_write + 1) & (UART_MAX_QUEUE - 1); // Don't overrun

    while (next == uart_output_queue_read)
        uart_loadOutput();

    uart_output_queue[uart_output_queue_write] = character;
    uart_output_queue_write = next;
}

void uart_write(const char *text)
{
    while (*text)
    {
        if (*text == '\n')
            uart_writeCurrentByte('\r');
        uart_writeCurrentByte(*text++);
    }
}

void uart_put(const char c)
{
    if (c == '\n')
        uart_writeCurrentByte('\r');
    uart_writeCurrentByte((unsigned char)c);
}

void uart_drainOutputQueue()
{
    while (!uart_outputQueueEmpty())
        uart_loadOutput();
}

unsigned char uart_update()
{
    uart_loadOutput();

    if (uart_readingReady())
    {
        unsigned char ch = uart_readByte();

        if (ch == '\r')
        {
            uart_write("\n");
            return '\n';
        }
        else
        {
            uart_writeNextByte(ch);
            return ch;
        }
    }

    return '\0';
}

char uart_readchar()
{
    return uart_update();
}

void putc(void *p, char c)
{
    uart_put((unsigned char)c);
    if (framebuffer != 0 && printf_use_framebuffer)
        framebuffer_putc(c, 0x0f);
}
