#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "catch.hpp"

#ifdef WIN32
#include <windows.h>
#else
#include "config.h"
#endif

#include <sql.h>
#include <sqlext.h>

#ifdef WIN32
#define snprintf _snprintf
#endif


extern void test_printf(const char* fmt, ...);
extern std::string test_printf_get();
extern void test_printf_reset();

extern void test_check_result(std::string name);
extern void fix_result(std::string name);

extern void release_statement(HSTMT &hstmt);

extern SQLHENV env;
extern SQLHDBC conn;


#define CHECK_STMT_RESULT(rc, msg, hstmt)	\
	if (!SQL_SUCCEEDED(rc)) \
	{ \
		print_diag(msg, SQL_HANDLE_STMT, hstmt);	\
		REQUIRE(0==1);									\
    }

#define CHECK_CONN_RESULT(rc, msg, hconn)	\
	if (!SQL_SUCCEEDED(rc)) \
	{ \
		print_diag(msg, SQL_HANDLE_DBC, hconn);	\
		REQUIRE(0==1);									\
    }

extern void test_fix_results();
extern void print_diag(const char *msg, SQLSMALLINT htype, SQLHANDLE handle);
extern const char *get_test_dsn(void);
extern int  IsAnsi(void);
extern void test_connect_ext(const char *extraparams);
extern void test_connect(void);
extern void test_disconnect(void);
extern void print_result_meta_series(HSTMT hstmt,
                                     SQLSMALLINT *colids,
                                     SQLSMALLINT numcols);
extern void print_result_series(HSTMT hstmt,
                                SQLSMALLINT *colids,
                                SQLSMALLINT numcols,
                                SQLINTEGER rowcount);
extern void print_result_meta(HSTMT hstmt);
extern void print_result(HSTMT hstmt);
extern const char *datatype_str(SQLSMALLINT datatype);
extern const char *nullable_str(SQLSMALLINT nullable);
extern void run_sql(HSTMT hstmt, const char *sql);
extern void initdb(HSTMT hstmt);
