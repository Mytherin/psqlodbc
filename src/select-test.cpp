#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "common.h"

TEST_CASE("select-test", "[odbc]") {
	int rc;
	HSTMT hstmt = SQL_NULL_HSTMT;
	char sql[100000];
	char *sqlend;
	int i;

	test_connect();

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

	rc = SQLExecDirect(hstmt, (SQLCHAR *) "SELECT 1 UNION ALL SELECT 2", SQL_NTS);

	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);
	print_result(hstmt);

	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

	/* Result set with 1600 cols */
	strcpy(sql, "SELECT 1");
	sqlend = &sql[strlen(sql)];
	for (i = 2; i <= 1600; i++)
	{
		sprintf(sqlend, ",%d", i);
		sqlend += strlen(sqlend);
	}
	*sqlend = '\0';

	rc = SQLExecDirect(hstmt, (SQLCHAR *) sql, SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);
	print_result(hstmt);

	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

	rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("SQLFreeHandle failed", SQL_HANDLE_STMT, hstmt);
		fflush(stdout);
		REQUIRE(1==0);
	}
	hstmt = NULL;

	/* Clean up */
	test_disconnect();

	return;
}
