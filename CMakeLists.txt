cmake_minimum_required(VERSION 3.19)
project(CacheCash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

include_directories(src)

add_executable(CacheCash
        src/config.h
        mains/main.cpp
        src/impl/cpu_info.cpp
        src/cpu_info.h src/stdafx.h src/impl/cacheutils.cpp src/cacheutils.h src/utils.h src/impl/Spectrev1.cpp src/Spectrev1.h)

find_package(Catch2 REQUIRED)

add_executable(CPUFeatures_tests
        src/impl/cpu_info.cpp
        tests/test_get_cpu_features.cpp
        tests/test_main.cpp src/utils.h)

add_executable(Cache_tests
        src/impl/cpu_info.cpp
        tests/test_main.cpp
        src/impl/cacheutils.cpp
        tests/test_cache.cpp)

add_executable(Spectrev1_orig_tests
        src/impl/cpu_info.cpp
        tests/test_main.cpp
        src/impl/cacheutils.cpp
        tests/test_specv1_orig.cpp)

add_executable(Spectrev1_tests
        src/impl/cpu_info.cpp
        tests/test_main.cpp
        src/impl/cacheutils.cpp
        src/impl/Spectrev1.cpp
        tests/test_specv1.cpp)

add_executable(Spec1Victim
        mains/spectre1victim.cpp)

add_executable(Spec1Attack
        mains/spectre1attack.cpp
        src/impl/cpu_info.cpp
        src/impl/cacheutils.cpp
        src/impl/Spectrev1.cpp
        src/impl/lilelf.cpp)

target_link_libraries(CPUFeatures_tests PRIVATE Catch2::Catch2)
target_link_libraries(Cache_tests PRIVATE Catch2::Catch2)
target_link_libraries(Spectrev1_orig_tests PRIVATE Catch2::Catch2)
target_link_libraries(Spectrev1_tests PRIVATE Catch2::Catch2)

#target_link_libraries(Spec1Attack PUBLIC libelf.a)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ofast")
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Ofast")
