// Copyright (c) 2023-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <RPI/RPIPCH.h>

#include <RPI/Core/Threading/RenderThread.h>

ezUInt32 spRenderThread::Run()
{
  while (!m_bCancellationRequested)
  {
    {
      EZ_LOCK(m_QueueMutex);

      if (!m_WorkQueue.IsEmpty())
      {
        m_CurrentWork = m_WorkQueue.PeekFront();
        m_WorkQueue.PopFront();
      }
    }

    if (m_CurrentWork.m_Action.IsValid())
    {
      Call(std::move(m_CurrentWork.m_Action), std::move(m_CurrentWork.m_Callback));

      if (m_CurrentWork.m_pThreadSignal != nullptr)
        m_CurrentWork.m_pThreadSignal->RaiseSignal();

      m_CurrentWork = {};
    }

    if (m_WorkQueue.IsEmpty())
    {
      // Wait for new tasks
      m_ThreadSignal.WaitForSignal();
    }
  }

  return 0;
}

spRenderThread::spRenderThread()
  : ezThread("spRenderingThread")
  , m_ThreadSignal(ezThreadSignal::Mode::AutoReset)
{
}

void spRenderThread::Start()
{
  ezThread::Start();
}

void spRenderThread::PostSync(WorkerFunction&& action)
{
  Post(std::move(action), {}, false);
}

void spRenderThread::PostAsync(WorkerFunction&& action)
{
  Post(std::move(action), {}, true);
}

void spRenderThread::PostAsync(WorkerFunction&& action, WorkerFunction&& onComplete)
{
  Post(std::move(action), std::move(onComplete), true);
}

void spRenderThread::Stop()
{
  if (!IsRunning())
    return;

  {
    EZ_LOCK(m_QueueMutex);
    m_WorkQueue.Clear();
  }

  m_bCancellationRequested.TestAndSet(false, true);
  m_ThreadSignal.RaiseSignal();

  if (ezThreadUtils::GetCurrentThreadID() == GetThreadID())
    return;

  Join();
}

void spRenderThread::Post(WorkerFunction&& action, WorkerFunction&& onComplete, bool bIsAsync)
{
  // If the caller is the same thread, we don't need to block or enqueue
  if (ezThreadUtils::GetCurrentThreadID() == GetThreadID())
  {
    Call(std::move(action), std::move(onComplete));
    return;
  }

  ezThreadSignal threadSignal(ezThreadSignal::Mode::ManualReset);

  {
    EZ_LOCK(m_QueueMutex);

    Work& work = m_WorkQueue.ExpandAndGetRef();

    std::swap(work.m_Action, action);
    std::swap(work.m_Callback, onComplete);
    work.m_pThreadSignal = bIsAsync ? nullptr : &threadSignal; // We can safely pass the pointer here, since we know the object won't get destroyed
  }

  m_ThreadSignal.RaiseSignal();

  if (!bIsAsync)
  {
    // Wait for the work to be executed
    threadSignal.WaitForSignal();
  }
}

void spRenderThread::Call(WorkerFunction&& action, WorkerFunction&& onComplete)
{
  action();

  if (onComplete.IsValid())
    onComplete();
}

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Core_RenderThread);
