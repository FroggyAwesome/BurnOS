.global irq0_handler_asm
.global irq1_handler_asm
.extern timer_handler
.extern keyboard_handler
.extern pic_send_end_of_interrupt

.section .text
irq0_handler_asm:
	pushal
	cld
	call timer_handler
	pushl $0
	call pic_send_end_of_interrupt
	addl $4, %esp
	popal
	iret

irq1_handler_asm:
	pushal
	cld
	call keyboard_handler
	pushl $1
	call pic_send_end_of_interrupt
	addl $4, %esp
	popal
	iret
