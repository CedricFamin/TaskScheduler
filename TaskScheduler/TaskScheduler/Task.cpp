#include <cassert>

#include "Task.h"
#include "TaskScheduler.h"

Task::Task()
	: _callback(nullptr)
	, _TaskTreeSize(1)
{
}

Task::Task(CallbackType * parCallback, TaskData const & parData)
	: _callback(parCallback)
	, _data(parData)
	, _TaskTreeSize(1)
{
	assert(this->_callback != nullptr);
}


Task::~Task()
{
}

void Task::SetCallback(CallbackType * parCallback)
{
	assert(parCallback != nullptr);
	this->_callback = parCallback;
}

void Task::SetData(TaskData const & parData)
{
	this->_data = parData;
}

void Task::SetParent(TaskId const & parParentId)
{
	this->_parent = parParentId;
}

void Task::SetTimeToLaunch(std::chrono::system_clock::time_point const & parTime)
{
	this->_timeLaunch = parTime;
}

void Task::Run()
{
	this->_callback(this->_data);
}

bool Task::CanExecuteThisTask() const
{
	return CanExecuteThisTaskWithoutTime() &&
		this->_timeLaunch <= std::chrono::system_clock::now();
}

bool Task::CanExecuteThisTaskWithoutTime() const
{
	return this->_TaskTreeSize == 1;
}

TaskId Task::Parent() const
{
	return this->_parent;
}

void Task::AddOneChild()
{
	++this->_TaskTreeSize;
}

void Task::RemoveOneChild()
{
	--this->_TaskTreeSize;
}

std::chrono::system_clock::time_point Task::TimeToLaunch() const
{
	return this->_timeLaunch;
}

void RepetitiveTask(Task::TaskData & parTask)
{
	TaskScheduler * scheduler = parTask.RepetetiveTaskData.Scheduler;
	TaskId taskToExec = parTask.RepetetiveTaskData.RepetiveTask;
	std::chrono::milliseconds repeatTimer = parTask.RepetetiveTaskData.RepeatTimer;

	Task * task = scheduler->GetTask(taskToExec);
	assert(task != nullptr);
	task->Run();
	scheduler->ScheduleEvery(repeatTimer, taskToExec, false);
}