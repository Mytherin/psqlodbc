/*
 * Tests for error diagnostics (SQLGetDiagRec)
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

TEST_CASE("diagnostic-test", "[odbc]") {
	test_printf_reset();

	int			rc;
	HSTMT		hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT	slen;
	char		sqlstate[32];
	char		buf[10000];

	test_connect();

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

	/*
	 * Execute an erroneous query, and call SQLGetDiagRec twice on the
	 * statement. Should get the same result both times; SQLGetDiagRec is
	 * not supposed to change the state of the statement.
	 */
	rc = SQLExecDirect(hstmt, (SQLCHAR *) "broken query", SQL_NTS);
	print_diag("SQLExecDirect", SQL_HANDLE_STMT, hstmt);
	print_diag("get same message again", SQL_HANDLE_STMT, hstmt);

	/*
	 * Test a very long error message.
	 */
	memset(buf, 'x', sizeof(buf) - 10);
	sprintf(buf + sizeof(buf) - 10, "END");
	rc = SQLExecDirect(hstmt, (SQLCHAR *) buf, SQL_NTS);
	print_diag("SQLExecDirect", SQL_HANDLE_STMT, hstmt);

	rc = SQLGetDiagField(SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_SQLSTATE,
						 sqlstate, sizeof(sqlstate), &slen);
	if (SQL_SUCCEEDED(rc))
		test_printf("SQL_DIAG_SQLSTATE=%s\n", sqlstate);
	else if (rc == SQL_INVALID_HANDLE)
		printf("Invalid handle\n");
	else
		printf("unexpected return code %d\n", rc);


	rc = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_ROLLBACK);
	print_diag("SQLEndTran", SQL_HANDLE_DBC, conn);
	print_diag("get same message again", SQL_HANDLE_DBC, conn);

	// CHECK_STMT_RESULT(rc, "SQLEndTran failed", hstmt);

	// rc = SQLFreeStmt(hstmt, SQL_DROP);
	// CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

	// /* kill this connection */
	// test_printf ("killing connection...\n");
	// rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	// if (!SQL_SUCCEEDED(rc))
	// {
	// 	print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
	// 	REQUIRE(1==0);
	// }

	// rc = SQLExecDirect(hstmt, (SQLCHAR *) "select pg_terminate_backend(pg_backend_pid());select 1;select 1 ", SQL_NTS);
	// print_diag(NULL, SQL_HANDLE_STMT, hstmt);

	/*
	 * Test SQLGetDiagRec on the connection, after the backend connection is
	 * dead. Twice, again to check that the first call doesn't clear the
	 * error.
	 */
	// print_diag("SQLGetDiagRec on connection says:", SQL_HANDLE_DBC, conn);
	// print_diag("SQLGetDiagRec called again:", SQL_HANDLE_DBC, conn);

	// clean up statement
	release_statement(hstmt);

	/* Clean up */
	test_disconnect();
	test_check_result("diagnostic");

	return;
}
