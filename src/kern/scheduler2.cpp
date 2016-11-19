#include <kern/scheduler.hpp>
#include <kern/console.h>
#include <SysCall.h>

#ifndef NULL
#define NULL 0
#endif

Scheduler* Sched;
Process* InitialProcess;
Lock* schedLock;
Lock* ioLock;
Lock* zombieLock;

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
	unsigned int activeListLength;
	if(this && this->activeList && (activeListLength = this->activeList->Length) > 1)
	{
		LunOS::System::Yield();
	}
	else
	{
		__asm__ __volatile__ ("hlt");
	}
}

void Scheduler::AddThread(Thread* thread)
{
	Node* newLast = new Node(thread);
	schedLock->GetLock();
	this->activeList->AddLast(newLast);
	if(this->activeList->Length <= 1)
	{
		this->activeThread = this->activeList->Root;
	}
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
	}while(current != NULL && current != this->activeList->Root);
	printf("%2IO Blocked Threads\n");
	current = this->ioBlocked->Root;
	do
	{
		if(current == NULL) break;
		Thread* currentThread = (Thread*)current->data;
		printf("%s:%i\n",currentThread->name, currentThread->tid);
		current = current->next;
	}while(current != NULL && current != this->ioBlocked->Root);

	printf("%2Sleeping Threads: Time Remaining(ms)\n");
	current = this->timerBlocked->Root;
	do
	{
		if(current == NULL) break;
		Thread* sleepingData = (Thread*)current->data;
		printf("%s:%i:%ims\n",sleepingData->name, sleepingData->tid, (sleepingData->WakeupAt - (unsigned int)timer_ticks));
		current = current->next;
	}while(current != NULL && current != this->timerBlocked->Root);
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
		return;
	}
	Process* activeProcess = this->GetActiveProcess();
	activeProcess->threads->RemoveItem(thread);
	Memory::KernalMemoryLock->GetLock();
	schedLock->GetLock();
	Node* activeNode = (Node*)this->activeThread;
	Node* threadNode;
	if(activeNode->data == thread)
	{
		threadNode = activeNode;
		this->SwitchToNext();
	}
	else
	{
		threadNode = this->activeList->Search(thread);
	}
	if(threadNode != NULL)
	{
		this->activeList->RemoveNode(threadNode);
		kfree((unsigned char*)thread->Stack);
		kfree((unsigned char*)threadNode->data);
		kfree((unsigned char*)threadNode);
	}
	else
	{
		printf("%3Tried to kill a non active thread!!!\n");
	}
	schedLock->Release();
	Memory::KernalMemoryLock->Release();
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
	if(activeThread == NULL)
	{
		this->activeThread = this->activeList->Root;
		if(this->activeThread == NULL)
		{
			printf("%3There are no active threads SYSTEM DOWN!\n");
		}
	}
}

void Scheduler::Yield()
{
	// Check to see if the scheduler has been initialised
	if(!schedLock)
	{
		SwitchToNext();
	}
	else
	{
		// If the lock exists try to get it
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
	Node* current = this->timerBlocked->Root;
	while(current != NULL)
	{
		Thread* headThread = (Thread*)current->data;
		if(headThread->WakeupAt >= timer_ticks)
		{
			this->timerBlocked->RemoveNode(current);
			this->activeList->AddLast(current);
		}
		else
		{
			break;
		}
		current = current->next;
	}
}

void Scheduler::Sleep(unsigned int ms)
{
	// for now, just skip along
	schedLock->GetLock();
	// Make sure this can be done all in 1 shot since a user program calls this
	Node* sleepNode = (Node*)this->activeThread;
	Thread* sleepThread = (Thread*)this->activeThread->data;
	RemoveActiveThread();
	sleepThread->WakeupAt = ((ms * 18) / 1000) + timer_ticks;
	// timerblocked needs to be sorted
	Node* current = this->timerBlocked->Root;
	bool done = false;
	while(current != NULL)
	{
		if(((Thread*)current->data)->WakeupAt >= sleepThread->WakeupAt)
		{
			Node* prev = current->prev;
			sleepNode->next = current;
			current->prev = sleepNode;
			if(prev == NULL)
			{
				timerBlocked->Root = sleepNode;
			}
			else
			{
				prev->next = sleepNode;
			}
			done = true;
			break;
		}
		current = current->next;
	}
	if(!done)
	{
		Node* root = timerBlocked->Root;
		if(root == NULL)
		{
			sleepNode->next = NULL;
			sleepNode->prev = NULL;
			timerBlocked->Root = timerBlocked->Tail = sleepNode;
		}
		else
		{
			Node* tail = timerBlocked->Tail;
			sleepNode->prev = tail;
			sleepNode->next = NULL;
			tail->next = sleepNode;
			timerBlocked->Tail = sleepNode;
		}
	}
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
	Node* sleepNode = (Node*)this->activeThread;
	Thread* sleepThread = (Thread*)this->activeThread->data;
	RemoveActiveThread();
	sleepThread->WakeupAt = 0;
	this->ioBlocked->AddLast(sleepNode);
	this->SwitchToNext();
	schedLock->Release();
	ioLock->Release();
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


