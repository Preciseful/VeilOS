.global return_from_fork
return_from_fork:
	bl preempt_enable
	cbz	x19, ret_to_user			// not a kernel thread
	mov	x0, x20
	mov sp, x9
	blr	x19