/*
 * LunOS - James Vaughan 2007
 * memory.c
 *
 * This file contains the implementation of the operating system's
 * memory management system.
 */

#include <kern/system.hpp>
#include <kern/Apic.h>
#include <SystemCalls.h>
#include <synchronization.h>

#ifndef DEBUG_MEM
//	#define DEBUG_MEM
#endif

#ifndef DEBUG_DYNAMICMEM
	#define DEBUG_DYNAMICMEM
#endif

extern "C"{
	//We must never go below this line!!! or we will be in the stack
	extern unsigned int memstackTrueBottom;
	//extern unsigned int pageTableStart;
	extern void SetCR3(unsigned int*);
	//Starts i386+ paging
	extern void SetPageFlag (void);
	//stops i386+ paging
	extern void ClearPageFlag (void);
	//Needs to be done to keep data consistant
	extern void UpdateTLBEntry(unsigned int* pointerToAddress);
	// Grab CR0
	extern unsigned int GetCR0(void);

	extern unsigned int GetCR3(void);
}

Lock* Memory::KernalMemoryLock = NULL;
unsigned int KernelMemoryLockAddress;
unsigned int* memstack;
unsigned int* memstackBottom;
unsigned int*** masterPageTable;
struct MemoryData memoryData;
bool Memory::DebugMode = false;

//Our kernel's heap's in place allocation unit
typedef struct Alloc{
	unsigned int length;
	struct Alloc* next;
} Allocation;

//The kernel's free heap
Allocation kheap;

//Free a section back onto the stack
void pushOnMemStack(unsigned int address);
//get a page off of the stack [address to it is stored]
char* popOffMemStack();
//Build Pagetable
void buildPageTable();

void printValidPageEntries()
{
	int i,j;
	printf("---------------------------------------------\n");
    for(j = 0; j < 1024; j++){
    	if(((unsigned int)*(masterPageTable +j) & 0x1) == 0) continue;
    	unsigned int* iner = (unsigned int*)(((unsigned int)*(masterPageTable + j))&0xFFFFF000);
    	printf("MPT[%x] = %x\n",j, (unsigned int)*(masterPageTable +j));
    	timer_wait(25);
    	for(i =0; i < 1024;i++){
    		if((*(iner + i) & 0x1) == 0) continue;
			printf("MPT[%x][%x] = %x\n",j, i, *(iner + i));
			timer_wait(2);
    	}
    }
    printf("-----------------------------------------------\n");
}

//writes a page entry at the given location with the given data
void setupPage(unsigned int* location, unsigned int physical, bool readWrite)
{
	*location = physical | 3;
	UpdateTLBEntry((unsigned int*)physical);
}

void setupPage(unsigned int* location, unsigned int physical, bool readWrite, bool enable){
	*location = physical | (enable?0x3:0x2);
	UpdateTLBEntry((unsigned int*)physical);
}

void Memory::TurnOnDebug()
{
	Memory::DebugMode = true;
}

void Memory::TurnOffDebug()
{
	Memory::DebugMode = false;
}

void Memory::MarkUncacheable(void* virtAddress)
{

	unsigned int first, second;
	Memory::ConvertVirtualAddressToEntries((unsigned int*)virtAddress,&first, &second);
	unsigned int root = (*((unsigned int*)masterPageTable + first) & ~0xFFF);
	Memory::LoadIntoLowVirtualSpace(root);
	*((unsigned int*)0x1c000 + second) |= 0x10;
	UpdateTLBEntry((unsigned int*)root);
}

void Memory::RemoveFromFreeStack(unsigned int* address)
{
	Memory::KernalMemoryLock->GetLock();
	unsigned int* current = memstackBottom;
	while(current < memstack)
	{
		if(address == (unsigned*)*current)
		{
			memcpy(current, current + 1, (memstack - current - 1) / 4);
		}
		current++;
	}
	Memory::KernalMemoryLock->Release();
}

//Sets up the page tables so that we are ready to jump into ownage
void buildPageTable()
{
	int i,j;
	unsigned int addr = 0;
	//This will set the first entry to be one the first lowest page of
	//memory
	unsigned int* innerPage;
	//Now we need to learn how high the kernel needs to address
	//for now the highest thing is the memory stack.  Nothing from it has been
	//used, so we can safely address it with no worries.  And it is full
	//so it will never grow higher than this.
	for(i = 0; addr <= (unsigned int)memstack + 0x100000;i++){
		//grab a new page to use for indexing
		innerPage = (unsigned int*)*(memstackBottom++);
		setupPage((unsigned int*)(masterPageTable + i),(unsigned int)innerPage,true);
		for(j = 0; j < 1024 && addr <= (unsigned int)memstack + 0x100000;j++)
		{
			setupPage((innerPage+j),addr,true);
			//move to the next page
			addr = (addr + 0x1000);
		}
		//Set the rest to not present
		for(;j<1024;j++)
		{
			*(innerPage+j) = 0x02;
		}
	}
	//Set the rest of the pages to off
	for(;i<1024;i++)
	{
		*(masterPageTable + i) = (unsigned int**)(0 | 2);
	}
	//Now we have a 1:1 mapping of everything under the memorystack so
	//when we start up paging the memory addresses will still be the same.
}

//Find the total ammount of pages of available memory
int findPageTotal(memory_map_t* map){
	unsigned int i = 0, total = 0;
	//if the map is non-reserved
	if(map->type == 1){
		if((unsigned)(map->base_addr_low & 0xFFFFF000) != map->base_addr_low)
		{
			map->base_addr_low += 0x1000;
			map->base_addr_low &= 0xFFFFF000;
		}
		for(i = map->base_addr_low; i + 4095 < map->base_addr_low + map->length_low;
		i += 0x1000){
			total++;
		}
	}
	return total;
}

//This builds up the page stack, called on initialization
void divideIntoPages(memory_map_t* map, unsigned int total)
{
	unsigned int i = 0;
	//don't include pages that are before or in the memory stack
	unsigned int AfterHere = (unsigned int)memstackBottom + total * sizeof(int*);
	//if the map is non-reserved
	if(map->type == 1){
		if((unsigned)(map->base_addr_low & 0xFFFFF000) != map->base_addr_low)
		{
			map->base_addr_low += 0x1000;
			map->base_addr_low &= 0xFFFFF000;
		}
		// It has to be 4096, since an allocation counts for all things for 4kb afterwards
		for(i = map->base_addr_low; i + 4096 < map->base_addr_low + map->length_low;
		i += 0x1000){
			//Only add pages that are after our table
			if(AfterHere < i){
				pushOnMemStack(i);
				memoryData.totalPages++;
			}
		}
	}
}

//Called on kernel startup to initialize the memory management
void Memory::initMemoryData(multiboot_info_t* info)
{
	int total = 0;
	memory_map_t* mmap;
	memoryData.higherMemory = info->mem_upper;
	memoryData.kernelPages = 0;
	memoryData.totalPages = 0;
	memoryData.usedPages = 0;
	//Start to fill up the memory stack
	memstack = memstackBottom = (unsigned int *)&memstackTrueBottom;
	//for each memory slice
	for (mmap = (memory_map_t *) info->mmap_addr;
                (unsigned long) mmap < info->mmap_addr + info->mmap_length;
                mmap = (memory_map_t *) ((unsigned long) mmap
                                         + mmap->size + sizeof (mmap->size)))
	{
		total += findPageTotal(mmap);
	}
	for (mmap = (memory_map_t *) info->mmap_addr;
        (unsigned long) mmap < info->mmap_addr + info->mmap_length;
        mmap = (memory_map_t *) ((unsigned long) mmap
                                        + mmap->size + sizeof (mmap->size)))
    {
    	divideIntoPages(mmap,total);
    }
	Memory::KernalMemoryLock = NULL; // null out the lock so we can request pages
    masterPageTable = (unsigned int***)((memstackBottom++)[0]);
	buildPageTable();
    initPaging((unsigned int*)masterPageTable);
    unsigned int numberOfPages = 50000;
    unsigned int sizeOfPage = 0x1000;
    //setup the kheap by throwing it 50 pages
    kheap.length = 0;
    kheap.next = (Allocation*)Memory::requestPages(numberOfPages);
    kheap.next->next = NULL;
    kheap.next->length = (sizeOfPage*numberOfPages) - sizeof(Allocation);
    // now that we have everything ready lets setup our lock
    Memory::KernalMemoryLock = new Lock(&KernelMemoryLockAddress);
}

//Gets a new page of memory
char* Memory::getPage()
{
	return popOffMemStack();
}

//Start paging against the given table
void Memory::initPaging(unsigned int* table)
{
	SetCR3(table);
	SetPageFlag();
}

void Memory::initPaging()
{
	Memory::initPaging((unsigned int*)masterPageTable);
}

/*Stops the paging engine
 * Use extream caution, when doing this you
 * are limited to below 16megs of memory
 * Avoid this at all costs.
 */
void Memory::stopPaging()
{
	ClearPageFlag();
}

bool Memory::IsMasterPageTableUsed(unsigned int entry)
{
	return (unsigned int)masterPageTable[entry] & 0x1;
}

bool Memory::IsSecondaryPageEntryUsed(unsigned int primaryEntry, unsigned int secondaryEntry)
{
	unsigned int masterPageEntry = (((unsigned int)masterPageTable[primaryEntry]) & ~0xFFF);
	LoadIntoLowVirtualSpace(masterPageEntry);
	return *((unsigned int*)0x1c000 + secondaryEntry) & 0x1;
}

unsigned int* Memory::GetVirtualMemoryAddress(unsigned int primaryEntry, unsigned int secondaryEntry)
{
	/*
	 * 4kb pages with 1024 entries means
	 * (0x1000 * 1024 * i) gets you into the right space, then offset by the secondary entry
	 * 4kb pages * j since there are only j of them
	 */
	return (unsigned int*)(0x1000 * 1024 * primaryEntry + 0x1000 * secondaryEntry);
}

unsigned int Memory::ReadPageDirectory(unsigned int i)
{
	return *((unsigned int*)masterPageTable + i);
}

unsigned int Memory::ReadInnerPageEntry(unsigned int i, unsigned int j)
{
	unsigned int root = (*((unsigned int*)masterPageTable + i)) & ~0xFFF;
	LoadIntoLowVirtualSpace(root);
	return *((unsigned int*)0x1c000 + j);
}

/**
 * Finds the starting point of a gap in the virtual tree the
 * size of pages in pages.
 */
unsigned int* Memory::walkTree(unsigned int pages)
{
	unsigned int* Start = NULL;
	unsigned int originalPagesNeeded = pages;
	for(unsigned int i = 1; i < 1024; i++)
	{
		if(IsMasterPageTableUsed(i))
		{
			for(unsigned int j = 0; j < 1024; j++)
			{
				if(IsSecondaryPageEntryUsed(i,j))
				{
					Start = NULL;
					pages = originalPagesNeeded;
				}
				else
				{
					if(Start == NULL)
					{
						Start = Memory::GetVirtualMemoryAddress(i,j);
					}
					pages--;
				}
				if(pages == 0)
				{
					return Start;
				}
			}
		}
		else
		{
			// Then there is no page here yet, so we can use all of it
			if(Start == NULL)
			{
				Start = Memory::GetVirtualMemoryAddress(i,0);
			}
			if(pages < 1024)
			{
				return Start;
			}
			pages -= 1024;
		}
	}
	// We can not just return NULL here since we might have just run out of virtual room for the allocation
	return NULL;
}

void Memory::LoadIntoLowVirtualSpace(unsigned int physicalAddress)
{
	((unsigned int*)((unsigned int)masterPageTable[0] & ~0xFFF))[0x1c] = (physicalAddress & ~0xFFF) | 3;
	SetCR3((unsigned int*)masterPageTable);
}

void Memory::ConvertVirtualAddressToEntries(unsigned int* address, unsigned int* i, unsigned int* j)
{
	// 0xFFFF F000
	// (8 + 2)|(2 + 8)|[offset == 12] (totals to 32)
	*i = ((unsigned int)address << 0 ) >> 22; // 32 - (10)
	*j = (((unsigned int)address) >> 12) & 0x3FF; //
}

void Memory::WritePageDirectory(unsigned int i, unsigned int value)
{
	*((unsigned int*)masterPageTable + i) = (value & ~0xfff)|3;
}

/**
 * Links a page into the virtual memory
 * page = physical
 * virt = virtual address
 * Paging is to be turned off when calling this
 */
bool Memory::linkPage(unsigned int* page, unsigned int* virt)
{
	unsigned int i,j;
	unsigned int entry = NULL;
	ConvertVirtualAddressToEntries(virt, &i,&j);
	if(!IsMasterPageTableUsed(i))
	{
		entry = (unsigned int)getPage();
		Memory::WritePageDirectory(i, entry);
		LoadIntoLowVirtualSpace(entry);
		for(unsigned int k = 0; k < 1024;k++)
		{
			*((unsigned int*)0x1c000 + k) = 0x2;
		}
		*((unsigned int*)0x1c000 + j) = (((unsigned int)page) & ~0xFFF) | 3;
		SetCR3((unsigned int*)masterPageTable);
	}
	else
	{
		LoadIntoLowVirtualSpace((unsigned int)masterPageTable[i]);
		// add in our new page
		*((unsigned int*)0x1c000 + j) = (((unsigned int)page) & ~0xFFF) | 3;
	}
	return true;
}

bool Memory::linkPages(unsigned int* startVirtual, unsigned int numberOfPages)
{
	unsigned int i,j;
	unsigned int entry = NULL;
	bool first = true;
	ConvertVirtualAddressToEntries(startVirtual, &i,&j);
	printf("Start Virtual 0x%x for %i pages\n", startVirtual, numberOfPages);
	while(numberOfPages)
	{
		if(first && !IsMasterPageTableUsed(i))
		{
			entry = (unsigned int)getPage();
			printf("Writing 0x%x to page directory %i\n", entry, i);
			Memory::WritePageDirectory(i, entry);
			SetCR3((unsigned int*)masterPageTable);
			LoadIntoLowVirtualSpace(entry);
			for(unsigned int k = 0; k < 1024;k++)
			{
				*((unsigned int*)0x1c000 + k) = 0x2;
			}
		}
		else if(first)
		{
			LoadIntoLowVirtualSpace((unsigned int)masterPageTable[i]);
		}
		*((unsigned int*)0x1c000 + j) = (((unsigned int)getPage()) & ~0xFFF) | 3;
		first = false;
		j++;
		if(j == 1024)
		{
			i++;
			j = 0;
			first = true;
		}
		numberOfPages--;
	}
	SetCR3((unsigned int*)masterPageTable);
	return true;
}


/*
 * Returns the starting virtual address
 */
unsigned int* Memory::requestPages(unsigned int pages)
{
	Memory::KernalMemoryLock->GetLock();
	unsigned int* start =  Memory::walkTree(pages);
	if(start == NULL)
	{
		Memory::KernalMemoryLock->Release();
		return NULL;
	}
	//Link each page into the tree
	for(unsigned int i = 0; i < pages; i++)
	{
		unsigned int* page = (unsigned int*)getPage();
		Memory::linkPage(page, start + (1024* i));
	}
	// TODO: This method is far nicer to the CPU,
	// but we need to make sure the regular link works as well
	//Memory::linkPages(start,pages);
	Memory::KernalMemoryLock->Release();
	return start;
}

unsigned int* Memory::walkTree(Process* p, unsigned int pages)
{
	//TODO: Implement this for a regular process
	return NULL;
}

//kernel malloc
unsigned char* kmalloc(unsigned int size)
{
	//Look at kheap to see if we can service this
	//if we can't ask for some more pages and then service
	Allocation* current = kheap.next;
	Allocation* prev = NULL;
	//32 is the smallest allocation
	if(size < 32 + sizeof(Allocation))
	{
		size = 32 + sizeof(Allocation);
	}
	//make sure there is something here
	while(current != NULL){
		if(size + sizeof(Allocation) <= current->length)
		{
			//granularity of 32 bits [if it isn't large enough, just give it all]
			if(size + sizeof(Allocation) >= current->length - 32)
			{
				// If there is nothing before us update where the heap starts
				if(current == kheap.next)
				{
					kheap.next = current->next;
				}
				else
				{
					//erase ourselves from the chain
					prev->next = current->next;
				}
				return (unsigned char*)((unsigned int)current + sizeof(Allocation));
			}
			else
			{
				// We need to make a new section to get this all to work
				unsigned int allocSize = size + sizeof(Allocation);
				Allocation* nAlloc = (Allocation*)((unsigned int)current + current->length - allocSize);
				nAlloc->length = size;
				nAlloc->next = NULL;
				current->length -=  allocSize;
				// we need to push the pointer 1 allocation higher so people don't write on our allocation
				return (unsigned char*)((unsigned int)nAlloc + sizeof(Allocation));
			}

		}
		// If it doesn't fit here, look at the next available section of memory
		prev = current;
		current = current->next;
	}
	printf("Unable to do allocated, we require additional memory!\n");
    __asm__ __volatile__ ("hlt");
	if(current == NULL){
		//TODO: allocate more pages
		Memory::KernalMemoryLock->Release();
		Allocation* newPages = (Allocation*)Memory::requestPages(4);
		newPages->next = NULL;
		newPages->length = (4096*4) - sizeof(Allocation);
		prev->next = newPages;
		unsigned char* result = kmalloc(size);
		if(result != NULL)
		{
			return result;
		}
	}
	printf("\n%3OUT OF MEMORY!!!!!! System Halted");
    for (;;) {
        __asm__ __volatile__ ("hlt");
    }
	return NULL;
}

void Memory::printDynamicMemoryState(void)
{
	unsigned int i, totalFree = 0;
	Allocation* curAlloc = kheap.next;
	for(i = 0;curAlloc; i++)
	{
		printf("Allocation %i: %x",i, curAlloc);
		totalFree += curAlloc->length;
		printf(", Length=%2%ibytes\n",curAlloc->length);
		if((curAlloc >= curAlloc->next) && (curAlloc->next != NULL))
		{
			printf("%4WARNING: ALLOCATION MEMORY LOOP!!!!\n");
			return;
		}
		curAlloc = curAlloc->next;
	}
	//printf("Total Free Memory: %2%i%1 bytes\n", totalFree);
}

void Memory::printMemoryAllocation(void* address)
{
	Allocation* al = (Allocation*)address;
	printf("length:%2%i %1next:%2%x\n",al->length,al->next);
}

//kernel free
unsigned char* kfree(unsigned char* variable)
{

	//Figure out where this was from
	//Insert the page back into the heap
	//try to combine it with another entry
	//if we have a totally free page try to free some up
	Allocation* avar = (Allocation*)((unsigned int)variable - sizeof(Allocation));
	Allocation* current = kheap.next;
	//test for the front
	if(avar < current)
	{
		//combine with the next
		if((unsigned int)avar + avar->length == (unsigned int)current)
		{
			// We can absorb the current first with our new deallocated variable
			avar->length += current->length + sizeof(Allocation);
			// We now point passed the current
			avar->next = current->next;
		}
		else
		{
			// we need to point to the current one
			avar->next = current;
		}
		// And now we are at the front of the pack
		kheap.next = avar;
		return (unsigned char*)variable;
	}

	// Since it isn't before our first, now look past the current to see if it is there
	while(current)
	{
		// if there is another allocation farther along...
		if(current->next)
		{
			//see if the allocation was inside
			if((current < avar) && (avar < current->next))
			{
				char type = 0;
				unsigned int endOfAvar = (unsigned int)(((unsigned int)avar) + avar->length + sizeof(Allocation));
				//if it consumes the next one
				if(endOfAvar >= (unsigned int)current->next)
				{
					type = 1;
				}
				// Check to see if avar can be consumed by the current
				if((unsigned int)avar <= (unsigned int)current + current->length + sizeof(Allocation))
				{
					type += 2;
				}
				// Now execute the right type of intermediate free
				switch(type)
				{
				// then it is a new segment
				case 0:
					avar->next = current->next; // we know that current->next != NULL
					current->next = avar;
					break;
				//It consumes current->next
				case 1:
					avar->length += current->next->length + sizeof(Allocation);
					avar->next = current->next->next;
					current->next = avar;
					break;
				// current consumes avar
				case 2:
					current->length += avar->length + sizeof(Allocation);
					// the current->next remains the same, so we don't need to fix any pointers
					break;
				// current consumes avar and current->next
				case 3:
					current->length += avar->length + current->next->length + (sizeof(Allocation) + sizeof(Allocation));
					current->next = current->next->next;
					break;
				}
				return variable;
			}
		}
		// if there is nothing past this
		else
		{
			if((unsigned int)current + current->length + sizeof(Allocation) >= (unsigned int)avar)
			{
				//printf("Absorbing the released variable\n");
				// this is redundant, but self healing
				current->next = NULL;
				current->length += avar->length + sizeof(Allocation);
				// we already know that the next element is null for current
			}
			else
			{
				//printf("Releasing something at the far end\n");
				avar->next = NULL;
				//avar->length... doesn't change
				current->next = avar;
				// and there is nothing to point to this to need to backlink
			}
			return (unsigned char*)variable;
		}

		// current->next != NULL
		if(current >= current->next)
		{
			printf("%3ERROR:DYNAMIC MEMORY LOOP!!!\n");
			return NULL;
		}
		current = current->next;
	}

	//Then this is the only allocation ?? and it was returned [should never occur]
	if(current == NULL){
		kheap.next = avar;
		return variable;
	}
	// This case can never occur
	return NULL;
}

//Free a section back onto the stack
void pushOnMemStack(unsigned int address)
{
	//int seconds;
	*(memstack) = address;
	memstack++;
}
//get a page off of the stack [address to it is stored]
char* popOffMemStack(){
	return ((memstack - 1) > memstackBottom ? (char*)(*--memstack) : NULL);
}

//so we can use new sz in bytes
void* operator new(unsigned int sz)
{
	Memory::KernalMemoryLock->GetLock();
	void* ret = kmalloc(sz);
	Memory::KernalMemoryLock->Release();
	return ret;
}

//So we can have dynamic arrays in bytes
void* operator new[](unsigned int sz)
{
	Memory::KernalMemoryLock->GetLock();
	void* ret = kmalloc(sz);
	Memory::KernalMemoryLock->Release();
	return ret;
}

//so we can use delete
void operator delete(void* variable)
{
	Memory::KernalMemoryLock->GetLock();
	kfree((unsigned char*)variable);
	Memory::KernalMemoryLock->Release();
}

//So we can use delete
void operator delete[](void* variable)
{
	Memory::KernalMemoryLock->GetLock();
	kfree((unsigned char*)variable);
	Memory::KernalMemoryLock->Release();
}

void* Memory::GetLocalAddress(void* address)
{
	unsigned int i,j;
	unsigned int offset = ((unsigned int)address) & 0xFFF;
	Memory::ConvertVirtualAddressToEntries((unsigned int*) address, &i, &j);
	return (void*)((Memory::ReadInnerPageEntry(i,j) & ~0xFFF) + offset);
}

unsigned int*** Memory::CreateAddressSpace()
{
	//TODO: Actually give it its own address space
	return masterPageTable;
}

void Memory::ReleaseAddressSpace(unsigned int*** memoryAddress)
{
	//TODO: Actually release the address space
}


