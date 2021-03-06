/*
 * Test SQLColAttribute
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

static void
runtest(const char *extra_conn_options)
{
	int rc;
	HSTMT hstmt = SQL_NULL_HSTMT;
	SQLUSMALLINT i;
	SQLSMALLINT numcols;

	// printf("Running tests with %s...\n", extra_conn_options);

	/* The behavior of these tests depend on the UnknownSizes parameter */
	test_connect_ext(extra_conn_options);

	rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

	/*
	 * Get column attributes of a simple query.
	 */
	test_printf("Testing SQLColAttribute...\n");
	rc = SQLExecDirect(hstmt,
			(SQLCHAR *) "SELECT '1'::int AS intcol, 'foobar'::text AS textcol, 'varchar string'::varchar as varcharcol, ''::varchar as empty_varchar_col, 'varchar-5-col'::varchar(5) as varchar5col, '5 days'::interval day to second",
			SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

	rc = SQLNumResultCols(hstmt, &numcols);
	CHECK_STMT_RESULT(rc, "SQLNumResultCols failed", hstmt);

	for (i = 1 ; i <= numcols; i++)
	{
		char buffer[64];
		SQLLEN number;

		rc = SQLColAttribute(hstmt, i, SQL_DESC_LABEL, buffer, sizeof(buffer), NULL, NULL);
		CHECK_STMT_RESULT(rc, "SQLColAttribute failed", hstmt);
		test_printf("\n-- Column %d: %s --\n", i, buffer);

		rc = SQLColAttribute(hstmt, i, SQL_DESC_OCTET_LENGTH, NULL, SQL_IS_INTEGER, NULL, &number);
		CHECK_STMT_RESULT(rc, "SQLColAttribute failed", hstmt);
		test_printf("SQL_DESC_OCTET_LENGTH: %d\n", (int) number);

		rc = SQLColAttribute(hstmt, i, SQL_DESC_TYPE_NAME, buffer, sizeof(buffer), NULL, NULL);
		CHECK_STMT_RESULT(rc, "SQLColAttribute failed", hstmt);
		test_printf("SQL_DESC_TYPE_NAME: %s\n", buffer);
	}

	// clean up statement
	release_statement(hstmt);

	/* Clean up */
	test_disconnect();
}

TEST_CASE("colattribute-test", "[odbc]") {
	test_printf_reset();

	/*
	 * The output of these tests depend on the UnknownSizes and
	 * MaxVarcharSize parameters
	 */
	// runtest("UnknownSizes=-1;MaxVarcharSize=100"); meaningless

	//! UnknownSizes and MaxVarcharSize are POSTGRES parameters, no matter for DuckDB
	runtest("UnknownSizes=0;MaxVarcharSize=100");

	// runtest("UnknownSizes=1;MaxVarcharSize=100");
	// runtest("UnknownSizes=2;MaxVarcharSize=100");
	// runtest("UnknownSizes=100;MaxVarcharSize=100"); meaningless

	test_check_result("colattribute");

	return;
}
