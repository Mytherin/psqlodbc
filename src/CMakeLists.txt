add_library(
  psqlodbc_test_src OBJECT
      alter-test.cpp
	  common.cpp
      arraybinding-test.cpp
      bindcol-test.cpp
      bookmark-test.cpp
      boolsaschar-test.cpp
      bulkoperations-test.cpp
      catalogfunctions-test.cpp
      colattribute-test.cpp
      commands-test.cpp
      connect-test.cpp
      cte-test.cpp
      cursor-block-delete-test.cpp
      cursor-commit-test.cpp
      cursor-movement-test.cpp
      cursor-name-test.cpp
      cursors-test.cpp
      cvtnulldate-test.cpp
      dataatexecution-test.cpp
      declare-fetch-block-test.cpp
      declare-fetch-commit-test.cpp
      deprecated-test.cpp
      diagnostic-test.cpp
      error-rollback-test.cpp
      errors-test.cpp
      getresult-test.cpp
      identity-test.cpp
      insertreturning-test.cpp
      large-object-data-at-exec-test.cpp
      large-object-test.cpp
      lfconversion-test.cpp
      multistmt-test.cpp
      notice-test.cpp
      numeric-test.cpp
      odbc-escapes-test.cpp
      param-conversions-test.cpp
      params-batch-exec-test.cpp
      params-test.cpp
      parse-test.cpp
      positioned-update-test.cpp
      premature-test.cpp
      prepare-test.cpp
      quotes-test.cpp
      result-conversions-test.cpp
      row-wise-fetching-test.cpp
      select-test.cpp
      stmthandles-test.cpp
      update-test.cpp
    #   wchar-char-test-eucjp.c
    #   wchar-char-test-sjis.c
    #   wchar-char-test-utf8.c
      # wchar-char-test.cpp
	)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:psqlodbc_test_src>
    PARENT_SCOPE)
