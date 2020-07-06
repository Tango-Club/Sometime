cmake_minimum_required (VERSION 2.6)
SET (CMAKE_CXX_FLAGS                "-Wall -std=c++11 -O3")
project (somServ)


include_directories(Headers)
find_package(RapidJSON)
add_executable (somServ Sources/server_main.cpp Sources/http_server.cpp Sources/mongoose.c)