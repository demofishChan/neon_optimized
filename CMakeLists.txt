cmake_minimum_required (VERSION 2.8)

#设置单元测试文件名称
set(UNIT_TEST_FILE ./cvFilter2D3x3.cpp)

include_directories(common)

project (Neon_unit_test)
add_executable(Neon_unit_test ${UNIT_TEST_FILE})