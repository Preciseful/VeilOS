.global return_from_fork
return_from_fork:
	bl preempt_enable
	mov	x0, x20
	blr	x19