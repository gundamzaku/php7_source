cmake_minimum_required(VERSION 3.7)
project(php7_source)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES sapi/cli/php_cli.c)

include_directories("main" "Zend" "/" "../ucrt")

add_executable(php7_source ${SOURCE_FILES})