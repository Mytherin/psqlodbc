/*
 * This test case tests for a bug in result set caching, with
 * UseDeclareFetch=1, that was fixed. The bug occurred when a cursor was
 * closed, due to transaction commit, before any rows were fetched from
 * it. That set the "base" of the internal cached rowset incorrectly,
 * off by one.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void more_tests_handlestmt() {
	int			rc;
	HSTMT		hstmt1 = SQL_NULL_HSTMT;
	HSTMT		hstmt2 = SQL_NULL_HSTMT;

	/* Start a transaction */
	rc = SQLSetConnectAttr(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt1);
	CHECK_STMT_RESULT(rc, "SQLAllocHandle SQL_HANDLE_STMT failed", hstmt1);

	/* Try to commit without an open query statement */
	rc = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_COMMIT);
	CHECK_STMT_RESULT(rc, "SQLEndTran with SQL_COMMIT failed", hstmt1);

	rc = SQLPrepare(hstmt1, (SQLCHAR *) "SELECT ?::BOOL", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLPrepare failed", hstmt1);

	/* Now commit with a prepated statement */
	rc = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_COMMIT);
	CHECK_STMT_RESULT(rc, "SQLEndTran with SQL_COMMIT failed", hstmt1);

	release_statement(hstmt1);

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt1);
	CHECK_STMT_RESULT(rc, "SQLAllocHandle SQL_HANDLE_STMT failed", hstmt1);

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt2);
	CHECK_STMT_RESULT(rc, "SQLAllocHandle SQL_HANDLE_STMT failed", hstmt2);

	rc = SQLExecDirect(hstmt1, (SQLCHAR *) "SELECT g FROM generate_series(1,3) g(g)", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt1);

	rc = SQLExecDirect(hstmt2, (SQLCHAR *) "SELECT g FROM generate_series(1,3) g(g)", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt2);

	// free first statement
	rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
	CHECK_STMT_RESULT(rc, "SQLFreeHandle failed", hstmt1);

	/* Commit test after released the firt handle statement */
	rc = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_COMMIT);
	CHECK_STMT_RESULT(rc, "SQLEndTran with SQL_COMMIT failed", hstmt2);

	SQLINTEGER	longvalue;
	SQLLEN		indLongvalue;

	rc = SQLBindCol(hstmt2, 1, SQL_C_LONG, &longvalue, sizeof(SQLINTEGER), &indLongvalue);
	CHECK_STMT_RESULT(rc, "SQLBindCol failed", hstmt2);

	int	row = 1;
	while (SQLFetchScroll(hstmt2, SQL_FETCH_NEXT, 0) != SQL_NO_DATA)
	{
		if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
			test_printf("row %d: %d\n", row, longvalue);
		else
		{
			print_diag("SQLFetchScroll failed", SQL_HANDLE_STMT, hstmt2);
			REQUIRE(1==0);
		}
		row++;
	}

	// clean up statement
	release_statement(hstmt2);

	hstmt1 = nullptr;
}

TEST_CASE("cursor-commit-test", "[odbc]") {
	test_printf_reset();

	int			rc;
	HSTMT		hstmt = SQL_NULL_HSTMT;
	HSTMT		hstmt2 = SQL_NULL_HSTMT;
	SQLCHAR		charval[100];
	SQLLEN		len;
	int			row;

	test_connect();

	/* Start a transaction */
	rc = SQLSetConnectAttr(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

	rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
						(SQLPOINTER) SQL_CURSOR_STATIC, SQL_IS_UINTEGER);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);

	/*
	 * Begin executing a query
	 */
	rc = SQLExecDirect(hstmt, (SQLCHAR *) "SELECT g FROM generate_series(1,3) g(g)", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

	rc = SQLBindCol(hstmt, 1, SQL_C_CHAR, &charval, sizeof(charval), &len);
	CHECK_STMT_RESULT(rc, "SQLBindCol failed", hstmt);

	/* Commit. This implicitly closes the cursor in the server. */
	rc = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_COMMIT);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to commit", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

	rc = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 0);
	CHECK_STMT_RESULT(rc, "SQLFetchScroll(FIRST) failed", hstmt);

	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
		test_printf("first row: %s\n", charval);

	row = 1;
	while (1)
	{
		rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
		if (rc == SQL_NO_DATA)
			break;

		row++;

		if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
			test_printf("row %d: %s\n", row, charval);
		else
		{
			print_diag("SQLFetchScroll failed", SQL_HANDLE_STMT, hstmt);
			REQUIRE(1==0);
		}
	}

	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

	test_printf("\nTesting statement handles and SQLEndTran(commit)\n");
	// more tests for handle statement with the SQLEndTran
	more_tests_handlestmt();

	// clean up statement
	release_statement(hstmt);

	/* Clean up */
	test_disconnect();

	test_check_result("cursor-commit");

	return;
}
