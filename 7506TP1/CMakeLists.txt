cmake_minimum_required(VERSION 3.2)
project(AlgebraRelacional)

#Esto setea que compile en bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#Aca van todos los flags del compilador
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -g -O0" )

#files en carpeta src, de forma recursiva(incluye archivos de carpetas adentro de la carpeta)
FILE(GLOB_RECURSE TESTS_SOURCE_FILES src/*)
#ejecutable tests
add_executable(tests ${TESTS_SOURCE_FILES})

#linkeos

