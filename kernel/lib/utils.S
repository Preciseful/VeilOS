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

.macro pushx r, x
    str \r, [\x, #-16]!
.endm

.macro popx r, x
    ldr \r, [\x], #16
.endm

.macro pushpx r1, r2, x
    stp \r1, \r2, [\x, #-16]!
.endm

.macro poppx r1, r2, x
    ldp \r1, \r2, [\x], #16
.endm