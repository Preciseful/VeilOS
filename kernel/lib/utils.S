.macro push r
    str \r, [sp, #-16]!
.endm

.macro pop r
    ldr \r, [sp], #16
.endm

.macro pushp r1, r2
    stp \r1, \r2, [sp, #-16]!
.endm

.macro popp r1, r2
    ldp \r1, \r2, [sp], #16
.endm