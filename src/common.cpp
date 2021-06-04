#include "common.h"

SQLHENV env;
SQLHDBC conn;

void
print_diag(const char *msg, SQLSMALLINT htype, SQLHANDLE handle)
{
	char		sqlstate[32];
	char		message[1000];
	SQLINTEGER	nativeerror;
	SQLSMALLINT textlen;
	SQLRETURN	ret;
	SQLSMALLINT	recno = 0;

	if (msg)
		test_printf("%s\n", msg);

	do
	{
		recno++;
		ret = SQLGetDiagRec(htype, handle, recno, (SQLCHAR *) sqlstate, &nativeerror,
							(SQLCHAR *) message, sizeof(message), &textlen);
		if (ret == SQL_INVALID_HANDLE)
            test_printf("Invalid handle\n");
		else if (SQL_SUCCEEDED(ret))
            test_printf("%s=%s\n", sqlstate, message);
	} while (ret == SQL_SUCCESS);

	if (ret == SQL_NO_DATA && recno == 1)
        test_printf("No error information\n");
}

const char * const default_dsn = "duckdbmemory";
const char * const test_dsn_env = "PSQLODBC_TEST_DSN";
const char * const test_dsn_ansi = "psqlodbc_test_dsn_ansi";

const char *get_test_dsn(void)
{
	char	*env = getenv(test_dsn_env);

	if (NULL != env && env[0]) {

		return env;
	}
    test_printf("Environment variable \"%s\" not defined... using default DSN \"%s\".", test_dsn_env, default_dsn);
	return default_dsn;
}

int IsAnsi(void)
{
	return (strcmp(get_test_dsn(), test_dsn_ansi) == 0);
}

void
test_connect_ext(const char *extraparams)
{
	SQLRETURN ret;
	SQLCHAR str[1024];
	SQLSMALLINT strl;
	SQLCHAR dsn[1024];
	const char * const test_dsn = get_test_dsn();
	char *envvar;

	/*
	 *	Use an environment variable to switch settings of connection
	 *	strings throughout the regression test. Note that extraparams
	 *	parameters have precedence over the environment variable.
	 *	ODBC spec says
	 *		If any keywords are repeated in the connection string,
	 *		the driver uses the value associated with the first
	 *		occurrence of the keyword.
	 *	But the current psqlodbc driver uses the value associated with
	 *	the last occurrence of the keyword. Here we place extraparams
	 *	both before and after the value of the environment variable
	 *	so as to protect the priority order whichever way we take.
	 */
	if ((envvar = getenv("COMMON_CONNECTION_STRING_FOR_REGRESSION_TEST")) != NULL && envvar[0] != '\0')
	{
		if (NULL == extraparams)
			snprintf((char *) dsn, sizeof(dsn), "DSN=%s;%s", test_dsn, envvar);
		else
			snprintf((char *) dsn, sizeof(dsn), "DSN=%s;%s;%s;%s",
			 test_dsn, extraparams, envvar, extraparams);
	}
	else
		snprintf((char *) dsn, sizeof(dsn), "DSN=%s;%s",
			 test_dsn, extraparams ? extraparams : "");

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

    printf("trying to connect to DSN \"%s\"...\n", dsn);

	SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
	ret = SQLDriverConnect(conn, NULL, dsn, SQL_NTS,
						   str, sizeof(str), &strl,
						   SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret)) {
        test_printf("connected\n");
	} else {
		print_diag("SQLDriverConnect failed.", SQL_HANDLE_DBC, conn);
		fflush(stdout);
		REQUIRE(1==0);
	}
}

void
test_connect(void)
{
	test_connect_ext(NULL);
}

void
test_disconnect(void)
{
	SQLRETURN rc;

    test_printf("disconnecting\n");
	rc = SQLDisconnect(conn);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("SQLDisconnect failed", SQL_HANDLE_DBC, conn);
		fflush(stdout);
		REQUIRE(1==0);
	}

	rc = SQLFreeHandle(SQL_HANDLE_DBC, conn);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("SQLFreeHandle failed", SQL_HANDLE_DBC, conn);
		fflush(stdout);
		REQUIRE(1==0);
	}
	conn = NULL;

	rc = SQLFreeHandle(SQL_HANDLE_ENV, env);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("SQLFreeHandle failed", SQL_HANDLE_ENV, env);
		fflush(stdout);
		REQUIRE(1==0);
	}
	env = NULL;
}

const char *
datatype_str(SQLSMALLINT datatype)
{
	static char buf[100];

	switch (datatype)
	{
		case SQL_CHAR:
			return "CHAR";
		case SQL_VARCHAR:
			return "VARCHAR";
		case SQL_LONGVARCHAR:
			return "LONGVARCHAR";
		case SQL_WCHAR:
			return "WCHAR";
		case SQL_WVARCHAR:
			return "WVARCHAR";
		case SQL_WLONGVARCHAR:
			return "WLONGVARCHAR";
		case SQL_DECIMAL:
			return "DECIMAL";
		case SQL_NUMERIC:
			return "NUMERIC";
		case SQL_SMALLINT:
			return "SMALLINT";
		case SQL_INTEGER:
			return "INTEGER";
		case SQL_REAL:
			return "REAL";
		case SQL_FLOAT:
			return "FLOAT";
		case SQL_DOUBLE:
			return "DOUBLE";
		case SQL_BIT:
			return "BIT";
		case SQL_TINYINT:
			return "TINYINT";
		case SQL_BIGINT:
			return "BIGINT";
		case SQL_BINARY:
			return "BINARY";
		case SQL_VARBINARY:
			return "VARBINARY";
		case SQL_LONGVARBINARY:
			return "LONGVARBINARY";
		case SQL_TYPE_DATE:
			return "TYPE_DATE";
		case SQL_TYPE_TIME:
			return "TYPE_TIME";
		case SQL_TYPE_TIMESTAMP:
			return "TYPE_TIMESTAMP";
		case SQL_GUID:
			return "GUID";
		default:
			snprintf(buf, sizeof(buf), "unknown sql type %d", datatype);
			return buf;
	}
}

const char *nullable_str(SQLSMALLINT nullable)
{
	static char buf[100];

	switch(nullable)
	{
		case SQL_NO_NULLS:
			return "not nullable";
		case SQL_NULLABLE:
			return "nullable";
		case SQL_NULLABLE_UNKNOWN:
			return "nullable_unknown";
		default:
			snprintf(buf, sizeof(buf), "unknown nullable value %d", nullable);
			return buf;
	}
}

void
print_result_meta_series(HSTMT hstmt,
						 SQLSMALLINT *colids,
						 SQLSMALLINT numcols)
{
	int i;

    test_printf("Result set metadata:\n");

	for (i = 0; i < numcols; i++)
	{
		SQLRETURN rc;
		SQLCHAR colname[50];
		SQLSMALLINT colnamelen;
		SQLSMALLINT datatype;
		SQLULEN colsize;
		SQLSMALLINT decdigits;
		SQLSMALLINT nullable;

		rc = SQLDescribeCol(hstmt, colids[i],
							colname, sizeof(colname),
							&colnamelen,
							&datatype,
							&colsize,
							&decdigits,
							&nullable);
		if (!SQL_SUCCEEDED(rc))
		{
			print_diag("SQLDescribeCol failed", SQL_HANDLE_STMT, hstmt);
			return;
		}
        test_printf("%s: %s(%u) digits: %d, %s\n",
			   colname, datatype_str(datatype), (unsigned int) colsize,
			   decdigits, nullable_str(nullable));
	}
}

void
print_result_meta(HSTMT hstmt)
{
	SQLRETURN rc;
	SQLSMALLINT numcols, i;
	SQLSMALLINT *colids;

	rc = SQLNumResultCols(hstmt, &numcols);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("SQLNumResultCols failed", SQL_HANDLE_STMT, hstmt);
		return;
	}

	colids = (SQLSMALLINT *) malloc(numcols * sizeof(SQLSMALLINT));
	for (i = 0; i < numcols; i++)
		colids[i] = i + 1;
	print_result_meta_series(hstmt, colids, numcols);
	free(colids);
}

/*
 * Initialize a buffer with "XxXxXx..." to indicate an uninitialized value.
 */
static void
invalidate_buf(char *buf, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
	{
		if (i % 2 == 0)
			buf[i] = 'X';
		else
			buf[i] = 'x';
	}
	buf[len - 1] = '\0';
}

/*
 * Print result only for the selected columns.
 */
void
print_result_series(HSTMT hstmt, SQLSMALLINT *colids, SQLSMALLINT numcols, SQLINTEGER rowcount)
{
	SQLRETURN rc;
	SQLINTEGER	rowc = 0;

    test_printf("Result set:\n");
	while (rowcount <0 || rowc < rowcount)
	{
		rc = SQLFetch(hstmt);
		if (rc == SQL_NO_DATA)
			break;
		if (rc == SQL_SUCCESS)
		{
			char buf[40];
			int i;
			SQLLEN ind;

			rowc++;
			for (i = 0; i < numcols; i++)
			{
				/*
				 * Initialize the buffer with garbage, so that we see readily
				 * if SQLGetData fails to set the value properly or forgets
				 * to null-terminate it.
				 */
				invalidate_buf(buf, sizeof(buf));
				rc = SQLGetData(hstmt, colids[i], SQL_C_CHAR, buf, sizeof(buf), &ind);
				if (!SQL_SUCCEEDED(rc))
				{
					print_diag("SQLGetData failed", SQL_HANDLE_STMT, hstmt);
					return;
				}
				if (ind == SQL_NULL_DATA)
					strcpy(buf, "NULL");
                test_printf("%s%s", (i > 0) ? "\t" : "", buf);
			}
            test_printf("\n");
		}
		else
		{
			print_diag("SQLFetch failed", SQL_HANDLE_STMT, hstmt);
			fflush(stdout);
			REQUIRE(1==0);
		}
	}
}

/*
 * Print result on all the columns
 */
void
print_result(HSTMT hstmt)
{
	SQLRETURN rc;
	SQLSMALLINT numcols, i;
	SQLSMALLINT *colids;

	rc = SQLNumResultCols(hstmt, &numcols);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("SQLNumResultCols failed", SQL_HANDLE_STMT, hstmt);
		return;
	}

	colids = (SQLSMALLINT *) malloc(numcols * sizeof(SQLSMALLINT));
	for (i = 0; i < numcols; i++)
		colids[i] = i + 1;
	print_result_series(hstmt, colids, numcols, -1);
	free(colids);
}

void run_sql(HSTMT hstmt, const char *sql) {
	SQLRETURN rc;

	/* Create a table to test with */
	rc = SQLExecDirect(hstmt, (SQLCHAR *) sql, SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);
}

void initdb(HSTMT hstmt) {
	run_sql(hstmt, "CREATE TABLE testtab1 (id integer PRIMARY KEY, t varchar(20));");
	run_sql(hstmt, "INSERT INTO testtab1 VALUES (1, 'foo');");
	run_sql(hstmt, "INSERT INTO testtab1 VALUES (2, 'bar');");
	run_sql(hstmt, "INSERT INTO testtab1 VALUES (3, 'foobar');");

	run_sql(hstmt, "CREATE TABLE booltab (id integer, t varchar(5), b boolean);");
	run_sql(hstmt, "INSERT INTO booltab VALUES (1, 'yeah', true);");
	run_sql(hstmt, "INSERT INTO booltab VALUES (2, 'yes', true);");
	run_sql(hstmt, "INSERT INTO booltab VALUES (3, 'true', true);");
	run_sql(hstmt, "INSERT INTO booltab VALUES (4, 'false', false)");
	run_sql(hstmt, "INSERT INTO booltab VALUES (5, 'not', false);");

	run_sql(hstmt, "CREATE TABLE byteatab (id integer, t blob);");
	run_sql(hstmt, "INSERT INTO byteatab VALUES (1, '\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x10'::blob);");
	run_sql(hstmt, "INSERT INTO byteatab VALUES (2, 'bar');");
	run_sql(hstmt, "INSERT INTO byteatab VALUES (3, 'foobar');");
	run_sql(hstmt, "INSERT INTO byteatab VALUES (4, 'foo');");
	run_sql(hstmt, "INSERT INTO byteatab VALUES (5, 'barf');");

	run_sql(hstmt, "CREATE TABLE intervaltable(id integer, iv interval, d varchar(100));");
	run_sql(hstmt, "INSERT INTO intervaltable VALUES (1, '1 day', 'one day');");
	run_sql(hstmt, "INSERT INTO intervaltable VALUES (2, '10 seconds', 'ten secs');");
	run_sql(hstmt, "INSERT INTO intervaltable VALUES (3, '100 years', 'hundred years');");

	run_sql(hstmt, "CREATE VIEW testview AS SELECT * FROM testtab1;");

	run_sql(hstmt, "CREATE TABLE lo_test_tab (id int4, large_data blob);");
}


static std::string test_printf_output;

std::string test_printf_get() {
    return test_printf_output;
}

void test_printf_reset() {
    test_printf_output  ="";
}


 void test_printf(const char* fmt, ...)
{
char buff[1024];
va_list args;
va_start(args, fmt);
vsprintf(buff, fmt, args);
va_end(args);
    test_printf_output += std::string(buff);
}


#include <string>
#include <fstream>
#include <cstdarg>


void test_check_result(std::string name) {
auto expect_filename = "expected/" + name + ".out";
std::ifstream in(expect_filename);
    std::string str((std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());

REQUIRE(test_printf_output == str);
}

