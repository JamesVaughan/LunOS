/*
 * Tasks.hpp
 *
 *  Created on: 2013-08-21
 *      Author: james
 */

#ifndef TASKS_HPP_
#define TASKS_HPP_
#include <LunOS.h>
#include <synchronization.h>
#include <Stdlib.h>

namespace LunOS
{
	namespace Tasks
	{
	class Task;
	class Threadpool;
	class TaskThread;

	//This task should be deleted by the invoker
	class Task
	{
		friend class TaskThread;
	public:
		// Create a new task for execution
		Task(void (*ToExecute)(void* data), void* data, bool cleanup = false);
		// Start executing this task on the current thread
		void StartOnThread();
		bool HasCompleted();
		~Task();
		static void Wait(Task* task);
		static void WaitAll(Array<Task*> tasks);
	private:
		volatile bool Completed;
		Lock ExecutionLock;
		// The method to execute
		void (*ToExecute)(void* data);
		void* Data;
		bool Cleanup;
	};

	class TaskThread
	{
	public:
		// Initialize another TaskThread
		TaskThread();
		// Add a task to this
		void AddTask(Task* task);
		// Remove a task from this Task Thread
		Task* TakeTask();
		// The number of tasks that this thread has been queued with
		unsigned int CurrentlyExecuting();
		// Finish executing tasks and then kill the thread
		void ShutdownTaskThread();
	private:
		static unsigned int ExecutionPoint(void* params);
		// The thread that is executing the tasks
		unsigned int ExecutingThread;
		// Should this thread continue to run?
		volatile bool ContinueRunning;
		// Locks access to the task queue
		Lock TaskQueueLock;
		//The queue of tasks that we have
		LunOS::DataStructures::Queue<Task*> TaskQueue;
	};

	// The primary threadpool for the kernel
	class Threadpool
	{

	public:
		//Initialize a new Threadpool, this will not set it to be the default
		Threadpool();
		// Destroy the threadpool
		~Threadpool();
		//Get or initialize a new threadpool
		static Threadpool* GetDefault();
		//Queue a new task for execution
		void StartTask(Task* task);
	private:
		void ManagePoolPass();
		// The default threadpool
		static Threadpool* Default;
		// Should this thread continue to run?
		volatile bool ContinueRunning;
		//The threads that this pool is using
		TaskThread** TaskThreads;
		//The number of task threads
		unsigned int NumberOfTaskThreads;
		// The lock to hold in order to move tasks around
		Lock TaskQueueLock;
		// The queue that contains uninitialized Tasks
		LunOS::DataStructures::Queue<Task*> TaskQueue;
		//The thread that manages this threadpools balance across TaskThreads
		unsigned int ManagementThread;
		// The entry point for the threadpool's thread
		static unsigned int ManagementEntryPoint(void* params);
	};
	}
}

#endif /* TASKS_HPP_ */
