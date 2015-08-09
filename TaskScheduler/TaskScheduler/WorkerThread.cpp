#include <iostream>

#include "WorkerThread.h"
#include "TaskScheduler.h"

class TaskScheduler;

namespace {
	void RunThread(WorkerThread * parWorker)
	{
		parWorker->Body();
	}
}

WorkerThread::WorkerThread()
	: _thread()
	, _running(false)
{
}

void WorkerThread::Run(TaskScheduler * parScheduler)
{
	assert(parScheduler != nullptr);
	this->_scheduler = parScheduler;
	this->_thread = std::thread(std::bind(RunThread, this));
}

void WorkerThread::Stop()
{
	this->_running = false;
}

void WorkerThread::Body()
{
	this->_running = true;
	while (this->_running)
	{
		Task * nextTask = WaitForTask();
		if (nextTask)
		{
			ExecuteTask(nextTask);
			this->_scheduler->ReleaseTask(nextTask);
		}
	}
	this->_scheduler = nullptr;
}

void WorkerThread::Join()
{
	assert(this->_running == false);
	this->_thread.join();
}

Task* WorkerThread::WaitForTask()
{
	std::unique_lock<std::mutex> ul = this->_scheduler->WaitForNewTask();
	Task* taskToExecute = this->_scheduler->PickupTaskIFP();
	return taskToExecute;
}

void WorkerThread::ExecuteTask(Task * parTask)
{
	parTask->Run();
}