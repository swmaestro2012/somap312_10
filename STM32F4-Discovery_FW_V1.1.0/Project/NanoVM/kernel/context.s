
	AREA    |.text|, CODE, READONLY

Exit_handler_mode PROC
	EXPORT	Exit_handler_mode
	ldr		r0, =0xFFFFFFF9
	push	{r0}
	push	{r0}
	pop		{r0,pc}
	;BX		r0
	ENDP

Context_switch_isr PROC
	EXPORT  Context_switch_isr	
	push	{r4-r11}
	;push	{r12}
	adr		r11, __switch_end_isr
	;push	{r12}
	;push	{lr}
	;ldr		r10, =0xA1000000
	mrs		r10, eapsr
	orr		r10, r10, #0x01000000
	push	{r10}
	push	{r11}
	;push	lr
	push	{r0-r3,r12,lr}
	;vpush   {s16-s31}
;	MRS		r2, CONTROL
	str		sp, [r0]
	mov		sp, r1
	ldr		r0, [sp, #24]
	adr		r11, __switch_end_isr
	cmp		r0, r11
	beq		__to_handle_mode
	ldr		pc, =0xFFFFFFF9
__to_handle_mode
	pop		{r0-r3,r12,lr}
	pop		{r4}
	pop		{r5}
	ldr		r5, =0x01000000
	msr		epsr, r5
	mrs		r5, epsr
	mov		pc,r4
__switch_end_isr
	pop		{r4-r11}
	bx		lr
	
;//		:
;//		: "m" (current_task->stack_pointer), "m" (next_task->stack_pointer)//, "m" (ctx->cpsr)
	ENDP

Context_switch PROC
	EXPORT  Context_switch
	push	{r4-r11}
	;push	{r12}
	adr		r11, __switch_end
	;push	{r12}
	;push	{lr}
	ldr		r10, =0x0
	push	{r10}
	push	{r11} 
	push	{r0-r3,r12,lr}
	str		sp, [r0]
	mov		sp, r1
	pop		{r0-r3,r12,lr}
	pop		{r4}
	pop		{r5}
	mov		pc, r4	
__switch_end
	pop		{r4-r11}
	bx		lr

;	push	{r12}
;	adr		r12, __switch_end
;	push	{r12}
;	push	{lr}
;	push	{r0-r11}
;	str		sp, [r0]
;	mov		sp, r1
;	pop		{r0-r11}
;	pop		{lr}
;	pop		{r12}
;	mov		pc,r12
;__switch_end
;	pop		{r12}	
;	BX		lr
	ENDP
	
	
IntSaveDisableIRQ PROC
	EXPORT IntSaveDisableIRQ
	push	{r1}
	ldr		r1,=0x1
	mrs		r0,PRIMASK
	msr		PRIMASK,r1
	pop		{r1}
	BX		lr
	ENDP
IntRestoreIRQ	PROC
	EXPORT IntRestoreIRQ
	push	{r0}
	;ldr		r0,=0x0
	msr		PRIMASK,r0
	pop		{r0}
	BX		lr
	ENDP
	
	END