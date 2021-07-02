#include <stdio.h>
#include <stdlib.h>

#include "common.h"

TEST_CASE("alter-test", "[odbc]") {
	test_printf_reset();

	SQLRETURN rc;
	HSTMT hstmt = SQL_NULL_HSTMT;

	test_connect();

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

	/* Create a table to test with */
	rc = SQLExecDirect(hstmt, (SQLCHAR *) "CREATE TABLE testtbl(t varchar(40))", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

	/**** A simple query against the table, fetch column info ****/

	rc = SQLExecDirect(hstmt, (SQLCHAR *) "SELECT * FROM testtbl", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

	/* Get column metadata */
	print_result_meta(hstmt);

	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

	/* Alter the table */
	rc = SQLExecDirect(hstmt, (SQLCHAR *) "ALTER TABLE testtbl ALTER t SET DATA TYPE varchar", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

	/* Run the query again, check if the metadata was updated */

	rc = SQLExecDirect(hstmt, (SQLCHAR *) "SELECT * FROM testtbl", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

	/* Get column metadata */
	print_result_meta(hstmt);

	/* releasing statement */
	release_statement(hstmt);

	/* Clean up */
	test_disconnect();

	test_check_result("alter");
}
