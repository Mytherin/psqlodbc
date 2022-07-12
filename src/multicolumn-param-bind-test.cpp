#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "common.h"

SQLHENV ENV;
SQLHDBC CONN;

/**
 * @brief Test case based on code of Steven Handerson from Verisk
 * 
 */

void doit_multicolumn()
{
    const char* Column11 = "Jon Doe", * Column12 = "John";
    const char* Column21 = "Jane Doe", * Column22 = "Jane";

    const int MAX_INSERT_COUNT = 2;
    const int MAX_BUFFER_SIZE = 100;
    char Column1Array[MAX_INSERT_COUNT][MAX_BUFFER_SIZE] = { 0 };
    char Column2Array[MAX_INSERT_COUNT][MAX_BUFFER_SIZE] = { 0 };
    SQLLEN cbColumn1Array[MAX_INSERT_COUNT] = { 0 };
    SQLLEN cbColumn2Array[MAX_INSERT_COUNT] = { 0 };

    SQLUSMALLINT  ParamStatusArray[MAX_INSERT_COUNT];
    SQLULEN ParamsProcessed;

    SQLHSTMT hStmt = NULL;
    SQLRETURN rc;
    test_printf("Doing multicolumn\n");

    rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hStmt);
	CHECK_STMT_RESULT(rc, "SQLAllocHandle failed", hStmt);

    rc = SQLExecDirect(hStmt, (SQLCHAR*)"CREATE TABLE SampleTable_2 (Column1 VARCHAR(100), Column2 VARCHAR(100))", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hStmt);

    rc = SQLFreeStmt(hStmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hStmt);
    hStmt = NULL;

    SQLAllocHandle(SQL_HANDLE_STMT, conn, &hStmt);
	CHECK_STMT_RESULT(rc, "SQLAllocHandle failed", hStmt);

    // Set the SQL_ATTR_PARAM_BIND_TYPE statement attribute to use  
    // column-wise binding.  
    SQLSetStmtAttr(hStmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0);

    // Specify an array in which to return the status of each set of  
    // parameters.  
    SQLSetStmtAttr(hStmt, SQL_ATTR_PARAM_STATUS_PTR, ParamStatusArray, 0);

    // Specify an SQLUINTEGER value in which to return the number of sets of  
    // parameters processed.  
    SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &ParamsProcessed, 0);

    rc = SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)MAX_INSERT_COUNT, 0);
    cbColumn1Array[0] = strlen(Column11);
    cbColumn1Array[1] = strlen(Column12);
    memcpy(Column1Array[0], Column11, strlen(Column11));
    memcpy(Column1Array[1], Column12, strlen(Column12));

    cbColumn2Array[0] = strlen(Column21);
    cbColumn2Array[1] = strlen(Column22);
    memcpy(Column2Array[0], Column21, strlen(Column21));
    memcpy(Column2Array[1], Column22, strlen(Column22));
    // SKH trying SQL_VARCHAR instead of SQL_CHAR
    rc = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_BUFFER_SIZE-1, 0, Column1Array, MAX_BUFFER_SIZE, cbColumn1Array);
	CHECK_STMT_RESULT(rc, "SQLBindParameter 1 failed", hStmt);

    rc = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_BUFFER_SIZE-1, 0, Column2Array, MAX_BUFFER_SIZE, cbColumn2Array);
	CHECK_STMT_RESULT(rc, "SQLBindParameter 2 failed", hStmt);

    rc = SQLExecDirect(hStmt, (SQLCHAR*)"INSERT INTO SampleTable_2 (Column1, Column2) VALUES(?,?)", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hStmt);

    // Check to see which sets of parameters were processed successfully.  
    for (int i = 0; i < ParamsProcessed; i++) {
        test_printf("Parameter Set  Status\n");
        test_printf("-------------  -------------\n");
        switch (ParamStatusArray[i]) {
        case SQL_PARAM_SUCCESS:
        case SQL_PARAM_SUCCESS_WITH_INFO:
            test_printf("%13d  Success\n", i);
            break;

        case SQL_PARAM_ERROR:
            test_printf("%13d  Error\n", i);
            break;

        case SQL_PARAM_UNUSED:
            test_printf("%13d  Not processed\n", i);
            break;

        case SQL_PARAM_DIAG_UNAVAILABLE:
            test_printf("%13d  Unknown\n", i);
            break;

        }
    }

	rc = SQLFreeStmt(hStmt, SQL_CLOSE);
	CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hStmt);

	rc = SQLExecDirect(hStmt, (SQLCHAR*)"SELECT * FROM SampleTable_2", SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hStmt);
	/* Get column metadata */
	print_result(hStmt);

	release_statement(hStmt);
}


TEST_CASE("multicolumn-param-bind-test", "[odbc]") {
	test_printf_reset();

	test_connect();

	doit_multicolumn();

	/* Clean up */
	test_disconnect();

	test_check_result("multicolumn-param-bind");

	return;
}
