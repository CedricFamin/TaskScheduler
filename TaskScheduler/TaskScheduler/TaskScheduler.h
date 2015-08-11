#ifndef TASK_SCHEDULER_H_
#define TASK_SCHEDULER_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>

#include "Task.h"
#include "FreeListAllocator.h"
#include "WorkerThread.h"

class TaskScheduler
{
	typedef std::pair<std::chrono::system_clock::time_point, Task*> TaskWTimeLaunch;
public:

	TaskScheduler(unsigned int nbThread);
	TaskScheduler(TaskScheduler const &) = delete;
	void operator=(TaskScheduler const &) = delete;
	~TaskScheduler();

	void Start();
	void Stop(); 
	TaskId CreateTask(Task::CallbackType * parCallback, Task::TaskData const & parData, TaskId const & parParentId = TaskId());
	void AddTask(TaskId parTask);
	void ScheduleAt(std::chrono::system_clock::time_point const & parTime, TaskId parTask);
	void ScheduleEvery(std::chrono::milliseconds const & parTime, TaskId parTask, bool parNeedAddTask = true);
	void ReleaseTask(Task * parTask);
	Task * GetTask(TaskId const & parTask);
	Task * PickupTaskIFP();
	bool IsTaskFinished(TaskId const & parTaskId);
	void WaitForTaskFinished(TaskId const & parTaskId);

	std::unique_lock<std::mutex> WaitForNewTask(std::chrono::system_clock::time_point const & parWaitUntil = std::chrono::time_point<std::chrono::system_clock>::max());

	void MainLoop();

private:

	std::atomic<bool>			_running = false;
	bool					    _useThisThreadDuringWait = false;
	std::mutex                  _taskListMutex;
	std::vector<WorkerThread>   _threads;
	std::list<Task*>			_taskToProcess;
	std::condition_variable     _newTask;
	std::condition_variable     _oneTaskRelease;
	FreeListAllocator<Task>     _taskAllocator;
};

#endif