#include <stdio.h>
#include <stdlib.h>

#include "common.h"

TEST_CASE("boolsaschar-test", "[odbc]") {
	test_printf_reset();

	SQLRETURN rc;
	HSTMT hstmt = SQL_NULL_HSTMT;
	const char *param1;
	SQLLEN cbParam1;
	SQLSMALLINT colid;

	/* BoolsAsChar is the default, but just in case.. */
//	test_connect_ext("Database=contrib_regression;BoolsAsChar=1");
	test_connect();

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

    //! Init database
    initdb(hstmt);

	/**** A simple query with one text param ****/

	/* Prepare a statement */
	rc = SQLPrepare(hstmt, (SQLCHAR *) "SELECT id, t, b FROM booltab WHERE t = ?", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLPrepare failed", hstmt);

	/* bind param  */
	param1 = "yes";
	cbParam1 = SQL_NTS;
	rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
						  SQL_C_CHAR,	/* value type */
						  SQL_VARCHAR,	/* param type */
						  5,			/* column size */
						  0,			/* dec digits */
						  (SQLPOINTER) param1,		/* param value ptr */
						  strlen(param1),			/* buffer len */
						  &cbParam1		/* StrLen_or_IndPtr */);
	CHECK_STMT_RESULT(rc, "SQLBindParameter failed", hstmt);

	/* Execute */
	rc = SQLExecute(hstmt);
	CHECK_STMT_RESULT(rc, "SQLExecute failed", hstmt);

	/* Fetch result */
	colid = 3;
	print_result_meta_series(hstmt, &colid, 1);
	print_result(hstmt);

	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

	/**** A simple query with one boolean param (passed as varchar) ****/

	/* bind param  */
	param1 = "true";
	cbParam1 = SQL_NTS;
	rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
						  SQL_C_CHAR,	/* value type */
						  SQL_VARCHAR,	/* param type */
						  5,			/* column size */
						  0,			/* dec digits */
						  (SQLPOINTER) param1,		/* param value ptr */
						  strlen(param1),			/* buffer len */
						  &cbParam1		/* StrLen_or_IndPtr */);
	CHECK_STMT_RESULT(rc, "SQLBindParameter failed", hstmt);

	/* Execute */
	rc = SQLExecDirect(hstmt, (SQLCHAR *) "SELECT id, t, b FROM booltab WHERE b = ?", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

	/* Fetch result */
	colid = 3;
	print_result_meta_series(hstmt, &colid, 1);
	print_result(hstmt);

	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

	// clean up statement
	release_statement(hstmt);

	/* Clean up */
	test_disconnect();

	test_check_result("boolsaschar");

	return;
}
