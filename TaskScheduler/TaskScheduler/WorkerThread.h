#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include <atomic>
#include <thread>
#include <condition_variable>

#include "Task.h"

class TaskScheduler;

class WorkerThread
{
public:
	WorkerThread();
	WorkerThread(WorkerThread const &) = delete;
	void operator=(WorkerThread const &) = delete;

	void Run(TaskScheduler * parScheduler);
	void Stop();
	void Body();
	void Join();
protected:
private:
	Task* WaitForTask();
	void ExecuteTask(Task * parTask);

	std::atomic<bool>		 _running;
	std::thread             _thread;
	TaskScheduler *         _scheduler;
};

#endif