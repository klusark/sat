project(gif)

SET(CMAKE_CXX_FLAGS "-std=c++11 -g -O0 -Wall")

add_executable(random random.cc)

add_executable(sudoku2sat sudoku2sat.cc)
add_executable(sudoku2sat2 sudoku2sat2.cc)

add_executable(direct solver.cc)

add_executable(sgen sgen.cc)

add_executable(fixformat fixformat.cc)
