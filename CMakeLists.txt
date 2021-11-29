cmake_minimum_required(VERSION 3.13)
project(css432KevinGabriel)

set(CMAKE_CXX_STANDARD 14)

add_executable(css432KevinGabriel server/tftpserver.cpp client/tftpclient.cpp)
