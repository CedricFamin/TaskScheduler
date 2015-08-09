// TaskScheduler.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <ctime>

#include "stdafx.h"
#include "TaskScheduler.h"

void StopSchedulerTask(Task::TaskData & parData)
{
	parData.StopData.Scheduler->Stop();
	std::cout << "Stop Scheduler" << std::endl;
}
void byeWorld(Task::TaskData & parData)
{
	std::cout << "Bye bye World !" << std::endl;
}


void helloWorld(Task::TaskData & parData)
{
	std::cout << "Hello World !" << std::endl;
}

void massAddition(Task::TaskData & parData)
{
	int nb = 1;
	while (nb > 0)
	{
		nb += 1;
	}
	std::cout << "Mass Addition Finished" << std::endl;
}

void ParentChildTask()
{
	TaskScheduler taskScheduler(8);
	taskScheduler.Start();

	Task::TaskData emptyTaskData;
	Task::TaskData stopSchedulerTask;
	stopSchedulerTask.StopData.Scheduler = &taskScheduler;

	TaskId stopTask = taskScheduler.CreateTask(StopSchedulerTask, stopSchedulerTask);
	
	TaskId idTask1 = taskScheduler.CreateTask(byeWorld, emptyTaskData, stopTask);
	TaskId idTask2 = taskScheduler.CreateTask(byeWorld, emptyTaskData, idTask1);
	TaskId idTask3 = taskScheduler.CreateTask(byeWorld, emptyTaskData, idTask1);
	TaskId idTask4 = taskScheduler.CreateTask(byeWorld, emptyTaskData, idTask1);
	TaskId idTask5 = taskScheduler.CreateTask(byeWorld, emptyTaskData, idTask1);
	TaskId idTask6 = taskScheduler.CreateTask(byeWorld, emptyTaskData, idTask5);

	/* Task executed ASAP */
	taskScheduler.AddTask(stopTask);
	taskScheduler.AddTask(idTask1);
	taskScheduler.AddTask(idTask2);
	taskScheduler.AddTask(idTask3);
	taskScheduler.AddTask(idTask4);
	taskScheduler.AddTask(idTask5);
	taskScheduler.AddTask(idTask6);
	taskScheduler.Stop();
}

void ScheduleTask()
{
	TaskScheduler taskScheduler(1);

	taskScheduler.Start();
	Task::TaskData stopSchedulerTask;
	stopSchedulerTask.StopData.Scheduler = &taskScheduler;
	TaskId stopTask = taskScheduler.CreateTask(StopSchedulerTask, stopSchedulerTask);

	Task::TaskData emptyTaskData;
	TaskId helloWorldTask = taskScheduler.CreateTask(helloWorld, emptyTaskData);
	TaskId byeWorldTask = taskScheduler.CreateTask(byeWorld, emptyTaskData);

	taskScheduler.ScheduleAt(std::chrono::system_clock::now() + std::chrono::seconds(10), helloWorldTask);
	taskScheduler.ScheduleAt(std::chrono::system_clock::now() + std::chrono::seconds(15), byeWorldTask);
	taskScheduler.ScheduleAt(std::chrono::system_clock::now() + std::chrono::seconds(20), stopTask);

	taskScheduler.MainLoop();
}

void RepeatTask()
{
	TaskScheduler taskScheduler(1);

	taskScheduler.Start();
	Task::TaskData stopSchedulerTask;
	stopSchedulerTask.StopData.Scheduler = &taskScheduler;
	TaskId stopTask = taskScheduler.CreateTask(StopSchedulerTask, stopSchedulerTask);

	Task::TaskData emptyTaskData;
	TaskId helloWorldTask = taskScheduler.CreateTask(helloWorld, emptyTaskData);
	TaskId byeWorldTask = taskScheduler.CreateTask(byeWorld, emptyTaskData);

	taskScheduler.ScheduleEvery(std::chrono::seconds(1), helloWorldTask);
	taskScheduler.ScheduleEvery(std::chrono::seconds(5), byeWorldTask);
	taskScheduler.ScheduleAt(std::chrono::system_clock::now() + std::chrono::seconds(20), stopTask);

	taskScheduler.MainLoop();
}

void ExecuteOneTask()
{
	TaskScheduler taskScheduler(1);
	
	taskScheduler.Start();
	Task::TaskData emptyTaskData;
	TaskId helloWorldTask = taskScheduler.CreateTask(massAddition, emptyTaskData);
	taskScheduler.AddTask(helloWorldTask);
	taskScheduler.Stop();
}

int main()
{
	std::cout << "====== Begin Test: ExecuteOneTask =====" << std::endl;
	ExecuteOneTask();
	std::cout << "====== End Test: ExecuteOneTask =====" << std::endl;

	std::cout << "====== Begin Test: ParentChildTask =====" << std::endl;
	ParentChildTask();
	std::cout << "====== End Test: ParentChildTask =====" << std::endl;

	std::cout << "====== Begin Test: ScheduleTask =====" << std::endl;
	ScheduleTask();
	std::cout << "====== End Test: ScheduleTask =====" << std::endl;

	std::cout << "====== Begin Test: RepeatTask =====" << std::endl;
	RepeatTask();
	std::cout << "====== End Test: RepeatTask =====" << std::endl;

	system("PAUSE");
    return 0;
}

