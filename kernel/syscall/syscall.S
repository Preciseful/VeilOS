.global sys_printf
sys_printf:
    mov x8, #0
    svc #8
    ret