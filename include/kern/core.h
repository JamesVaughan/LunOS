#ifndef CORE_H_
#define CORE_H_

#ifndef size_t
#define size_t unsigned int
#endif

/* This defines what the stack looks like after an ISR was running */
typedef struct regs
{
    volatile unsigned int gs, fs, es, ds;
    volatile unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    volatile unsigned int int_no, err_code;
    volatile unsigned int eip, cs, eflags;
}__attribute__((packed)) regs;


/* MAIN.C */
extern void *memcpy(void *dest, const void *src, size_t count);
extern "C" void *memset(void *dest, char val, size_t count);
extern unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count);
extern size_t strlen(const char *str);
extern unsigned char inportb (unsigned short _port);
extern void outportb (unsigned short _port, unsigned char _data);

extern unsigned short inportw (unsigned short _port);
extern void outportw (unsigned short _port, unsigned short _data);

extern void outportd (unsigned short _port, unsigned int _data);
extern unsigned int inportd (unsigned short _port);

extern int strcmp(const char* a, const char* b);
extern int strncmp (const char* a, const char* b, unsigned int size);
extern void strcpy(char* a, const char* b);

extern bool CheckSum(unsigned char* data, unsigned int size);

/* GDT.C */
extern void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_install();

/* IDT.C */
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
extern void idt_install();

/* ISRS.C */
extern void isrs_install();

/* IRQ.C */

extern void irq_install_handler(int irq, void (*handler)(struct regs *r));
extern void irq_uninstall_handler(int irq);
extern void irq_install();

/* TIMER.C */
extern void timer_wait(int ticks);
extern void timer_install();
extern volatile unsigned int timer_ticks;

/*FP.asm*/
extern "C"
{
	typedef struct sseRegs
	{
		unsigned int Offset;
		char fxsave_region[512 + 16];
	}__attribute__((aligned(16))) sseRegs;

	extern unsigned int LoadSSEStatus(void);
	extern void StoreSSEStatus(unsigned int newStatus);
	// 16-bit aligned address to store the floating point variables.
	extern void fxSave(char* address);
	// 16-bit aligned address to copy the floating point variables from into registers.
	extern void fxStore(char* address);
}

extern "C"
{
	extern unsigned int GetCPUID(unsigned int call, unsigned int*a, unsigned int*d);
	extern void ReadMSR(unsigned int registerNumber, long long* value);
	extern void WriteMSR(unsigned int registerNumber, long long* value);
}


#endif /*CORE_H_*/
