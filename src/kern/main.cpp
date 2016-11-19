/*
 * main.c - James Vaughan
 *
 * This file holds startC, the start of C/C++ code.
 * It init's the rest of the systems.
 * Some of the memory type functions are also here
 * though they should be moved to another file at some point.
 */
#include <kern/system.hpp>
#include <kern/grub.h>
#include <kern/memory.h>
#include <kern/Apic.h>
#include <kern/process.hpp>
#include <prompt.h>
#include <SysCall.h>

void *memcpy(void *dest, const void *src, size_t count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; count > 0; count--) *(dp++) = *(sp++);
    return dest;
}

extern "C" void *memset(void *dest, char val, size_t count)
{
    char *temp = (char *)dest;
    for( ; count > 0; count--) *temp++ = val;
    return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count)
{
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

extern "C" void * memmove(void * dst, const void * src, size_t len)
{
	memcpy(dst, src, len);
	return dst;
}

size_t strlen(const char *str)
{
    size_t retval;
    for(retval = 0; *str != '\0'; str++) retval++;
    return retval;
}

void strcpy(char* a, const char* b)
{
	do
	{
		*(a++) = *b;
	}
	while(*(b++) != 0);
}

int strcmp(const char* a, const char* b)
{
	for(; (*a & *b); a++, b++)
	{
		if(*a != *b)
			return *a - *b;
	}
	return 0;
}

int strncmp(const char* a, const char* b, unsigned int size)
{
	unsigned int i;
	for(i = size; (*a & *b) & (i); i--, a++, b++)
	{
		if(*a != *b)
			return *a - *b;
	}
	return (i == size? 0 : *a - *b);
}

unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

unsigned short inportw (unsigned short _port)
{
    unsigned short rv;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outportw (unsigned short _port, unsigned short _data)
{
	__asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

unsigned int inportd (unsigned short _port)
{
    unsigned int rv;
    __asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportd (unsigned short _port, unsigned int _data)
{
	__asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

bool CheckSum(unsigned char* data, unsigned int size)
{
	unsigned char total = 0;
	int i;
	for(i = size; i > 0; i--)
	{
		total += *(data++);
	}
	return (total == 0);
}

extern "C"{

void __cxa_pure_virtual()
{
	using namespace LunOS;
	Thread* ourThread = Sched->GetActiveThread();
	if(ourThread == NULL)
	{
		printf("%3ERROR: The active thead is NULL!!!\nRemaking the Idle Thread\n");
		LunOS::System::CreateThread((unsigned char*)"New Idle Thread",
				(unsigned int(*)(void*))IdleThread,
				NULL );
		LunOS::System::Yield();
	}
	else
	{
		printf("%3ERROR:%s called a pure function and will be terminated!\n", ourThread->name);
		Sched->KillThisThread();
	}
}

extern void InitFPU();

int startAPC()
{
	printf("Hello World!!!");
    for (;;) {
        __asm__ __volatile__ ("hlt");
    }
	return 0;
}

//Basically main, but changed the name to avoid stupid warnings
int startC(multiboot_info_t *boot_info)
{
	__asm__ __volatile__ ("cli");
	memsetw((unsigned short*)0x0, 0xdead, 0x20);
	//turn off the floppy in case we came from there... (not for many years now)
	outportb(0x3f2,0x00);
    gdt_install();
    idt_install();
    InitFPU();
	initConsole(); // get something ready from the console for debugging
    isrs_install();
    irq_install();
    Memory::initMemoryData(boot_info);
    initConsolePhase2(); // now that we have memory we can use it
	CPU::InitCPUs();
	Process::initProcesses();
    timer_install();
    keyboard_install();
    new Scheduler();
    powerManagement.Init();
    System::Init();
	__asm__ __volatile__ ("sti");

//	unsigned int i = 0;
//Kind of like an idle thread
    for (;;) {
        __asm__ __volatile__ ("hlt");
    }
    return 1;
}//end of startC
}//end of the externC



