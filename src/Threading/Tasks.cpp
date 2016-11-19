/*
 * Tasks.cpp
 *
 *  Created on: 2013-08-22
 *      Author: james
 */
#include <kern/system.hpp>
#include <Threading/Tasks.hpp>

namespace LunOS
{
namespace Tasks
{

Threadpool* Threadpool::Default = NULL;

Task::Task(void (*ToExecute)(void* data), void* data, bool cleanup)
{
	this->ToExecute = ToExecute;
	this->Data = data;
	this->Cleanup = cleanup;
	this->Completed = false;
}

Task::~Task()
{
	//nothing to do here
	if (this->Cleanup)
	{
		auto temp = (unsigned int*)this->Data;
		SafeDelete(temp);
	}
}

void Task::StartOnThread()
{
	this->ExecutionLock.GetLock();
	if (!this->Completed)
	{
		this->ToExecute(this->Data);
		this->Completed = true;
		if (this->Cleanup)
		{
			delete (unsigned int*) this->Data;
			this->Data = NULL;
		}
	}
	this->ExecutionLock.Release();
}

bool Task::HasCompleted()
{
	return this->Completed;
}

void Task::Wait(Task* task)
{
	while (!task->HasCompleted())
	{
		LunOS::System::Yield();
	}
}

void Task::WaitAll(Array<Task*> taskArray)
{
	for (unsigned int i = 0; i < taskArray.Length(); i++)
	{
		Task::Wait(taskArray[i]);
	}
}

TaskThread::TaskThread()
{
	this->ContinueRunning = true;
	// When we are done setting up, spin-up the thread
	this->ExecutingThread = LunOS::System::CreateThread(
			(unsigned char*) "TaskThread", TaskThread::ExecutionPoint,
			(void*) this);
}

unsigned int TaskThread::CurrentlyExecuting()
{
	return this->TaskQueue.GetSize();
}

unsigned int TaskThread::ExecutionPoint(void* params)
{
	TaskThread* us = (TaskThread*) params;
	while (us->ContinueRunning)
	{
		if (us->TaskQueue.GetSize() > 0 && us->TaskQueueLock.TryLock())
		{
			Task* task = us->TaskQueue.Dequeue();
			us->TaskQueueLock.Release();
			if (!task->HasCompleted())
			{
				task->StartOnThread();
			}
		}
		else
		{
			LunOS::System::Yield();
		}
	}
	us->TaskQueueLock.GetLock();
	while (us->TaskQueue.GetSize() > 0)
	{
		Task* task = us->TaskQueue.Dequeue();
		if (!task->HasCompleted())
		{
			task->StartOnThread();
		}
	}
	us->TaskQueueLock.Release();
	return 0;
}

void TaskThread::ShutdownTaskThread()
{
	this->ContinueRunning = false;
}

void TaskThread::AddTask(Task* task)
{
	this->TaskQueueLock.GetLock();
	this->TaskQueue.Add(task);
	this->TaskQueueLock.Release();
}

Task* TaskThread::TakeTask()
{
	this->TaskQueueLock.GetLock();
	Task* ret = this->TaskQueue.RemoveLast();
	this->TaskQueueLock.Release();
	return ret;
}

Threadpool::Threadpool()
{
	this->ContinueRunning = true;
	this->NumberOfTaskThreads = 2;
	this->TaskThreads = new TaskThread*[this->NumberOfTaskThreads];
	for (unsigned int i = 0; i < this->NumberOfTaskThreads; i++)
	{
		this->TaskThreads[i] = new TaskThread();
	}
	this->ManagementThread = LunOS::System::CreateThread(
			(unsigned char*) "Task Management Thead",
			Threadpool::ManagementEntryPoint, (void*) this);
}

Threadpool::~Threadpool()
{
	this->ContinueRunning = false;
}

Threadpool* Threadpool::GetDefault()
{
	if (Threadpool::Default == NULL)
	{
		Threadpool::Default = new Threadpool();
	}
	return Threadpool::Default;
}

void Threadpool::StartTask(Task* task)
{
	if (task)
	{
		this->TaskQueueLock.GetLock();
		this->TaskQueue.Add(task);
		this->TaskQueueLock.Release();
	}
}

unsigned int Threadpool::ManagementEntryPoint(void* params)
{
	Threadpool* us = (Threadpool*) params;
	while (us->ContinueRunning)
	{
		if (us->TaskQueueLock.TryLock())
		{
			//Manage the pool
			us->ManagePoolPass();
			//Done
			us->TaskQueueLock.Release();
		}
		LunOS::System::Yield();
	}
	return 0;
}

void Threadpool::ManagePoolPass()
{
	Task* task = this->TaskQueue.Dequeue();
	if (!task)
	{
		return;
	}
	unsigned int minIndex = 0;
	unsigned int minTasks = this->TaskThreads[0]->CurrentlyExecuting();
	for (unsigned int i = 1; i < this->NumberOfTaskThreads; i++)
	{
		unsigned int ammount = this->TaskThreads[i]->CurrentlyExecuting();
		if (ammount < minTasks)
		{
			minTasks = ammount;
			minIndex = i;
		}
	}
	this->TaskThreads[minIndex]->AddTask(task);
}

} // end namespaces
}
