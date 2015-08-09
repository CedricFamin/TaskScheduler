#ifndef TASK_H_
#define TASK_H_

#include <chrono>

#include "FreeListAllocator.h"

// Task class
// All task should have the same memory footprint in order to use the FreeListAllocator
// Avoid inheritence 

typedef Id TaskId;
class TaskScheduler;

class Task
{
public:
	struct TaskData
	{
		TaskData() {}
		union {
			struct
			{
				TaskScheduler * Scheduler;
			} StopData;
			struct
			{
				TaskScheduler * Scheduler;
				TaskId RepetiveTask;
				std::chrono::milliseconds RepeatTimer;
			} RepetetiveTaskData;
		};
	};
	typedef void(CallbackType)(TaskData &);
public:
	Task();
	Task(CallbackType * parCallback, TaskData const & parData);
	~Task();

	void SetCallback(CallbackType * parCallback);
	void SetData(TaskData const & parData);
	void SetParent(TaskId const & parParentId);
	void SetTimeToLaunch(std::chrono::system_clock::time_point const & parTime);
	void Run();

	bool CanExecuteThisTask() const;
	bool CanExecuteThisTaskWithoutTime() const;
	TaskId Parent() const;
	void AddOneChild();
	void RemoveOneChild();
	std::chrono::system_clock::time_point TimeToLaunch() const;
private:

	std::chrono::system_clock::time_point	_timeLaunch;
	CallbackType *							_callback;
	TaskData								_data;
	TaskId									_parent;
	unsigned int							_TaskTreeSize;
};

void RepetitiveTask(Task::TaskData & parTask);

#endif