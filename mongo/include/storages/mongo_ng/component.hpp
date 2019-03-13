#pragma once

/// @file storages/mongo_ng/component.hpp
/// @brief @copybrief components::MongoNg

#include <memory>
#include <unordered_map>

#include <components/component_base.hpp>
#include <components/component_config.hpp>
#include <components/component_context.hpp>
#include <storages/mongo_ng/pool.hpp>
#include <storages/mongo_ng/pool_config.hpp>
#include <storages/secdist/component.hpp>
#include <utils/swappingsmart.hpp>

namespace components {

// clang-format off

/// @brief MongoDB client component
///
/// Provides access to a MongoDB database.
///
/// ## Configuration example:
///
/// ```
/// mongo-taxi:
///   dbalias: taxi
///   appname: userver-sample
///   conn_timeout: 5s
///   so_timeout: 10s
///   queue_timeout: 1s
///   initial_size: 4
///   max_size: 128
///   idle_limit: 32
///   connecting_limit: 8
/// ```
/// You must specify one of `dbalias` or `dbconnection`.
///
/// ## Available options:
/// Name | Description | Default value
/// ---- | ----------- | -------------
/// dbalias | name of the database in secdist config (if available) | --
/// dbconnection | connection string (used if no dbalias specified) | --
/// appname | application name for the DB server | userver
/// conn_timeout | connection timeout | 5s
/// so_timeout | socket timeout | 10s
/// queue_timeout | max connection queue wait time | 1s
/// initial_size | number of connections created initially | 4
/// max_size | limit for total connections number | 128
/// idle_limit | limit for idle connections number | 32
/// connecting_limit | limit for establishing connections number | 8

// clang-format on

class MongoNg : public LoggableComponentBase {
 public:
  /// Component constructor
  MongoNg(const ComponentConfig&, const ComponentContext&);

  /// Client pool accessor
  storages::mongo_ng::PoolPtr GetPool() const;

 private:
  storages::mongo_ng::PoolPtr pool_;
};

// clang-format off

/// @brief Dynamically configurable MongoDB client component
///
/// Provides acces to a dynamically reconfigurable set of MongoDB databases.
///
/// ## Configuration example:
///
/// ```
/// multi-mongo:
///   appname: userver-sample
///   conn_timeout: 5s
///   so_timeout: 10s
///   queue_timeout: 1s
///   initial_size: 4
///   max_size: 128
///   idle_limit: 32
///   connecting_limit: 8
/// ```
///
/// ## Available options:
/// Name | Description | Default value
/// ---- | ----------- | -------------
/// appname | application name for the DB server | userver
/// conn_timeout | connection timeout | 5s
/// so_timeout | socket timeout | 10s
/// queue_timeout | max connection queue wait time | 1s
/// initial_size | number of connections created initially (per database) | 4
/// max_size | limit for total connections number (per database) | 128
/// idle_limit | limit for idle connections number (per database) | 32
/// connecting_limit | limit for establishing connections number (per database) | 8

// clang-format on

class MultiMongoNg : public LoggableComponentBase {
 private:
  using PoolMap = std::unordered_map<std::string, storages::mongo_ng::PoolPtr>;

 public:
  static constexpr const char* kName = "multi-mongo";

  /// Component constructor
  MultiMongoNg(const ComponentConfig&, const ComponentContext&);

  /// Client pool accessor
  /// @param dbalias name previously passed to `AddPool`
  /// @throws PoolNotFound if no such database is enabled
  storages::mongo_ng::PoolPtr GetPool(const std::string& dbalias) const;

  /// @brief Adds a database to the working set by its name
  /// Equivalent to
  /// `NewPoolSet()`-`AddExistingPools()`-`AddPool(dbalias)`-`Activate()`
  /// @param dbalias name of the database in secdist config
  void AddPool(std::string dbalias);

  /// @brief Removes the database with the specified name from the working set
  /// Equivalent to
  /// `NewPoolSet()`-`AddExistingPools()`-`RemovePool(dbalias)`-`Activate()`
  /// @param dbalias name of the database passed to AddPool
  /// @returns whether the database was in the working set
  bool RemovePool(const std::string& dbalias);

  /// Database set builder
  class PoolSet {
   public:
    explicit PoolSet(MultiMongoNg&);

    PoolSet(const PoolSet&);
    PoolSet(PoolSet&&) noexcept;
    PoolSet& operator=(const PoolSet&);
    PoolSet& operator=(PoolSet&&) noexcept;

    /// Adds all currently enabled databases to the set
    void AddExistingPools();

    /// @brief Adds a database to the set by its name
    /// @param dbalias name of the database in secdist config
    void AddPool(std::string dbalias);

    /// @brief Removes the database with the specified name from the set
    /// @param dbalias name of the database passed to AddPool
    /// @returns whether the database was in the set
    bool RemovePool(const std::string& dbalias);

    /// @brief Replaces the working database set of the component
    void Activate();

   private:
    MultiMongoNg* target_;
    std::shared_ptr<PoolMap> pool_map_ptr_;
  };

  /// Creates an empty database set bound to the component
  PoolSet NewPoolSet();

 private:
  storages::mongo_ng::PoolPtr FindPool(const std::string& dbalias) const;

  const std::string name_;
  const Secdist& secdist_;
  const storages::mongo_ng::PoolConfig pool_config_;
  utils::SwappingSmart<PoolMap> pool_map_ptr_;
};

}  // namespace components