// TaskScheduler.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <chrono>

#include "stdafx.h"
#include "TaskScheduler.h"
#include "Promise.h"
#include "TripeBuffering.h"


void ReadFutureTask(Task::TaskData & parData)
{
	std::cout << "Future : " << parData.FutureData.Future->GetValue() << std::endl;
}

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
	TaskScheduler taskScheduler(8);

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
	TaskScheduler taskScheduler(8);

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

	Task::TaskData stopSchedulerTask;
	stopSchedulerTask.StopData.Scheduler = &taskScheduler;
	TaskId stopTask = taskScheduler.CreateTask(StopSchedulerTask, stopSchedulerTask);
	
	taskScheduler.AddTask(stopTask);

	taskScheduler.MainLoop();
}

void ExecuteMassTask()
{
	TaskScheduler taskScheduler(8);
	taskScheduler.Start();

	Task::TaskData stopSchedulerTask;
	stopSchedulerTask.StopData.Scheduler = &taskScheduler;
	TaskId stopTask = taskScheduler.CreateTask(StopSchedulerTask, stopSchedulerTask);

	Task::TaskData emptyTaskData;
	for (int i = 0; i < 512; ++i)
	{
		TaskId task = taskScheduler.CreateTask(massAddition, emptyTaskData, stopTask);
		taskScheduler.AddTask(task);
	}
	taskScheduler.AddTask(stopTask);

	taskScheduler.MainLoop();
}

void DelayedInit()
{
	promise_impl::DelayedInit<int> delayedInt;
	promise_impl::DelayedInit<std::string> delayedString;
	promise_impl::DelayedInit<TaskScheduler> delayedComplexObject;

	delayedInt.Init(5);
	
	std::cout << "- DelayedInit : delayedInt.Init(5) = " << *delayedInt << std::endl;

	delayedString.Init("String");
	std::cout << "- DelayedInit : delayedString.Init(\"String\") = " << *delayedString << std::endl;

	std::cout << "- DelayedInit : delayedComplexObject.Init(8) = " << *delayedString << std::endl;
	delayedComplexObject.Init(8);

	delayedComplexObject->Start();
	Task::TaskData stopSchedulerTask;
	stopSchedulerTask.StopData.Scheduler = &(*delayedComplexObject);
	TaskId stopTask = delayedComplexObject->CreateTask(StopSchedulerTask, stopSchedulerTask);

	Task::TaskData emptyTaskData;
	TaskId helloWorldTask = delayedComplexObject->CreateTask(helloWorld, emptyTaskData);
	TaskId byeWorldTask = delayedComplexObject->CreateTask(byeWorld, emptyTaskData);

	delayedComplexObject->ScheduleAt(std::chrono::system_clock::now() + std::chrono::seconds(1), helloWorldTask);
	delayedComplexObject->ScheduleAt(std::chrono::system_clock::now() + std::chrono::seconds(3), byeWorldTask);
	delayedComplexObject->ScheduleAt(std::chrono::system_clock::now() + std::chrono::seconds(6), stopTask);
	delayedComplexObject->MainLoop();
}

void PromiseTest()
{
	TaskScheduler taskScheduler(8);
	taskScheduler.Start();
	Task::TaskData stopSchedulerTask;
	stopSchedulerTask.StopData.Scheduler = &taskScheduler;
	TaskId stopTask = taskScheduler.CreateTask(StopSchedulerTask, stopSchedulerTask);

	Promise<int> promise;
	Future<int> future = promise.GetFuture();
	Task::TaskData promiseData;
	promiseData.FutureData.Future = &future;
	TaskId futureTask = taskScheduler.CreateTask(ReadFutureTask, promiseData, stopTask);


	
	taskScheduler.AddTask(futureTask);
	taskScheduler.AddTask(stopTask);

	massAddition(stopSchedulerTask);
	massAddition(stopSchedulerTask);
	massAddition(stopSchedulerTask);

	promise = 42;
	taskScheduler.MainLoop();
}

void TripleBuffering()
{
	TripleBufferedItem<int> tripleBufferingInt;
	tripleBufferingInt.ProducerValue() = 5;
	tripleBufferingInt.Commit();
	std::thread thread([&]()
	{
		int lastValue = 0;
		while (tripleBufferingInt.ConsumerValue() < 10)
		{
			tripleBufferingInt.Fetch();
			if (tripleBufferingInt.ConsumerValue() != lastValue)
			{
				std::cout << tripleBufferingInt.ConsumerValue() << " ";
				lastValue = tripleBufferingInt.ConsumerValue();
			}
		}
		std::cout << std::endl;
	});

	while (tripleBufferingInt.ProducerValue() < 10)
	{
		using namespace std::literals;
		std::this_thread::sleep_for(1s);
		tripleBufferingInt.ProducerValue() = tripleBufferingInt.ProducerValue() + 1;
		tripleBufferingInt.Commit();
	}
	thread.join();
}

int main()
{
	std::cout << "====== Begin Test: Triple Buffering =====" << std::endl;
	TripleBuffering();
	std::cout << "====== End Test: Triple Buffering =====" << std::endl;

	std::cout << "====== Begin Test: DelayedInit =====" << std::endl;
	DelayedInit();
	std::cout << "====== End Test: DelayedInit =====" << std::endl;

	std::cout << "====== Begin Test: Promise =====" << std::endl;
	PromiseTest();
	std::cout << "====== End Test: Promise =====" << std::endl;

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

	std::cout << "====== Begin Test: ExecuteMassTask =====" << std::endl;
	ExecuteMassTask();
	std::cout << "====== End Test: ExecuteMassTask =====" << std::endl;

	system("PAUSE");
    return 0;
}

