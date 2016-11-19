#include <kern/process.hpp>
#include <SysCall.h>
#include <kern/core.h>
CPU* cpus;
LinkedList* CPU::cpuList;
LinkedList* processes;
volatile int tidPool = 0;

Process::Process(User* owner)
{
	this->owner = owner;
	this->threads = new LinkedList();
	this->masterPageTable = Memory::CreateAddressSpace();
	this->fdt.Init();
	processes->AddLast(this);
}

Process::~Process()
{
	Node* cur = this->threads->Root;
	while(cur)
	{
		Sched->KillThread((Thread*)cur->data);
		cur = cur->next;
	}
	this->fdt.Release();
	Memory::ReleaseAddressSpace(this->masterPageTable);
}

unsigned int detectProcessors()
{
	return CPU::cpuList->Length;
}

void Process::initProcesses()
{
	processes = new LinkedList();
}

void* Process::GetLocalMemoryAddress(short int upper, short int lower)
{
	unsigned int* cur = (unsigned int*)*(this->masterPageTable + upper);
	if(((unsigned int)cur & 0x1) == 0) return NULL;
	cur = (unsigned int*)(((unsigned int)cur)&0xFFFFF000);
	if(cur == NULL) return NULL;
	return (void*)(((unsigned int)*cur)&0xFFFFF000);
}

void ThreadDeath(Thread* us)
{
	LunOS::System::KillThread();
}

Thread::Thread(unsigned char* name, Process* proc, unsigned int Start, unsigned char* stack, unsigned int stackSize, void* param)
: CalledStack((unsigned int)16)
{
	int i;
	if(name != NULL)
	{
		for(i = 0; i < 63; i++)
		{
			if(name[i] == 0)
				break;
			this->name[i] = name[i];
		}
		// make sure this string is terminated properly
		this->name[i] = 0;
	}
	else
	{
		strcpy((char*)this->name, "Unnamed Thread");
	}
	this->Stack = stack - stackSize;
	this->proc = proc;
	// we need to buffer up a bit of space in our stack
	this->inKernelCall = false;
	struct regs registers;
	registers.cs = 0x08;
	registers.ds =  registers.es  = registers.fs  =  registers.gs = 0x10;
	registers.eax = registers.ebx = registers.ecx =  registers.edx = 0x00;
	registers.edi = registers.esi = 0x00;
	registers.err_code = registers.int_no = 0;
	// we need to fill the next values
	registers.esp = ((unsigned int) stack) - (sizeof(void*) * 2);
	registers.eip = Start;
	// BA98 7654 3210
	// 0010 0000 0010
	registers.eflags = 0x202;
	*((unsigned int*)stack - 2) = (unsigned int)(ThreadDeath);
	*((unsigned int*)stack - 1) = (unsigned int)param;
	registers.esp -= 20;
	registers.ebp = registers.esp;
	this->CalledStack.Push(registers);
	struct regs* r = (struct regs*)(registers.esp - sizeof(struct regs));
	r->cs = registers.cs;
	r->eip = registers.eip;
	r->eflags = registers.eflags;
	this->tid = tidPool++;
	this->proc->threads->AddLast(this);
}

Thread::~Thread()
{
	Node* n = this->proc->threads->Search(this);
	if(n->prev){
		n->prev->next = n->next;
		n->next->prev = n->prev;
	}
	if(this->proc->threads->Root == n)
	{
		this->proc->threads->Root = n->next;
	}
	delete this->Stack;
}

Process* Thread::GetProcess()
{
	return this->proc;
}

bool Process::SwitchToUser(unsigned int uid, const char* password)
{
	User* ourGuy;
	if((ourGuy = User::GetUserWithID(uid)) != NULL && ourGuy->checkPassword(password))
	{
		this->owner = ourGuy;
		return true;
	}
	return false;
}

void CPU::InitCPUs()
{
	CPU::cpuList = new LinkedList();
}

