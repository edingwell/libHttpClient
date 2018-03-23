// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once
#include "task_impl.h"

enum http_task_state
{
    pending,
    processing,
    completed
};

struct HC_TASK
{
    HC_TASK() :
        state(http_task_state::pending),
        executionRoutine(nullptr),
        executionRoutineContext(nullptr),
        writeResultsRoutine(nullptr),
        writeResultsRoutineContext(nullptr),
        completionRoutine(nullptr),
        completionRoutineContext(nullptr),
        taskSubsystemId(HC_SUBSYSTEM_ID_GAME_MIN),
        taskGroupId(0),
        id(0)
    {
#if HC_USE_HANDLES
        eventTaskCompleted.set(CreateEvent(NULL, FALSE, FALSE, NULL));
#endif
    }

    http_task_state state;
    HC_TASK_EXECUTE_FUNC executionRoutine;
    void* executionRoutineContext;
    HC_TASK_WRITE_RESULTS_FUNC writeResultsRoutine;
    void* writeResultsRoutineContext;
    void* completionRoutine;
    void* completionRoutineContext;
    HC_SUBSYSTEM_ID taskSubsystemId;
    uint64_t taskGroupId;
    uint64_t id;

#if HC_USE_HANDLES
    win32_handle eventTaskCompleted;
#endif
};

typedef HC_UNIQUE_PTR< HC_TASK > HC_TASK_PTR;

NAMESPACE_XBOX_HTTP_CLIENT_BEGIN

void http_task_queue_pending(_In_ HC_TASK* info);
void http_task_process_pending(_In_ HC_TASK* task);
HC_TASK* http_task_get_next_pending(_In_ HC_SUBSYSTEM_ID taskSubsystemId);

void http_task_process_completed(_In_ HC_TASK* task);
void http_task_queue_completed(_In_ HC_TASK_HANDLE taskHandle);
HC_TASK* http_task_get_next_completed(_In_ HC_SUBSYSTEM_ID taskSubsystemId, _In_ uint64_t taskGroupId);

HC_TASK* http_task_get_task_from_handle_id(_In_ HC_TASK_HANDLE taskHandleId);
void http_task_store_task_from_handle_id(_In_ HC_TASK_PTR task);
void http_task_clear_task_from_handle_id(_In_ HC_TASK_HANDLE taskHandleId);

NAMESPACE_XBOX_HTTP_CLIENT_END

