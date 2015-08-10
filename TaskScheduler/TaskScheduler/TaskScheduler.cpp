#include "TaskScheduler.h"

TaskScheduler::TaskScheduler(unsigned int nbThread)
	: _taskAllocator(1024)
	, _threads(nbThread)
	, _useThisThreadDuringWait(false)
{

}

TaskScheduler::~TaskScheduler()
{
	assert(this->_running == false);
	for (auto & thread : this->_threads)
	{
		thread.Join();
	}
	for (auto & task : this->_taskToProcess)
	{
		this->_taskAllocator.ReleaseObject(task);
	}
}

void TaskScheduler::Start()
{
	this->_running = true;
	for (auto & thread : this->_threads)
	{
		thread.Run(this);
	}
}

void TaskScheduler::Stop()
{
	this->_running = false;
	for (auto & thread : this->_threads)
	{
		thread.Stop();
	}
}

TaskId TaskScheduler::CreateTask(Task::CallbackType * parCallback, Task::TaskData const & parData, TaskId const & parParentId)
{
	std::pair<TaskId, Task*> idWTask = this->_taskAllocator.GetObject();
	idWTask.second->SetCallback(parCallback);
	idWTask.second->SetData(parData);
	idWTask.second->SetParent(parParentId);
	if (parParentId != TaskId::InvalidId())
	{
		std::lock_guard<std::mutex> lg(this->_taskListMutex);
		Task * parentTask = this->_taskAllocator.GetObject(parParentId);
		parentTask->AddOneChild();
	}
	return idWTask.first;
}

void TaskScheduler::AddTask(TaskId parTask)
{
	this->ScheduleAt(std::chrono::system_clock::now(), parTask);
}

void TaskScheduler::ScheduleAt(std::chrono::system_clock::time_point const & parTime, TaskId parTask)
{
	assert(this->_running);
	std::lock_guard<std::mutex> lg(this->_taskListMutex);
	assert(this->_taskAllocator.IsAlive(parTask));
	Task * taskPtr = this->_taskAllocator.GetObject(parTask);
	taskPtr->SetTimeToLaunch(parTime);
	this->_taskToProcess.push_back(taskPtr); // todo sort the vector !!!
	this->_newTask.notify_all();
}

void TaskScheduler::ScheduleEvery(std::chrono::milliseconds const & parTime, TaskId parTask, bool parNeedAddTask)
{
	// we create a new task, it will execute parTask
	// parTask is a Parent of the new task so parTask will never be executed itself
	// but we can add partask to the scheduler in order to release memory during stop
	Task::TaskData newTaskData;
	newTaskData.RepetetiveTaskData.RepetiveTask = parTask;
	newTaskData.RepetetiveTaskData.RepeatTimer = parTime;
	newTaskData.RepetetiveTaskData.Scheduler = this;
	TaskId newTask = this->CreateTask(RepetitiveTask, newTaskData, parTask);
	this->ScheduleAt(std::chrono::system_clock::now() + parTime, newTask);
	Task* execTask = GetTask(parTask);
	if (parNeedAddTask)
		this->AddTask(parTask);
}


void TaskScheduler::ReleaseTask(Task * parTask)
{
	std::lock_guard<std::mutex> lg(this->_taskListMutex);
	if (parTask->Parent() != TaskId::InvalidId())
	{
		assert(this->_taskAllocator.IsAlive(parTask->Parent()));
		Task * taskParent = this->_taskAllocator.GetObject(parTask->Parent());
		taskParent->RemoveOneChild();
	}

	this->_taskAllocator.ReleaseObject(parTask);
	this->_oneTaskRelease.notify_all();
	this->_newTask.notify_all();
}

Task * TaskScheduler::GetTask(TaskId const & parTask)
{
	std::lock_guard<std::mutex> ul(this->_taskListMutex);
	return this->_taskAllocator.GetObject(parTask);
}

Task * TaskScheduler::PickupTaskIFP()
{
	if (this->_taskToProcess.empty())
		return nullptr;

	for (auto task : this->_taskToProcess)
	{
		if (task->CanExecuteThisTask())
		{
			this->_taskToProcess.remove(task);
			return task;
		}
	}
	return nullptr;
}

bool TaskScheduler::IsTaskFinished(TaskId const & parTaskId)
{
	std::lock_guard<std::mutex> ul(this->_taskListMutex);
	return this->_taskAllocator.IsAlive(parTaskId);
}

void TaskScheduler::WaitForTaskFinished(TaskId const & parTaskId)
{
	while (IsTaskFinished(parTaskId))
	{
		// don't waste my time, maybe I can work with you my friends
		Task * task = nullptr;
		if (_useThisThreadDuringWait)
		{
			std::unique_lock<std::mutex> ul(this->_taskListMutex);
			task = PickupTaskIFP();
		}
			
		if (task)
		{
			task->Run();
			ReleaseTask(task);
			task = nullptr;
		}
		else
		{
			std::unique_lock<std::mutex> ul(this->_taskListMutex);
			this->_oneTaskRelease.wait(ul);
		}
	}
}

std::unique_lock<std::mutex> TaskScheduler::WaitForNewTask(std::chrono::system_clock::time_point const & parWaitUntil)
{
	std::unique_lock<std::mutex> ul(this->_taskListMutex);
	if (parWaitUntil < std::chrono::time_point<std::chrono::system_clock>::max())
		this->_newTask.wait_until(ul, parWaitUntil);
	else
		this->_newTask.wait(ul);
	return ul;
}

void TaskScheduler::MainLoop()
{
	this->_running = true;
	while (this->_running)
	{
		std::chrono::system_clock::time_point nextTimePoint = std::chrono::time_point<std::chrono::system_clock>::max();
		bool taskShouldBePicked = false;
		{
			std::unique_lock<std::mutex> ul(this->_taskListMutex);
			for (auto task : this->_taskToProcess)
			{
				if (task->CanExecuteThisTaskWithoutTime())
					nextTimePoint = nextTimePoint > task->TimeToLaunch() ? task->TimeToLaunch() : nextTimePoint;
				if (task->CanExecuteThisTask())
				{
					taskShouldBePicked = true;
					break;
				}
			}
		}

		if (taskShouldBePicked)
		{
			this->_newTask.notify_all();
		}
		this->WaitForNewTask(nextTimePoint);
	}
}
