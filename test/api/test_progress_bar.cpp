#ifndef DUCKDB_NO_THREADS

#include <duckdb/execution/executor.hpp>
#include <future>
#include "catch.hpp"
#include "test_helpers.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/common/progress_bar.hpp"

#include <thread>
#include <future>

using namespace duckdb;
using namespace std;

class TestProgressBar {
public:
	explicit TestProgressBar(ClientContext *context) : context(context) {
	}

	ClientContext *context;
	bool stop;
	std::thread check_thread;

	void CheckProgressThread() {
		int prev_percentage = -1;
		while (!stop) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			int new_percentage = context->GetProgress();
			REQUIRE((new_percentage >= prev_percentage || new_percentage == -1));
			REQUIRE(new_percentage <= 100);
		}
	}
	void Start() {
		stop = false;
		check_thread = std::thread(&TestProgressBar::CheckProgressThread, this);
	}
	void End() {
		stop = true;
		check_thread.join();
	}
};

TEST_CASE("Test Progress Bar", "[api]") {
	unique_ptr<QueryResult> result;
	DuckDB db(nullptr);
	Connection con(db);
	con.context->test = true;
	TestProgressBar test_progress(con.context.get());
	REQUIRE_NO_FAIL(con.Query("create  table tbl as select range a, mod(range,10) b from range(10000000);"));
	REQUIRE_NO_FAIL(con.Query("create  table tbl_2 as select range a from range(10000000);"));

	REQUIRE_NO_FAIL(con.Query("PRAGMA set_progress_bar_time=10"));
	//! Simple Aggregation
	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl"));
	test_progress.End();

	//! Simple Join
	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl inner join tbl_2 on (tbl.a = tbl_2.a)"));
	test_progress.End();

	//! Subquery
	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl where a = (select min(a) from tbl_2)"));
	test_progress.End();

	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl where a = (select min(b) from tbl)"));
	test_progress.End();

	//! Test Multiple threads
	REQUIRE_NO_FAIL(con.Query("PRAGMA threads=4"));
	REQUIRE_NO_FAIL(con.Query("PRAGMA force_parallelism"));

	//! Simple Aggregation
	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl"));
	test_progress.End();

	//! Simple Join
	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl inner join tbl_2 on (tbl.a = tbl_2.a)"));
	test_progress.End();

	//! Subquery
	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl where a = (select min(a) from tbl_2)"));
	test_progress.End();

	test_progress.Start();
	REQUIRE_NO_FAIL(con.Query("select count(*) from tbl where a = (select min(b) from tbl)"));
	test_progress.End();
}
#endif