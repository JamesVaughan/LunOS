#ifndef SYNCHRONIZATION_H_
#define SYNCHRONIZATION_H_
class Lock;

class Lock
{
protected:
	void* addr;
	// This needs to be 32 bits otherwise the lock will fail!
	unsigned int LocalAddress;
	bool given;
public:
	// Create a new lock
	Lock();
	// Make a lock at a given address
	Lock(void* addr);
	~Lock();
	// Lock the lock
	void GetLock();
	// Release the lock
	void Release();
	// See if you got the lock [actually locks if it succeeds]
	bool TryLock();
	// Allows us to change the address so 2 locks can share the same address
	void BindAddresses(Lock* lock);
	// Used for debugging
	unsigned int GetLockValue();
};

#endif /*SYNCHRONIZATION_H_*/
