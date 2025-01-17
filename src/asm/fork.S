.global return_from_fork
return_from_fork:
	bl preempt_enable
	mov	x0, x20
	cmp x11, #0
	beq el0
	bne el1
el0:	
	bl go_el0
	b finish
el1:
	bl go_el1
	b finish

finish: 
	mov sp, x9
	blr	x19