#include "Foundation/Threading/ThreadUtils.h"
#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderingThread.h>

ezUInt32 spRenderingThread::Run()
{
  while (!m_bCancellationRequested)
  {
    {
      EZ_LOCK(m_QueueMutex);

      if (!m_WorkQueue.IsEmpty())
      {
        m_CurrentWork = m_WorkQueue.PeekBack();
        m_WorkQueue.PopBack();
      }
    }

    if (m_CurrentWork.m_Action.IsValid())
    {
      Call(std::move(m_CurrentWork.m_Action), std::move(m_CurrentWork.m_Callback));

      if (m_CurrentWork.m_pThreadSignal != nullptr)
        m_CurrentWork.m_pThreadSignal->RaiseSignal();

      m_CurrentWork = {};
    }

    // Wait for new tasks
    m_ThreadSignal.WaitForSignal();
  }

  return 0;
}

spRenderingThread::spRenderingThread()
  : ezThread("spRenderingThread")
  , m_ThreadSignal(ezThreadSignal::Mode::AutoReset)
{
}

void spRenderingThread::Start()
{
  ezThread::Start();
}

void spRenderingThread::PostSync(WorkerFunction&& action)
{
  Post(std::move(action), {}, false);
}

void spRenderingThread::PostAsync(WorkerFunction&& action)
{
  Post(std::move(action), {}, true);
}

void spRenderingThread::PostAsync(WorkerFunction&& action, WorkerFunction&& onComplete)
{
  Post(std::move(action), std::move(onComplete), true);
}

void spRenderingThread::Stop()
{
  if (!IsRunning())
    return;

  m_bCancellationRequested.TestAndSet(false, true);
  m_ThreadSignal.RaiseSignal();

  {
    EZ_LOCK(m_QueueMutex);
    m_WorkQueue.Clear();
  }

  if (ezThreadUtils::GetCurrentThreadID() == GetThreadID())
    return;

  Join();
}

void spRenderingThread::Post(WorkerFunction&& action, WorkerFunction&& onComplete, bool bIsAsync)
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
    work.m_bIsAsync = bIsAsync;
    work.m_pThreadSignal = bIsAsync ? nullptr : &threadSignal;
  }

  m_ThreadSignal.RaiseSignal();

  if (!bIsAsync)
  {
    // Wait for the work to be executed
    threadSignal.WaitForSignal();
  }
}

void spRenderingThread::Call(WorkerFunction&& action, WorkerFunction&& onComplete)
{
  action();

  if (onComplete.IsValid())
    onComplete();
}
