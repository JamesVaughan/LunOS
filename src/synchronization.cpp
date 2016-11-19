#include <synchronization.h>
#include <SysCall.h>

typedef int size_t;

#define LockFree 0
#define LockUsed 1

extern "C"
{
unsigned int asmTryLock(volatile void* addr);
}

Lock::Lock()
{
	this->given = false;
	this->addr = &this->LocalAddress;
	*(unsigned char*) this->addr = LockFree;
}

Lock::Lock(void* addr)
{
	this->given = true;
	this->addr = addr;
	// Initialize our local variable anyways
	this->LocalAddress = LockFree;
	*((unsigned char*) this->addr) = LockFree;
}

Lock::~Lock()
{
	if (!this->given)
	{
		delete ((size_t*) this->addr);
	}
}

void Lock::GetLock()
{
	// check to see if we are looking at a NULL, this way we don't screw up the memory initialization
	if (!this)
	{
		return;
	}
	// a spin lock
	while (asmTryLock(this->addr) == LockUsed)
	{
		LunOS::System::Yield();
	}
}

void Lock::Release()
{
	// check to see if we are looking at a NULL, this way we don't screw up the memory initialization
	if (!this)
	{
		return;
	}
	*((volatile unsigned char*) this->addr) = LockFree;
}

bool Lock::TryLock()
{
	// check to see if we are looking at a NULL, this way we don't screw up the memory initialization
	if (!this)
	{
		return true;
	}
	return (asmTryLock(this->addr) == LockFree);
}

void Lock::BindAddresses(Lock* lock)
{
	if (!this->given)
	{
		delete (unsigned char*) this->addr;
	}
	this->given = false;
	this->addr = lock->addr;
}

unsigned int Lock::GetLockValue()
{
	return (unsigned int)(*((unsigned char*)this->addr));
}

