// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"

using namespace xbox::httpclient;

NAMESPACE_XBOX_HTTP_CLIENT_BEGIN

void raise_task_event(
    _In_ const std::shared_ptr<http_singleton>& httpSingleton,
    _In_ HC_TASK* task,
    _In_ HC_TASK_EVENT_TYPE eventType
    )
{
    std::map<HC_TASK_EVENT_HANDLE, HC_TASK_EVENT_FUNC_NODE> taskEventFuncList;
    {
        std::lock_guard<std::mutex> guard(httpSingleton->m_taskEventListLock);
        taskEventFuncList = httpSingleton->m_taskEventFuncList;
    }

    for (const auto& eventFunc : taskEventFuncList)
    {
        auto taskEventContext = eventFunc.second.taskEventFuncContext;
        auto taskEvent = eventFunc.second.taskEventFunc;
        auto taskEventSubsystemId = eventFunc.second.taskSubsystemId;
        if (taskEvent != nullptr && task->taskSubsystemId == taskEventSubsystemId)
        {
            taskEvent(taskEventContext, eventType, task->id);
        }
    }
}

void http_task_queue_pending(_In_ HC_TASK* task)
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return;

    task->state = http_task_state::pending;
    {
        std::lock_guard<std::mutex> guard(httpSingleton->m_taskLock);
        auto& taskPendingQueue = httpSingleton->get_task_pending_queue(task->taskSubsystemId);
        taskPendingQueue.push(task);

        HC_TRACE_INFORMATION(HTTPCLIENT, "Task queue pending: queueSize=%zu taskId=%llu",
            taskPendingQueue.size(), task->id);
    }

    raise_task_event(httpSingleton, task, HC_TASK_EVENT_PENDING);
    httpSingleton->set_task_pending_ready();
}

HC_TASK* http_task_get_next_pending(_In_ HC_SUBSYSTEM_ID taskSubsystemId)
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return nullptr;

    std::lock_guard<std::mutex> guard(httpSingleton->m_taskLock);
    auto& taskPendingQueue = httpSingleton->get_task_pending_queue(taskSubsystemId);
    if (!taskPendingQueue.empty())
    {
        auto it = taskPendingQueue.front();
        taskPendingQueue.pop();
        return it;
    }
    return nullptr;
}

void http_task_process_pending(_In_ HC_TASK* task)
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return;

    task->state = http_task_state::processing;

    {
        std::lock_guard<std::mutex> guard(httpSingleton->m_taskLock);
        auto& taskExecutingQueue = httpSingleton->m_taskExecutingQueue;
        taskExecutingQueue.push_back(task);

        HC_TRACE_INFORMATION(HTTPCLIENT, "Task execute: executeQueueSize=%zu taskId=%llu",
            taskExecutingQueue.size(), task->id);
    }

    if (task->executionRoutine != nullptr)
    {
        raise_task_event(httpSingleton, task, HC_TASK_EVENT_EXECUTE_STARTED);

        task->executionRoutine(
            task->executionRoutineContext,
            task->id
            );
    }
}

void http_task_queue_completed(_In_ HC_TASK_HANDLE taskHandleId)
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return;

    HC_TASK* taskHandle = http_task_get_task_from_handle_id(taskHandleId);
    if (taskHandle == nullptr)
        return; // invalid or old taskHandleId ?

    taskHandle->state = http_task_state::completed;

    HC_TASK* task = nullptr;
    {
        std::lock_guard<std::mutex> guard(httpSingleton->m_taskLock);
        auto& taskProcessingQueue = httpSingleton->m_taskExecutingQueue;
        for (auto& it : taskProcessingQueue)
        {
            if (it == taskHandle)
            {
                task = it;
            }
        }

        if (task != nullptr)
        {
            taskProcessingQueue.erase(std::remove(taskProcessingQueue.begin(), taskProcessingQueue.end(), task), taskProcessingQueue.end());
            auto& taskCompletedQueue = httpSingleton->get_task_completed_queue_for_taskgroup(taskHandle->taskSubsystemId, taskHandle->taskGroupId)->get_completed_queue();
            taskCompletedQueue.push(task);

            HC_TRACE_INFORMATION(HTTPCLIENT, "Task queue completed: queueSize=%zu taskId=%llu", taskCompletedQueue.size(), task->id);
        }
        else
        {
            HC_TRACE_ERROR(HTTPCLIENT, "Task not found: taskHandleId=%llu", taskHandleId);
        }
    }

#if HC_USE_HANDLES
    SetEvent(taskHandle->eventTaskCompleted.get());
    httpSingleton->get_task_completed_queue_for_taskgroup(taskHandle->taskSubsystemId, taskHandle->taskGroupId)->set_task_completed_event();
#endif

    raise_task_event(httpSingleton, task, HC_TASK_EVENT_EXECUTE_COMPLETED);
}

HC_TASK* http_task_get_next_completed(_In_ HC_SUBSYSTEM_ID taskSubsystemId, _In_ uint64_t taskGroupId)
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return nullptr;

    std::lock_guard<std::mutex> guard(httpSingleton->m_taskLock);
    auto& completedQueue = httpSingleton->get_task_completed_queue_for_taskgroup(taskSubsystemId, taskGroupId)->get_completed_queue();
    if (!completedQueue.empty())
    {
        auto it = completedQueue.front();
        completedQueue.pop();
        return it;
    }
    return nullptr;
}

void http_task_process_completed(_In_ HC_TASK* task)
{
    if (task->writeResultsRoutine != nullptr)
    {
        task->writeResultsRoutine(
            task->writeResultsRoutineContext,
            task->id,
            task->completionRoutine,
            task->completionRoutineContext
        );
    }
}

HC_TASK* http_task_get_task_from_handle_id(
    _In_ HC_TASK_HANDLE taskHandleId
    )
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return nullptr;

    std::lock_guard<std::mutex> lock(httpSingleton->m_taskHandleIdMapLock);
    auto& taskHandleIdMap = httpSingleton->m_taskHandleIdMap;
    auto it = taskHandleIdMap.find(taskHandleId);
    if (it != taskHandleIdMap.end())
    {
        return it->second.get();
    }

    return nullptr;
}

void http_task_store_task_from_handle_id(
    _In_ HC_TASK_PTR task
    )
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return;

    std::lock_guard<std::mutex> lock(httpSingleton->m_taskHandleIdMapLock);
    auto& taskHandleIdMap = httpSingleton->m_taskHandleIdMap;
    taskHandleIdMap[task->id] = std::move(task);
}

void http_task_clear_task_from_handle_id(
    _In_ HC_TASK_HANDLE taskHandleId
    )
{
    auto httpSingleton = get_http_singleton(false);
    if (nullptr == httpSingleton)
        return;

    std::lock_guard<std::mutex> lock(httpSingleton->m_taskHandleIdMapLock);
    auto& taskHandleIdMap = httpSingleton->m_taskHandleIdMap;
    taskHandleIdMap.erase(taskHandleId);
}

NAMESPACE_XBOX_HTTP_CLIENT_END
