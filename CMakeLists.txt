project(psqlodbctest)

if (CMAKE_VERSION VERSION_LESS "3.1")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else ()
  set (CMAKE_CXX_STANDARD 11)
endif ()

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

find_package(ODBC REQUIRED)

include_directories(include)
include_directories(third_party/catch/)
include_directories(${ODBC_INCLUDE_DIRS})

add_subdirectory(src)

add_executable(psql_odbc_test main.cpp ${ALL_OBJECT_FILES})
target_link_libraries(psql_odbc_test ${ODBC_LIBRARIES})

add_executable(reset-db reset-db.cpp src/common.cpp)
target_link_libraries(reset-db ${ODBC_LIBRARIES})
