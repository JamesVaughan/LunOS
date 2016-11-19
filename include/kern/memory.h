#ifndef MEMORY_H_
#define MEMORY_H_ 0

/*
 * LunOS - James Vaughan 2007
 * memory.c
 *
 * This file contains the implementation of the operating system's
 * memory management system.
 */
#include <kern/process.hpp>
#include <kern/system.hpp>
#include <kern/grub.h>
#include <synchronization.h>

//A structure to hold our memory data
struct MemoryData
{
	unsigned long higherMemory;
	unsigned int totalPages;
	unsigned int usedPages;
	unsigned int kernelPages;
};

extern struct MemoryData memoryData;
//Not sure why Process isn't be declared in process.h
class Process;

class Memory
{
	public:
	//Used by kmalloc to get pages from the kernel's stack
	static unsigned int* requestPages(unsigned int pages);
	//Used by a userLevel malloc to get pages from the kernel's stack
	static unsigned int* requestPages(unsigned int pages, Process* p);
	//Goes through the kernel space and finds size free virtual addresses
	static unsigned int* walkTree(unsigned int pages);
	//Goes through the process space and finds pages free for virtual addresses
	static unsigned int* walkTree(Process* p, unsigned int pages);
	//Links a page into the OS's virtual memory
	static bool linkPage(unsigned int* page, unsigned int* virt);
	//Links a page into the given process' virtual memory
	static bool linkPage(unsigned int* page, unsigned int* virt, Process* p);
	//Fills out a serious of virtual addresses with new physical addresses
	static bool linkPages(unsigned int* startVirtual, unsigned int numberOfPages);
	//Unlink a page from the kernel pages
	static void unlinkPage(unsigned int* virt);
	//Unlink a page from the user pages
	static void unlinkPage(unsigned int* virt, Process* p);
	//Starts up the memory management system
	static void initMemoryData(multiboot_info_t* info);
	//Gets a new page of memory [may be excluded in a later version]
	static char* getPage();
	//Start paging against the given table
	//This may become private later on
	static void initPaging(unsigned int* table);
	// Start the paging engine with the previous table
	static void initPaging();
	//Stops the paging engine
	static void stopPaging();
	//Prints out the dynamic memory state
	static void printDynamicMemoryState();
	//Prints out the information of the memory allocation
	static void printMemoryAllocation(void* address);
	// Make sure an address currently linked in is not cacheable
	static void MarkUncacheable(void* virtAddress);
	// Returns the kernel's memory address for the given address of the current thread
	static void* GetLocalAddress(void* address);
	// Remove a given address from the free memory stack
	static void RemoveFromFreeStack(unsigned int* address);
	// Creates a new address space for the process
	static unsigned int*** CreateAddressSpace();
	static void ReleaseAddressSpace(unsigned int*** memoryAddress);
	static void TurnOnDebug();
	static void TurnOffDebug();
	static Lock* KernalMemoryLock;
	private:
	static bool IsMasterPageTableUsed(unsigned int entry);
	static bool IsSecondaryPageEntryUsed(unsigned int primaryEntry, unsigned int secondaryEntry);
	static unsigned int* GetVirtualMemoryAddress(unsigned int primaryEntry, unsigned int secondaryEntry);
	static void ConvertVirtualAddressToEntries(unsigned int* virt, unsigned int* i,unsigned int* j);
	static void LoadIntoLowVirtualSpace(unsigned int physicalAddress);
	static unsigned int ReadInnerPageEntry(unsigned int i, unsigned int j);
	static unsigned int ReadPageDirectory(unsigned int i);
	static void WritePageDirectory(unsigned int i, unsigned int value);
	static bool DebugMode;
};

//kernel malloc
unsigned char* kmalloc(unsigned int size);

//kernel free
unsigned char* kfree(unsigned char* variable);

//so we can use new
void* operator new(unsigned int sz);
void* operator new[](unsigned int sz);
//so we can use delete
void operator delete(void* variable);

#endif /*MEMORY_H_*/

