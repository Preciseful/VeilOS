void print(const char *msg, unsigned long size)
{
    for (unsigned long i = 0; i < size; i++)
    {
        register unsigned long x8 asm("x8") = 6;
        register unsigned long x0 asm("x0") = (unsigned long)msg[i];
        asm volatile("svc #8" : "+r"(x0) : "r"(x8) : "memory", "cc");
    }
}

int main()
{
    print("Voidelle fs module\n", 20);
    while (1)
    {
    }
}