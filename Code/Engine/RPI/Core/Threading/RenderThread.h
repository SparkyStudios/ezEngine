#pragma once

#include <RPI/RPIDLL.h>

#include <Foundation/Threading/Thread.h>
#include <Foundation/Threading/ThreadSignal.h>

/// \brief The rendering thread.
///
/// Rendering operations should be executed inside this thread. You can use \a PostSync and
/// \a PostAsync to push functions to be executed synchronously or asynchronously respectively.
class SP_RPI_DLL spRenderThread final : ezThread
{
  typedef ezDelegate<void(), 128> WorkerFunction;

  // ezThread

public:
  ezUInt32 Run() override;

  // spRenderThread

public:
  spRenderThread();

  /// \brief Starts the thread.
  void Start();

  /// \brief Post a function to be executed synchronously in the render thread.
  /// The caller thread will block until the function is fully executed.
  /// \param action The function to execute.
  void PostSync(WorkerFunction&& action);

  /// \brief Post a function to be executed asynchronously in the render thread.
  /// The function will be pushed in a poll an executed later. This will not block
  /// the caller thread.
  /// \param action The function to execute.
  void PostAsync(WorkerFunction&& action);

  /// \brief Post a function to be executed asynchronously in the render thread.
  /// The function will be pushed in a poll an executed later. This will not block
  /// the caller thread.
  /// \param action The function to execute.
  /// \param onComplete An optional action to executed when the function has been
  /// fully executed.
  void PostAsync(WorkerFunction&& action, WorkerFunction&& onComplete);

  /// \brief Stops the thread.
  void Stop();

private:
  /// \brief A single task in the work queue.
  struct Work
  {
    WorkerFunction m_Action;
    WorkerFunction m_Callback{};
    ezThreadSignal* m_pThreadSignal{nullptr};
  };

  void Post(WorkerFunction&& action, WorkerFunction&& onComplete, bool bIsAsync);
  void Call(WorkerFunction&& action, WorkerFunction&& onComplete);

  ezAtomicBool m_bCancellationRequested{false};

  ezThreadSignal m_ThreadSignal;

  ezDeque<Work> m_WorkQueue;
  Work m_CurrentWork;

  ezMutex m_QueueMutex;
};
