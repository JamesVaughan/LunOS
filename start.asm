[BITS 32]
global start
start:
    mov esp, sys_stack     ; This points the stack to our new stack area
    push ebx
    jmp stublet

; This part MUST be 4byte aligned, so we solve that issue using 'ALIGN 4'
ALIGN 4
mboot:
    MULTIBOOT_PAGE_ALIGN   equ 1<<0
    MULTIBOOT_MEMORY_INFO  equ 1<<1

    MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
    CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

    ; The Multiboot header (in NASM syntax)
        align 4
        dd MULTIBOOT_HEADER_MAGIC
        dd MULTIBOOT_HEADER_FLAGS
        dd CHECKSUM

; This is an endless loop here. Make a note of this: Later on, we
; will insert an 'extern main', followed by 'call main', right
; before the 'jmp $'.
stublet:
    extern startC
	;ENABLE SSE
	mov eax, cr0
	and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
	or ax, 0x2			;set coprocessor monitoring  CR0.MP
	mov cr0, eax
	mov eax, cr4
	or eax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	or eax, 1 << 18      ;set CR4.OSXSAVE
	mov cr4, eax
	FINIT
	;Start the main phase of the kernel
    call startC
    jmp $

Global InitFPU
InitFPU:
pusha
mov ebx, esp
push dword 0
	mov eax, cr4
	or eax, 0x200
	mov cr4, eax
	fstcw [ebx]
	mov dword[ebx], 0x37F
	fldcw [edx]
pop dword ebx
popa
ret

;Read from the global registers
global ReadMSR
ReadMSR:
push ebp
mov ebp, esp
pushad
	mov ecx, [ebp+8]
	rdmsr
	mov ebx, [ebp+12]
	mov [ebx], eax
	mov [ebx + 4], edx
popad
pop ebp
retn

;Read from the global registers
global WriteMSR
WriteMSR:
push ebp
mov ebp, esp
pushad
	mov ecx, [ebp+8]
	mov ebx, [ebp+12]
	mov eax, [ebx]
	mov edx, [ebx + 4]
	wrmsr
popad
pop ebp
retn

global GetCPUID
GetCPUID:
push ebp
mov ebp, esp
pushad
	mov eax, [ebp+8]
	cpuid
	mov ebx, [ebp+12]
	mov [ebx], eax
	mov ebx, [ebp+16]
	mov [ebx], edx
popad
pop ebp
retn
; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'
global gdt_flush
extern gp
gdt_flush:
    lgdt [gp]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2
flush2:
    ret

global SetCR3
SetCR3:
	push ebp
	mov ebp, esp
	pushad
	mov eax, [ebp+8]
	mov cr3, eax
	popad
	pop ebp
retn

global UpdateTLBEntry
UpdateTLBEntry:
	push ebp
	mov ebp, esp
	push eax
	mov eax, ebp
	add eax, 8
	invlpg [eax] ;invalidate the page located in the parameter
	pop eax
	pop ebp
retn

global SetPageFlag
SetPageFlag:
	pushad
	mov eax, cr0
	;set flag 31
	or eax, 0x80000000
	mov cr0, eax
	popad
retn

global GetCR2
GetCR2:
 mov eax, cr2
retn

global GetCR3
GetCR3:
 mov eax, cr3
retn

global GetCR0
GetCR0:
 mov eax, cr0
retn

global ClearPageFlag
ClearPageFlag:
	wbinvd
	push eax
	mov eax, cr0
	;clear flag 31
	and eax,0x7FFFFFFF
	mov cr0, eax
	pop eax
retn

global asmTryLock
asmTryLock:
	push ebp
	mov ebp, esp
	push ebx
	mov eax, 0x01
	mov ebx, [ebp + 8]
	xchg dword[ebx], eax
	pop ebx
	pop ebp
retn

; Loads the IDT defined in 'idtp' into the processor.
; This is declared in C as 'extern void idt_load();'
global idt_load
extern idtp
idt_load:
    lidt [idtp]
retn

; In just a few pages in this tutorial, we will add our Interrupt
; Service Routines (ISRs) right here!
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

;  0: Divide By Zero Exception
isr0:
    cli
    push byte 0
    push byte 0
    jmp isr_common_stub

;  1: Debug Exception
isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common_stub

;  2: Non Maskable Interrupt Exception
isr2:
    cli
    push byte 0
    push byte 2
    jmp isr_common_stub

;  3: Int 3 Exception
isr3:
    cli
    push byte 0
    push byte 3
    jmp isr_common_stub

;  4: INTO Exception
isr4:
    cli
    push byte 0
    push byte 4
    jmp isr_common_stub

;  5: Out of Bounds Exception
isr5:
    cli
    push byte 0
    push byte 5
    jmp isr_common_stub

;  6: Invalid Opcode Exception
isr6:
    cli
    push byte 0
    push byte 6
    jmp isr_common_stub

;  7: Coprocessor Not Available Exception
isr7:
    cli
    push byte 0
    push byte 7
    jmp isr_common_stub

;  8: Double Fault Exception (With Error Code!)
isr8:
    cli
    push byte 8
    jmp isr_common_stub

;  9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push byte 0
    push byte 9
    jmp isr_common_stub

; 10: Bad TSS Exception (With Error Code!)
isr10:
    cli
    push byte 10
    jmp isr_common_stub

; 11: Segment Not Present Exception (With Error Code!)
isr11:
    cli
    push byte 11
    jmp isr_common_stub

; 12: Stack Fault Exception (With Error Code!)
isr12:
    cli
    push byte 12
    jmp isr_common_stub

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    cli
    push byte 13
    jmp isr_common_stub

; 14: Page Fault Exception (With Error Code!)
isr14:
    cli
    push byte 14
    jmp isr_common_stub

; 15: Reserved Exception
isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_common_stub

; 16: Floating Point Exception
isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_common_stub

; 17: Alignment Check Exception
isr17:
    cli
    push byte 0
    push byte 17
    jmp isr_common_stub

; 18: Machine Check Exception
isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_common_stub

; 19: Reserved
isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_common_stub

; 20: Reserved
isr20:
    cli
    push byte 0
    push byte 20
    jmp isr_common_stub

; 21: Reserved
isr21:
    cli
    push byte 0
    push byte 21
    jmp isr_common_stub

; 22: Reserved
isr22:
    cli
    push byte 0
    push byte 22
    jmp isr_common_stub

; 23: Reserved
isr23:
    cli
    push byte 0
    push byte 23
    jmp isr_common_stub

; 24: Reserved
isr24:
    cli
    push byte 0
    push byte 24
    jmp isr_common_stub

; 25: Reserved
isr25:
    cli
    push byte 0
    push byte 25
    jmp isr_common_stub

; 26: Reserved
isr26:
    cli
    push byte 0
    push byte 26
    jmp isr_common_stub

; 27: Reserved
isr27:
    cli
    push byte 0
    push byte 27
    jmp isr_common_stub

; 28: Reserved
isr28:
    cli
    push byte 0
    push byte 28
    jmp isr_common_stub

; 29: Reserved
isr29:
    cli
    push byte 0
    push byte 29
    jmp isr_common_stub

; 30: Reserved
isr30:
    cli
    push byte 0
    push byte 30
    jmp isr_common_stub

; 31: Reserved
isr31:
    cli
    push byte 0
    push byte 31
    jmp isr_common_stub


; We call a C function in here. We need to let the assembler know
; that '_fault_handler' exists in another file
extern fault_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:

    push ds
    push es
    push fs
    push gs
    pushad
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, fault_handler
    call eax
    pop eax
    popad
    pop gs
    pop fs
    pop es
    pop ds

    add esp, 8
    iret

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15
global irq16
global irq17

; 32: IRQ0
irq0:
    cli
    push byte 0
    push byte 32
    jmp irq_common_stub

; 33: IRQ1
irq1:
    cli
    push byte 0
    push byte 33
    jmp irq_common_stub

; 34: IRQ2
irq2:
    cli
    push byte 0
    push byte 34
    jmp irq_common_stub

; 35: IRQ3
irq3:
    cli
    push byte 0
    push byte 35
    jmp irq_common_stub

; 36: IRQ4
irq4:
    cli
    push byte 0
    push byte 36
    jmp irq_common_stub

; 37: IRQ5
irq5:
    cli
    push byte 0
    push byte 37
    jmp irq_common_stub

; 38: IRQ6
irq6:
    cli
    push byte 0
    push byte 38
    jmp irq_common_stub

; 39: IRQ7
irq7:
    cli
    push byte 0
    push byte 39
    jmp irq_common_stub

; 40: IRQ8
irq8:
    cli
    push byte 0
    push byte 40
    jmp irq_common_stub

; 41: IRQ9
irq9:
    cli
    push byte 0
    push byte 41
    jmp irq_common_stub

; 42: IRQ10
irq10:
    cli
    push byte 0
    push byte 42
    jmp irq_common_stub

; 43: IRQ11
irq11:
    cli
    push byte 0
    push byte 43
    jmp irq_common_stub

; 44: IRQ12
irq12:
    cli
    push byte 0
    push byte 44
    jmp irq_common_stub

; 45: IRQ13
irq13:
    cli
    push byte 0
    push byte 45
    jmp irq_common_stub

; 46: IRQ14
irq14:
    cli
    push byte 0
    push byte 46
    jmp irq_common_stub

; 47: IRQ15
irq15:
    cli
    push byte 0
    push byte 47
    jmp irq_common_stub

irq16: ; OUR SYS_CALLS
	cli
	push byte 0
	push byte 48
	jmp irq_common_stub

irq17:
	cli
	push byte 0
	push byte 49
	jmp irq_common_stub

global Sys_Call
Sys_Call:
push ebp
mov ebp, esp
pushad
	mov eax, [ebp + 16]
	mov ebx, [ebp + 12]
	mov ecx, [ebp + 8]
	int 48
popad
pop ebp
ret

extern irq_handler

extern printVal
irq_common_stub:
    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
	push eax
    mov eax, irq_handler
    call eax
    pop eax

    pop gs
    pop fs
    pop es
    pop ds
    popad
	add esp, 8

    ; copy over what is in r
    push eax
	push ebx
		mov ebx, [esp - 20]
		mov eax, [esp + 8]
		mov dword[ebx + 8], eax
		mov eax, [esp + 12]
		mov dword[ebx + 12], eax
		mov eax, [esp + 16]
		mov dword[ebx + 16], eax
	pop ebx
	pop eax

    mov esp, [esp - 28]
    add esp, 8
    iret
[BITS 16]
global APSTART
APSTART:
extern startAPC
call startAPC
jmp $
[BITS 32]

; Here is the definition of our BSS section. Right now, we'll use
; it just to store the stack. Remember that a stack actually grows
; downwards, so we declare the size of the data before declaring
; the identifier '_sys_stack'
SECTION .bss
align 16
    resb (8192*4)               ; This reserves 32KBytes of memory here
sys_stack:
resb 4
