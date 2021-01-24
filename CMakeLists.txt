cmake_minimum_required(VERSION 3.19)
project(CacheCash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

include_directories(src)

add_executable(Spec1Victim
        mains/spectre1victim.cpp)

add_executable(Spec1Attack
        src/impl/cpu_info.cpp
        src/impl/cacheutils.cpp
        src/impl/Spectrev1.cpp
        src/impl/lilelf.cpp
        mains/spectre1attack.cpp)

add_executable(VictimStats
        src/impl/cpu_info.cpp
        src/impl/cacheutils.cpp
        mains/victimstats.cpp)

find_package(Catch2 REQUIRED)

add_executable(CPUFeatures_tests
        src/impl/cpu_info.cpp
        tests/test_main.cpp
        tests/test_get_cpu_features.cpp)

add_executable(Cache_tests
        src/impl/cpu_info.cpp
        src/impl/cacheutils.cpp
        tests/test_main.cpp
        tests/test_cache.cpp)

add_executable(Spectrev1_orig_tests
        src/impl/cpu_info.cpp
        src/impl/cacheutils.cpp
        tests/test_main.cpp
        tests/test_specv1_orig.cpp)

add_executable(Spectrev1_tests
        src/impl/cpu_info.cpp
        src/impl/cacheutils.cpp
        src/impl/Spectrev1.cpp
        tests/test_main.cpp
        tests/test_specv1.cpp)

add_executable(Pipes_tests
        tests/test_main.cpp
        tests/test_pipes.cpp)

add_executable(Dedup_tests
        src/impl/lilelf.cpp
        src/impl/cpu_info.cpp
        src/impl/cacheutils.cpp
        tests/test_main.cpp
        tests/test_deduplication.cpp)

target_link_libraries(CPUFeatures_tests PRIVATE Catch2::Catch2)
target_link_libraries(Cache_tests PRIVATE Catch2::Catch2)
target_link_libraries(Spectrev1_orig_tests PRIVATE Catch2::Catch2)
target_link_libraries(Spectrev1_tests PRIVATE Catch2::Catch2)
target_link_libraries(Pipes_tests PRIVATE Catch2::Catch2)
target_link_libraries(Dedup_tests PRIVATE Catch2::Catch2)

#target_link_libraries(Spec1Attack PUBLIC libelf.a)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ofast")
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Ofast")
