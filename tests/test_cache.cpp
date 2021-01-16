//
// Created by sage on 1/8/21.
//
#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
// TODO: Call get_thresh first since it tends to be weird on the first call
#include <x86intrin.h>
#include <immintrin.h>
TEST_CASE ("read fs and gs", "[.read seg desc]")
{
    // read fs base
    REQUIRE( _readfsbase_u64() != 0 );
    std::cout << std::hex << _readfsbase_u64() << std::endl;
    std::cout << std::hex << _readgsbase_u64() << std::endl;
    _writegsbase_u64(0xdeadbeef);
//    _writefsbase_u64(gs_base);
    std::cout << std::hex << _readgsbase_u64() << std::endl;
}

TEST_CASE("rdtscp vs rdpid", "[.tsc]")
{
    unsigned int tsc_aux{};
    _rdtscp(&tsc_aux);
    REQUIRE(tsc_aux == _rdpid_u32());
}

TEST_CASE("thresh", "[cache]")
{
    REQUIRE( get_thresh(true) < get_thresh(false) );
}


TEST_CASE("cache for library fn", "[cache]")
{
    void *inst_mem = MEM_ADD(fopen, 0);
    auto t1 = load(inst_mem);
    auto t2 = load(inst_mem);
    auto t3 = flush(inst_mem);
    auto t4 = load(inst_mem);
    std::cout
        << "==> lib:"
        << " load: " << t1
        << " load (hit): " << t2
        << " flush: " << t3
        << " load (miss): " << t4
        << std::endl;
    REQUIRE( is_hit(t2) );
    REQUIRE( is_miss(t4) );
}

TEST_CASE("cache for local fn", "[cache]")
{
    void *inst_mem = MEM_ADD(is_hit, 0);
    auto t1 = load(inst_mem);
    auto t2 = load(inst_mem);
    auto t3 = flush(inst_mem);
    auto t4 = load(inst_mem);
    std::cout
            << "==> local:"
            << " load: " << t1
            << " load (hit): " << t2
            << " flush: " << t3
            << " load (miss): " << t4
            << std::endl;
    REQUIRE( is_hit(t2) );
    REQUIRE( is_miss(t4) );
}

TEST_CASE("cache for heap data", "[cache]")
{
    int *mem = (int*) malloc(0x1000);
    void *data_mem = MEM_ADD(mem, 696);
    auto t1 = load(data_mem);
    auto t2 = load(data_mem);
    auto t3 = flush(data_mem);
    auto t4 = load(data_mem);
    std::cout
            << "==> heap:"
            << " load: " << t1
            << " load (hit): " << t2
            << " flush: " << t3
            << " load (miss): " << t4
            << std::endl;
    REQUIRE( is_hit(t2) );
    REQUIRE( is_miss(t4) );
    free(mem);
}

TEST_CASE("cacheline size", "[cache]")
{
    REQUIRE( 64 == get_cachelinesize() );
}

TEST_CASE("L1 D$ latency", "[cache]")
{
    // bring into L2 and I$ and TLB
    printf("");
    // miss in D$, hit in L2
    auto m_printf = MEM_ADD(printf, 0);
    auto hit_L2 = load(m_printf );
    auto hit_d = load(m_printf);
    // flush to cause miss in D
    flush(m_printf);
    auto miss_L2 = load (m_printf);

    auto d_latency = hit_L2 - hit_d;
    std::cout << "miss D$ " << hit_L2
        << "\nhit D$ " << hit_d
        << "\nmiss L2 " << miss_L2
        << std::endl;
    std::cout << "D$ latency: " << d_latency << std::endl;
}

TEST_CASE("TLB miss", "[.cache]")
{
    auto base = load(MEM_ADD(flush, 0));

    auto m_scanf = MEM_ADD(scanf, 0);
    auto tlb_miss_d_miss = load(m_scanf);
    auto tlb_hit_d_hit = load(m_scanf);
    flush(m_scanf);
    auto tlb_hit_d_miss = load(m_scanf);

    std::cout
        << "base: " << base
        << "\ntlb_miss_d$_miss: " << tlb_miss_d_miss
        << "\ntlb_hit_d$_hit: " << tlb_hit_d_hit
        << "\ntlb_hit_d$_miss: " << tlb_hit_d_miss
        << std::endl;

    auto main_mem_latency = tlb_hit_d_miss - tlb_hit_d_hit;
    auto tlb_miss_lat = tlb_miss_d_miss - tlb_hit_d_miss;
    std::cout
        << "main mem: " << main_mem_latency
        << "\ntlb lat: " << tlb_miss_lat
        << std::endl;
}

TEST_CASE("invd", "[.cache]")
{
    // may or may not include lower level caches associated with another processor that shares
    // any level of this processor's cache hierarchy
    // should work as long as the thing is not in L3
//    asm volatile("wbinvd");
}

TEST_CASE("prefetch", "[cache]")
{
    // PREFETCH loads the entire 64-byte aligned memory sequence into L1 data cache
    // sets cache-line state to Exclusive
    auto m_scanf = MEM_ADD(scanf, 0);
    flush(m_scanf);
    asm volatile("PREFETCHT0 (%0); mfence" ::"R"(m_scanf));
    auto t1 = load(m_scanf);
    auto t2 = load(m_scanf);
    std::cout << "prefetched: " << t1 << " " << t2 << std::endl;

}


TEST_CASE("test bad load", "[.cache]")
{
    auto legal = load(MEM_ADD(fprintf, 0));
    auto illegal = load(MEM_ADD(23, 0));
    auto legal_hit = load(MEM_ADD(fprintf, 0));
    std::cout
        << "legal (miss): " << legal
        << " | illegal (miss): " << illegal
        << " | legal (hit): " << legal_hit
        << std::endl;
}

TEST_CASE("test const write", "[.cache]")
{
    volatile const int x = 53; // Not using volatile will make x remain 53 in the printf
//    const int x = 53;
    int y = 64;
    volatile void *m_x = reinterpret_cast<volatile void*>(const_cast<int *>(&x));
    auto t1 = load(m_x);
    auto t2 = load(MEM_ADD(&y, 0));
    int* p_x = const_cast<int *>(&x);
    *p_x = 32;
    printf("&x: %X | p_x: %X | x: %d | *p_x: %d\n", &x, p_x, x, *p_x);
    std::cout
            << "const: " << t1
            << " | nonconst: " << t2
            << std::endl;

}
