cmake_minimum_required(VERSION 3.28)

add_executable(exptable_dump exptable_dump.cpp)
target_compile_features(exptable_dump PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(exptable_dump PRIVATE /W3 /permissive- /utf-8)
endif()
