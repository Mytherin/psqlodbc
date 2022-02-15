This is a clone of the [PostgreSQL ODBC driver](https://www.postgresql.org/ftp/odbc/versions/src/). Specifically, this repo contains the tests that are defined there for usage with the DuckDB ODBC client.

# Running the tests

This directory should be cloned inside of a DuckDB directory, and `pwd` should be inside that directory.

```sh
git clone git@github.com:Mytherin/psqlodbc.git
```

After that, we build DuckDB with the ODBC driver enabled, and build the PSQL ODBC tests.

```sh
export BUILD_ODBC=1 make debug
(cd psqlodbc && make debug)
```

Now we can run the tests using the following script:

**Warning: this script will override your ODBC driver config (`~/.odbc.ini` and `~/.odbcinst.ini`).**

```sh
./tools/odbc/test/run_psqlodbc_tests.sh
```

# Fixing Output
If anything goes wrong running this script, we can run individual tests like this:

```sh
(cd psqlodbc && ./build/debug/psql_odbc_test [test_name])
```

If the test fails, but the new output looks correct (e.g. when you have changed something that causes the output to change), we can append `--fix` to this command to accept the new output as the correct output. For example:

```shell
(cd psqlodbc && ./build/debug/psql_odbc_test [test_name] --fix)
```

You may need to delete lingering database files first by running the following commands:

```sh
rm -f psqlodbc/contrib_regression
rm -f psqlodbc/contrib_regression.wal
```

