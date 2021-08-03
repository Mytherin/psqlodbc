#include "common.h"

//* This test was based on the Microsoft ODBC guide in the following link:
//! https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/row-wise-binding?view=sql-server-ver15

SQLULEN ROW_ARRAY_SIZE = 10;

// Define the ORDERINFO struct and allocate an array of 10 structs.  
// LenOrInd structure members should by 8-byte aligned, i.e, SQLULEN,
// because of sanitizer might detect missaligng address in row-wise fetching
typedef struct {  
   SQLUINTEGER  OrderID;
   SQLULEN      OrderIDInd;
   SQLCHAR      SalesPerson[13];
   SQLULEN      SalesPersonLenOrInd;
   SQLCHAR      Status[8];
   SQLULEN      StatusLenOrInd;
} ORDERINFO;

typedef struct {  
   SQLCHAR              Bool[2]; // 2 chars because of \0
   SQLULEN              BoolInd;
   SQLUINTEGER          UInt;
   SQLULEN              UIntInd;
   SQLINTEGER           Int;
   SQLULEN              IntInd;
   SQLDOUBLE            Double;
   SQLULEN              DoubleInd;
   SQL_NUMERIC_STRUCT   Numeric;
   SQLULEN              NumericInd;
   SQLCHAR              Varchar[16];
   SQLULEN              VarcharLenOrInd;
   SQLDATE              Date[10]; //10 chars
   SQLULEN              DateLenOrInd;
} MANY_SQL_TYPES;

void print_order_addresses(ORDERINFO *OrderInfoArray, uint32_t array_size) {
    printf("Index\tOrderID\t\tOrderIDInd\tSalesPerson\tSalesPersonLen\tStatus\t\tStatusLenOrInd\n");
    printf("--------------------------------------------------------------------------------------------------------\n");
    for(int i=0; i < array_size; ++i) {
        printf("%d\t%p\t%p\t%p\t%p\t%p\t%p |\n", i, &OrderInfoArray[i].OrderID, &OrderInfoArray[i].OrderIDInd, &OrderInfoArray[i].SalesPerson,
                                            &OrderInfoArray[i].SalesPersonLenOrInd, &OrderInfoArray[i].Status, &OrderInfoArray[i].StatusLenOrInd);
    }
    printf("--------------------------------------------------------------------------------------------------------\n");
}

void test_microsoft_example() {
    SQLULEN         NumRowsFetched;
    SQLUSMALLINT    RowStatusArray[ROW_ARRAY_SIZE], i;
    SQLRETURN       rc;
    SQLHSTMT        hstmt;

    rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

    // Specify the size of the structure with the SQL_ATTR_ROW_BIND_TYPE
    // statement attribute. This also declares that row-wise binding will
    // be used. Declare the rowset size with the SQL_ATTR_ROW_ARRAY_SIZE 
    // statement attribute. Set the SQL_ATTR_ROW_STATUS_PTR statement
    // attribute to point to the row status array. Set the
    // SQL_ATTR_ROWS_FETCHED_PTR statement attribute to point to
    // NumRowsFetched.
    SQLULEN order_info_size = sizeof(ORDERINFO);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_BIND_TYPE, &order_info_size, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) ROW_ARRAY_SIZE, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_STATUS_PTR, RowStatusArray, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &NumRowsFetched, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);

    ORDERINFO OrderInfoArray[ROW_ARRAY_SIZE];

    // print_order_addresses(OrderInfoArray);

    // Bind elements of the first structure in the array to the OrderID,
    // SalesPerson, and Status columns.  
    rc = SQLBindCol(hstmt, 1, SQL_C_ULONG, &OrderInfoArray[0].OrderID, sizeof(OrderInfoArray[0].OrderID), (SQLLEN *)&OrderInfoArray[0].OrderIDInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 2, SQL_C_CHAR, &OrderInfoArray[0].SalesPerson, sizeof(OrderInfoArray[0].SalesPerson), (SQLLEN *)&OrderInfoArray[0].SalesPersonLenOrInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 3, SQL_C_CHAR, &OrderInfoArray[0].Status, sizeof(OrderInfoArray[0].Status), (SQLLEN *)&OrderInfoArray[0].StatusLenOrInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);

    SQLCHAR *SQL = (SQLCHAR *) "SELECT i AS OrderID, i::VARCHAR || 'SalesPerson' AS SalesPerson, i::VARCHAR || 'Status' AS Status FROM range(10) t(i)";

    // Execute a statement to retrieve rows from the Orders table.
    rc = SQLExecDirect(hstmt, SQL, SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);

    // Fetch up to the rowset size number of rows at a time. Print the actual
    // number of rows fetched; this number is returned in NumRowsFetched.
    // Check the row status array to print only those rows successfully
    // fetched. Code to check if rc equals SQL_SUCCESS_WITH_INFO or
    // SQL_ERRORnot shown.
    while ((rc = SQLFetch(hstmt)) != SQL_NO_DATA) {
        for (i = 0; i < NumRowsFetched; i++) {
            if (RowStatusArray[i] == SQL_ROW_SUCCESS|| RowStatusArray[i] == SQL_ROW_SUCCESS_WITH_INFO) {
                if (OrderInfoArray[i].OrderIDInd == SQL_NULL_DATA)
                    test_printf(" NULL      ");
                else
                    test_printf("%d\t", OrderInfoArray[i].OrderID);
                if (OrderInfoArray[i].SalesPersonLenOrInd == SQL_NULL_DATA)
                    test_printf(" NULL      ");
                else
                    test_printf("%s\t", OrderInfoArray[i].SalesPerson);
                if (OrderInfoArray[i].StatusLenOrInd == SQL_NULL_DATA)
                    test_printf(" NULL\n");
                else
                    test_printf("%s\n", OrderInfoArray[i].Status);
            }
        }
    }

	// clean up statement
	release_statement(hstmt);
}

void test_many_sql_types() {
    SQLULEN         NumRowsFetched;
    SQLUSMALLINT    RowStatusArray[ROW_ARRAY_SIZE], i;
    SQLRETURN       rc;
    SQLHSTMT        hstmt;

    rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
	if (!SQL_SUCCEEDED(rc))
	{
		print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
		REQUIRE(1==0);
	}

    uint64_t row_size = sizeof(MANY_SQL_TYPES);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_BIND_TYPE, &row_size, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) ROW_ARRAY_SIZE, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_STATUS_PTR, RowStatusArray, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &NumRowsFetched, 0);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);

    MANY_SQL_TYPES ManyTypesArray[ROW_ARRAY_SIZE];

    // Bind elements of the first structure in the array
    rc = SQLBindCol(hstmt, 1, SQL_C_CHAR, &ManyTypesArray[0].Bool, sizeof(ManyTypesArray[0].Bool), (SQLLEN *)&ManyTypesArray[0].BoolInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 2, SQL_C_ULONG, &ManyTypesArray[0].UInt, sizeof(ManyTypesArray[0].UInt), (SQLLEN *)&ManyTypesArray[0].UIntInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 3, SQL_C_LONG, &ManyTypesArray[0].Int, sizeof(ManyTypesArray[0].Int), (SQLLEN *)&ManyTypesArray[0].IntInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &ManyTypesArray[0].Double, sizeof(ManyTypesArray[0].Double), (SQLLEN *)&ManyTypesArray[0].DoubleInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 5, SQL_C_NUMERIC, &ManyTypesArray[0].Numeric, sizeof(ManyTypesArray[0].Numeric), (SQLLEN *)&ManyTypesArray[0].NumericInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 6, SQL_C_CHAR, &ManyTypesArray[0].Varchar, sizeof(ManyTypesArray[0].Varchar), (SQLLEN *)&ManyTypesArray[0].VarcharLenOrInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    rc = SQLBindCol(hstmt, 7, SQL_C_TYPE_DATE, &ManyTypesArray[0].Date, sizeof(ManyTypesArray[0].Date), (SQLLEN *)&ManyTypesArray[0].DateLenOrInd);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);

    SQLCHAR *SQL = (SQLCHAR *) "SELECT i::bool::char, i::uint8, i::int8, i::double, i::numeric, i::varchar || '-Varchar'," \
                                       "('200' || i::char || '-10-1' || i::char )::date FROM range(10) t(i)";

    // Execute a statement to retrieve rows from the Orders table.
    rc = SQLExecDirect(hstmt, SQL, SQL_NTS);
	CHECK_STMT_RESULT(rc, "SQLSetStmtAttr failed", hstmt);
    
    // Fetch up to the rowset size number of rows at a time. Print the actual
    // number of rows fetched; this number is returned in NumRowsFetched.
    // Check the row status array to print only those rows successfully
    // fetched. Code to check if rc equals SQL_SUCCESS_WITH_INFO or
    // SQL_ERRORnot shown.
    while ((rc = SQLFetch(hstmt)) != SQL_NO_DATA) {
        for (i = 0; i < NumRowsFetched; i++) {
            if (RowStatusArray[i] == SQL_ROW_SUCCESS|| RowStatusArray[i] == SQL_ROW_SUCCESS_WITH_INFO) {
                if (ManyTypesArray[i].BoolInd == SQL_NULL_DATA)
                    test_printf("NULL\t");
                else
                    test_printf("%s\t", ManyTypesArray[i].Bool);

                if (ManyTypesArray[i].UIntInd == SQL_NULL_DATA)
                    test_printf("NULL\t");
                else
                    test_printf("%d\t", ManyTypesArray[i].UInt);

                if (ManyTypesArray[i].IntInd == SQL_NULL_DATA)
                    test_printf("NULL\t");
                else
                    test_printf("%d\t", ManyTypesArray[i].Int);

                if (ManyTypesArray[i].DoubleInd == SQL_NULL_DATA)
                    test_printf("NULL\t");
                else
                    test_printf("%.3f\t", ManyTypesArray[i].Double);

                if (ManyTypesArray[i].NumericInd == SQL_NULL_DATA) {
                    test_printf("NULL\t");
                }
                else {
                    // fflush(stdout);
                    SQL_NUMERIC_STRUCT ns = ManyTypesArray[i].Numeric;
                    // printf("precision: %u scale: %d sign: %d val: ", ns.precision, ns.scale, ns.sign);
				    for (int val_idx = 0; val_idx < SQL_MAX_NUMERIC_LEN; val_idx++) {
					    test_printf("%02x", ns.val[val_idx]);
                    }
                    test_printf("\t");
                }

                if (ManyTypesArray[i].VarcharLenOrInd == SQL_NULL_DATA)
                    test_printf("NULL\t");
                else
                    test_printf("%s\t", ManyTypesArray[i].Varchar);

                if (ManyTypesArray[i].DateLenOrInd == SQL_NULL_DATA)
                    test_printf("NULL\t");
                else {
                    DATE_STRUCT *date = (DATE_STRUCT *) ManyTypesArray[i].Date;
                    test_printf("%d-%u-%u\n", date->year, date->month, date->day);
                }
            }
        }
    }

	// clean up statement
	release_statement(hstmt);
}


TEST_CASE("row-wise-fetching-test", "[odbc]") {
	test_printf_reset();

   	test_connect();

    test_microsoft_example();

    test_printf("\nTesting other types...\n\n");

    test_many_sql_types();

	/* Clean up */
	test_disconnect();

    test_check_result("row-wise-fetching");
}