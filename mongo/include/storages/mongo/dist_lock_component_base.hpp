#pragma once

#include <components/component_base.hpp>
#include <components/component_config.hpp>
#include <components/component_context.hpp>
#include <dist_lock/dist_locked_worker.hpp>
#include <storages/mongo/collection.hpp>
#include <storages/mongo/dist_lock_strategy.hpp>
#include <utils/statistics/storage.hpp>

namespace storages::mongo {

class DistLockComponentBase : public components::LoggableComponentBase {
 public:
  DistLockComponentBase(const components::ComponentConfig&,
                        const components::ComponentContext&,
                        storages::mongo::Collection);

  ~DistLockComponentBase();

  dist_lock::DistLockedWorker& GetWorker();

 protected:
  /// Override this function with anything that must be done under the mongo
  /// lock
  virtual void DoWork() = 0;

  /// Must be called in ctr
  void Start();

  /// Must be called in dtr
  void Stop();

 private:
  std::unique_ptr<dist_lock::DistLockedWorker> worker_;
  ::utils::statistics::Entry statistics_holder_;
};

}  // namespace storages::mongo