#include <gtest/gtest.h>

#include <engine/async.hpp>
#include <engine/sleep.hpp>
#include <engine/task/task_context.hpp>

#include <utest/utest.hpp>

// TAXICOMMON-347 -- Task closure gets destroyed in TaskProcessor::ProcessTasks.
//
// The task was detached (no coroutine ownership) and it got cancelled as
// the processor shut down. As the task hasn't started yet, we invoked the fast
// path that did not acquire a coroutine, and payload was destroyed in
// an unexpected context.
TEST(TaskContext, DetachedAndCancelledOnStart) {
  class DtorInCoroChecker final {
   public:
    ~DtorInCoroChecker() {
      EXPECT_NE(nullptr,
                engine::current_task::GetCurrentTaskContextUnchecked());
    }
  };

  static constexpr size_t kWorkerThreads = 1;

  RunInCoro(
      [] {
        auto task = engine::impl::Async([checker = DtorInCoroChecker()] {
          FAIL() << "Cancelled task started";
        });
        task.RequestCancel();
        std::move(task).Detach();
      },
      kWorkerThreads);

  // Same, but with WrappedCall closure
  RunInCoro(
      [] {
        auto task = engine::impl::Async(
            [](auto&&) { FAIL() << "Cancelled task started"; },
            DtorInCoroChecker());
        task.RequestCancel();
        std::move(task).Detach();
      },
      kWorkerThreads);
}

TEST(TaskContext, WaitInterruptedReason) {
  RunInCoro([] {
    auto long_task = engine::impl::Async(
        [] { engine::InterruptibleSleepFor(std::chrono::seconds{5}); });
    auto waiter = engine::impl::Async([&] {
      auto reason = engine::Task::CancellationReason::kNone;
      try {
        long_task.Get();
      } catch (const engine::WaitInterruptedException& ex) {
        reason = ex.Reason();
      }
      EXPECT_EQ(engine::Task::CancellationReason::kUserRequest, reason);
    });

    engine::Yield();
    ASSERT_EQ(engine::Task::State::kSuspended, waiter.GetState());
    waiter.RequestCancel();
    waiter.Get();
  });
}
