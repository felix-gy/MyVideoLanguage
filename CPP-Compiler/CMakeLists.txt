cmake_minimum_required(VERSION 3.30)
project(CPP_Compiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(CPP_Compiler main.cpp
        Scanner.cpp
        Scanner.h
        Token.h)
