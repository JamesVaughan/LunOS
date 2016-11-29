#ifndef PROCESS_H_
#define PROCESS_H_

class Thread;
class CPU;
class Process;

#include <kern/system.hpp>
#include <kern/user.h>
#include <kern/io/io.h>
#include <Stdlib.h>

/**
 * Represents a process
 */
class Process
{
	private:
		//Yes a triple unsigned int table for working with the pages
		unsigned int*** masterPageTable; // is the same for all Threads in a process
	public:
		Process(User* owner);
		~Process();
		LinkedList* threads;
		User* owner;
		static void initProcesses();
		void* GetLocalMemoryAddress(short int upper, short int lower);
		bool SwitchToUser(unsigned int uid, const char* password);
		FDT fdt;
		unsigned int pid;
};

/**
 * Represents a cpu
 */
class CPU
{
	public:
	//The speed of this CPU
	unsigned int mhz;
	//The Active thread for the given CPU
	Thread* active;
	//The List of activated CPU's on the system
	static LinkedList* cpuList;
	//The number of CPU's on the system
	static unsigned int cpus;
	// Detect and load all of our CPU's
	static void InitCPUs();
};


/**
 * Represents a thread
 */
class Thread
{
	private:
	Process* proc;
	public:
	unsigned char name[64];
	unsigned int priority;
	unsigned int tid;
	unsigned char* Stack;
	unsigned int WakeupAt;
	bool inKernelCall;
	Thread(unsigned char* name, Process* proc, unsigned int Start, unsigned char* stack, unsigned int stackSize, void* params);
	Process* GetProcess();
	~Thread();
	LunOS::DataStructures::FixedStack<struct regs> CalledStack;
	LunOS::DataStructures::FixedStack<struct sseRegs> CalledStackSSE;
};


#endif /*PROCESS_H_*/
