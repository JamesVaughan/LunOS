#include <kern/scheduler.h>
#include <kern/console.h>
#include <SysCall.h>
Scheduler* Sched;
Process* InitialProcess;
Lock* schedLock;
Lock* ioLock;
Lock* zombieLock;

#ifndef NULL
#define NULL 0
#endif

Scheduler::Scheduler()
{
	this->activeList = new LinkedList();
	this->ioBlocked = new LinkedList();
	this->timerBlocked = new LinkedList();
	this->zombies = new LinkedList();
	this->activeThread = NULL;
	schedLock = new Lock();
	ioLock = new Lock();
	zombieLock = new Lock();
	Sched = this;
}

Scheduler::~Scheduler()
{
	delete this->activeList;
	delete this->ioBlocked;
	delete this->timerBlocked;
	delete this->zombies;
	delete schedLock;
	delete ioLock;
	delete zombieLock;
	Sched = NULL;
}

void Scheduler::HandelIdleThread()
{
	if(this && this->activeList && this->activeList->Length > 1)
	{
		LunOS::System::Yield();
	}
	else
	{
		LunOS::System::Yield();
		// run a check again so we don't lag behind for interrupts
		if( (this != NULL) & (this->activeList != NULL) & (this->activeList->Length == 1))
		{
			__asm__ __volatile__ ("sti");
			__asm__ __volatile__ ("hlt");
		}
	}
}

void Scheduler::AddThread(Thread* thread)
{
	Node* newLast = new Node(thread);
	schedLock->GetLock();
	this->activeList->AddLast(newLast);
	newLast->next = this->activeList->Root;
	this->activeList->Root->prev = newLast;
	schedLock->Release();
}

void Scheduler::PrintOutThreads()
{
	// Grab all of the locks so we can do stuff
	zombieLock->GetLock();
	ioLock->GetLock();
	schedLock->GetLock();
	// There is no point to allow this core to do a context switch since we have all of the locks
	printf("%2Running Threads\n");
	Node* current = this->activeList->Root;
	do
	{
		Thread* currentThread = (Thread*)current->data;
		printf("%s:%i\n",currentThread->name, currentThread->tid);
		current = current->next;
	}while(current != this->activeList->Root);
	printf("%2IO Blocked Threads\n");
	current = this->ioBlocked->Root;
	do
	{
		if(current == NULL) break;
		Thread* currentThread = (Thread*)current->data;
		printf("%s:%i\n",currentThread->name, currentThread->tid);
		current = current->next;
	}while(current != this->ioBlocked->Root);

	printf("%2Sleeping Threads: Time Remaining(ms)\n");
	current = this->timerBlocked->Root;
	do
	{
		if(current == NULL) break;
		Thread* sleepingData = (Thread*)current->data;
		printf("%s:%i:%ims\n",sleepingData->name, sleepingData->tid, (sleepingData->WakeupAt - (unsigned int)timer_ticks));
		current = current->next;
	}while(current != this->timerBlocked->Root);
	schedLock->Release();
	ioLock->Release();
	zombieLock->Release();
	// now that we are not hogging everything let the timer elapse
}

void Scheduler::SetInitialProcess(Process* process)
{
	InitialProcess = process;
}

unsigned int Scheduler::CreateThread(unsigned char* name, void* startingAddress, void* parameterPointer)
{
	unsigned char* stack = (new unsigned char[0x10000]);
	if(stack == NULL)
	{
		printf("%3ERROR, no memory to create a new thread stack!\n");
		return 0;
	}
	Process* proc = Sched->GetActiveProcess();
	proc = (proc == NULL?InitialProcess:proc);
	Thread* nThread = new Thread(name, proc,(unsigned int)startingAddress, stack + 0x10000,0x10000, parameterPointer);
	AddThread(nThread);
	this->PrintOutThreads();
	return nThread->tid;
}

void Scheduler::KillThisThread()
{
	Thread* threadThatDies = ((Thread*)this->activeThread->data);
	this->KillThread(threadThatDies);
}

void Scheduler::KillThread(Thread* thread)
{
	if(thread == NULL)
	{
		printf("%3Attempted to kill a thread with an invalid address!\n");
		return;
	}
	Memory::KernalMemoryLock->GetLock();
	schedLock->GetLock();
	// check to see what case we are in, deleting the current thread or not.
	if(thread == this->activeThread->data)
	{
		this->activeList->DeleteNode((Node*)this->activeThread, false);
		// now that no reference to us exists anymore free the node and ourselves.
		kfree( (unsigned char*)this->activeThread );
		kfree( (unsigned char*)thread );
	}
	else
	{
		Node* current = this->activeList->Root;
		bool first = true;
		while((current != this->activeList->Root) | (first))
		{
			if((Thread*)current->data == thread)
			{
				this->activeList->DeleteNode(current, false);
				kfree ((unsigned char*) current);
				kfree ((unsigned char*) thread);
				break;
			}
			current = current->next;
		}
	}
	this->activeList->Root->prev = this->activeList->Tail;
	this->activeList->Tail->next = this->activeList->Root;
	Sched->SwitchToNext();
	// we need to release the memory lock first
	Memory::KernalMemoryLock->Release();
	schedLock->Release();
}

void Scheduler::Exit(int code)
{
	// we will set the return value here before we kill at some point
	this->KillThread((Thread*)this->activeThread->data);
}

void Scheduler::KillProcess(unsigned int pid)
{
	schedLock->GetLock();
	// Find the process from the process list
	// for each thread, kill it
	schedLock->Release();
}

void Scheduler::KillActiveProcess()
{
	// find this process' ID and then kill it (We do not need to lock since we will always be currently running
	KillProcess(((Thread*)(this->activeThread->data))->tid);
}

// THERE MUST ALWAYS BE A THREAD TO RUN [an IDLE thread at least]
void Scheduler::SwitchToNext()
{
	this->activeThread = this->activeThread->next;
}

void Scheduler::Yield()
{
	if(!schedLock)
	{
		SwitchToNext();
	}
	else
	{
		if(schedLock->TryLock())
		{
			SwitchToNext();
			schedLock->Release();
		}
	}
}

void Scheduler::YieldIfNotBusy()
{
	if(schedLock->TryLock())
	{
		// Update IO
		WakeUp();
		// Switch to Next
		SwitchToNext();
		schedLock->Release();
	}
}

// Call this only while having the scheduler Lock
void Scheduler::WakeUp()
{
	Node* current;
	// Wake up our sleeping threads, if it is time
	if(this->timerBlocked->Root != NULL && ((Thread*)((current = this->timerBlocked->Root)->data))->WakeupAt <= (unsigned int)timer_ticks)
	{
		this->timerBlocked->Root = current->next;
		this->timerBlocked->Length--;
		if(current->next == NULL)
		{
			this->timerBlocked->Tail = NULL;
		}
		// Put it back in the active thread list
		current->prev = (Node*)this->activeThread;
		current->next = this->activeThread->next;
		this->activeThread->next->prev = current;
		this->activeThread->next = current;
	}
}

void Scheduler::Sleep(unsigned int ms)
{
	// for now, just skip along
	schedLock->GetLock();
	// Make sure this can be done all in 1 shot since a user program calls this
	Node* nextToBeActive = this->activeThread->prev;
	RemoveActiveThread();
	Thread* sleep = (Thread*)this->activeThread->data;
	sleep->WakeupAt = ((ms * 18) / 1000) + timer_ticks;

	this->activeThread->data = sleep;
	Node* current = this->timerBlocked->Root;
	while(current != NULL)
	{
		if(((Thread*)current->data)->WakeupAt >= sleep->WakeupAt)
		{
			break;
		}
		current = current->next;
	}
	if(current != NULL)
	{
		// if we come first inject it to be the first
		if(this->timerBlocked->Root == current)
		{
			this->timerBlocked->Root = (Node*)this->activeThread;
		}
		this->activeThread->prev = current->prev;
		current->prev->next = (Node*)this->activeThread;
		this->activeThread->next = current;
		current->prev = (Node*)this->activeThread;
	}
	else
	{
		if(this->timerBlocked->Root == NULL)
		{
			this->timerBlocked->Root = (Node*)activeThread;
		}
		activeThread->next = NULL;
		activeThread->prev = this->timerBlocked->Tail;
		this->timerBlocked->Tail->next = (Node*)activeThread;
		this->timerBlocked->Tail = (Node*)activeThread;
	}
	this->timerBlocked->Length++;
	this->activeThread = nextToBeActive;
	this->SwitchToNext();
	schedLock->Release();
}

void Scheduler::RemoveActiveThread()
{
	Node* active = (Node*)this->activeThread;
	this->activeList->RemoveNode(active);
}

void Scheduler::IOBlock()
{
	ioLock->GetLock();
	schedLock->GetLock();
	Node* current = (Node*)this->activeThread;
	//if it isn't the only one
	if(this->activeThread->prev != this->activeThread)
	{
		if(this->activeList->Root == this->activeThread)
		{
			this->activeList->Root = this->activeThread->next;
		}
		if(this->activeList->Tail == this->activeThread)
		{
			this->activeList->Tail = this->activeThread->prev;
		}
		this->activeThread->prev->next = this->activeThread->next;

	}
	//if it is the only one
	else
	{
		// null everything out in this case
		this->activeList->Root = this->activeList->Tail = NULL;
		this->activeThread = NULL;
	}

	// now that the active thread is free we can move it to blocked
	if(this->ioBlocked->Tail)
	{
		current->prev = this->ioBlocked->Tail;
		current->next = this->ioBlocked->Root;
		this->ioBlocked->Tail->next = current;
		this->ioBlocked->Tail = current;
	}
	else
	{
		current->prev = current;
		current->next = current;
		this->ioBlocked->Tail = current;
		this->ioBlocked->Root = current;
	}
	ioLock->Release();
	// we need to have the schedLock set to do SwitchToNext
	SwitchToNext();
	schedLock->Release();
}

void Scheduler::IOResume(Thread* thread)
{
	// we can never get back, yet
	Yield(); // for now
}

Thread* Scheduler::GetActiveThread()
{
	if(this->activeThread == NULL)
	{
		return NULL;
	}
	return (Thread*)this->activeThread->data;
}

Process* Scheduler::GetActiveProcess()
{
	Thread* thread = GetActiveThread();
	return thread==NULL?NULL:thread->GetProcess();
}

void Scheduler::SaveRegisters(struct regs* r)
{
	Node* active = (Node*)this->activeThread;
	if(active)
	{
		Thread* current = (Thread*)active->data;
		current->CalledStack.Push(*r);
	}
	else
	{
		this->activeThread = this->activeList->Root->prev;
		this->SwitchToNext();
	}
}


