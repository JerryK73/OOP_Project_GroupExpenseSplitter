cmake_minimum_required(VERSION 3.10)
project(ExpenseSplitter CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

add_executable(expense_splitter
    src/main.cpp
    src/Member.cpp
    src/Split.cpp
    src/EqualSplitStrategy.cpp
    src/CustomSplitStrategy.cpp
    src/Expense.cpp
    src/Settlement.cpp
    src/Group.cpp
    src/Database.cpp
    src/ExpenseSplitterCLI.cpp
)

target_include_directories(expense_splitter PRIVATE include ${SQLite3_INCLUDE_DIRS})
target_link_libraries(expense_splitter PRIVATE ${SQLite3_LIBRARIES})
