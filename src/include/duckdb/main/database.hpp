//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/main/database.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/main/config.hpp"
#include "duckdb/main/extension.hpp"

namespace duckdb {
class StorageManager;
class Catalog;
class TransactionManager;
class ConnectionManager;
class FileSystem;
class TaskScheduler;
class ObjectCache;

class DatabaseInstance : public std::enable_shared_from_this<DatabaseInstance> {
	friend class BufferManager;
	friend class ClientContext;
	friend class ObjectCache;
	friend class StorageManager;
	friend class DuckDB;
	friend class TaskScheduler;

public:
	DatabaseInstance();

	DBConfig config;

public:
	FileSystem &GetFileSystem();

	idx_t NumberOfThreads();

	//! Shutdown the database system, destroying it and invalidating any connections or clients
	void Shutdown();
	//! Returns true if the database system has been shut down
	bool IsShutdown();

private:
	void Initialize(const char *path, DBConfig *config);

	void Configure(DBConfig &config);

private:
	unique_ptr<StorageManager> storage;
	unique_ptr<Catalog> catalog;
	unique_ptr<TransactionManager> transaction_manager;
	unique_ptr<TaskScheduler> scheduler;
	unique_ptr<ObjectCache> object_cache;
};

//! The database object. This object holds the catalog and all the
//! database-specific meta information.
class DuckDB {
public:
	DuckDB(const char *path = nullptr, DBConfig *config = nullptr);
	DuckDB(const string &path, DBConfig *config = nullptr);
	~DuckDB();

	//! Reference to the actual database instance
	shared_ptr<DatabaseInstance> instance;

public:
	template <class T> void LoadExtension() {
		T extension;
		extension.Load(*this);
	}

	FileSystem &GetFileSystem();
	//! Shutdown the database system, destroying it and invalidating any connections or clients
	void Shutdown();

	idx_t NumberOfThreads();
	static const char *SourceID();
	static const char *LibraryVersion();
};

} // namespace duckdb
