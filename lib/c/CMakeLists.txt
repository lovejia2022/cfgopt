cmake_minimum_required(VERSION 3.29)

project(libcfgopt)

option(CFGOPT_build_test "Build test for cfgopt" ON)
option(CFGOPT_log "Print debugging logs for libcfgopt" OFF)

add_library(cfgopt src/cfgopt.c)

target_include_directories(cfgopt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(CFGOPT_log)
  target_compile_definitions(cfgopt PUBLIC -DCFGOPT_CONFIG_LOG=1)
endif()

if(CFGOPT_build_test)
  add_executable(cfgopt_test tests/cfgopt_test.c)
  target_link_libraries(cfgopt_test PRIVATE cfgopt)

  target_include_directories(cfgopt_test
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  add_executable(cfgopt_example tests/cfgopt_example.c)
  target_link_libraries(cfgopt_example PRIVATE cfgopt)
endif()
