#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include <kern/system.h>
#include <synchronization.h>

class Scheduler
{
private:
	LinkedList* activeList;
	LinkedList* ioBlocked;
	LinkedList* timerBlocked;
	LinkedList* zombies;
	volatile Node* activeThread;
	void SwitchToNext();
	void printDiagnostics();
	void WakeUp();
	void RemoveActiveThread();
public:
	// Get the scheduler ready REQUIRES Memory Management to be active
	Scheduler();
	// Not sure why you'd do this, but we will code it anyways
	~Scheduler();
	// Do what needs to be done to idle the system if nothing needs to run
	void HandelIdleThread();
	// For initial startup only
	void SetInitialProcess(Process* process);
	// Add a new thread to be scheduled
	void AddThread(Thread* thread);
	// Create a new Thread
	unsigned int CreateThread(unsigned char* name, void* startingAddress, void* parameterPointer);
	// Kill this thread
	void KillThisThread();
	// Kill a thread
	void KillThread(Thread* thread);
	// Kill a process
	void KillProcess(unsigned int pID);
	// Kill this process
	void KillActiveProcess();

	// Give up the CPU for another process [good practice for CPU intensive programs]
	void Yield();
	// Specifically to be used by the timer to allow for pre-emption
	void YieldIfNotBusy();
	// Block for a given amount of time
	void Sleep(unsigned int ms);

	// Block the current thread on an I/O wait
	void IOBlock();
	//Resume the given thread from an I/O wait
	void IOResume(Thread* thread);

	// Gets the active thread
	Thread* GetActiveThread();

	// Gets the active process
	Process* GetActiveProcess();

	void PrintOutThreads();

	// Terminate this process
	void Exit(int code);
	// Terminates the thread
	void ExitThread(int code);
	// Saves the registers of the current thread
	void SaveRegisters(struct regs* r);
};

extern Scheduler* Sched;


#endif /*SCHEDULER_H_*/
