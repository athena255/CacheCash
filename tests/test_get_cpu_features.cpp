#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cpu_info.h>

TEST_CASE ("get vendor string", "[cpuid]")
{
    REQUIRE( CPUInfo(0).cpui.ECX == 0x444D4163 ); // "DMAc"
}

TEST_CASE ("Fn0000_0001", "[cpuid]")
{
    auto c = CPUInfo(1);

    // Print the clflush size
    std::cout << RD_INT(c.cpui.EBX, 15, 8)*8 << std::endl;

    // Check the clfsh is supported
    REQUIRE( RD_INT(c.cpui.EDX, 19, 19) == 1 );
    REQUIRE( RD_BIT(c.cpui.EDX, 19) );

    // Check that TSC instructions supported
    REQUIRE( RD_BIT(c.cpui.EDX, 4) );

    // Check APIC exists and is enabled
    REQUIRE( RD_BIT(c.cpui.EDX, 9) );

    // Check for MSR - RDMSR/WRMSR
    REQUIRE( RD_BIT(c.cpui.EDX, 5) );

    // Check for MONITOR instruction
    REQUIRE( RD_BIT(c.cpui.ECX, 3) == false );

    // Check for HTT (hyperthreading)
    REQUIRE( RD_BIT(c.cpui.ECX, 28) );

    // Check for PCID
//    REQUIRE( RD_BIT(c.cpui.ECX, 24) );

    // Check if hypervisor is present
    REQUIRE( RD_BIT(c.cpui.ECX, 31) );

    REQUIRE( CPUInfo::detect_vm() );

}

TEST_CASE ("Fn0000_0007_x0", "[cpuid]")
{
    auto c = CPUInfo(7, 0);

    // Check if CLWB supported
    REQUIRE( RD_BIT(c.cpui.EBX, 24) == true );

    // Check if CLFLUSHOPT supported
    REQUIRE( RD_BIT(c.cpui.EBX, 23) == true );

    // PKU is not supported
    REQUIRE( RD_BIT(c.cpui.ECX, 3) == false );

    // Check for FSGSBASE read/write instruction support
    REQUIRE( RD_BIT(c.cpui.EBX, 0) );
}

TEST_CASE ("Fn8000_0001", "[cpuid-extended proc features]")
{
    auto c = CPUInfo(0x80000001);

    // Check RDTSCP support
    REQUIRE( RD_BIT(c.cpui.EDX, 27) == true );

    // Check for NX page protection
    REQUIRE( RD_BIT(c.cpui.EDX, 20) == true );

    // See if there is a SVM
    REQUIRE( RD_BIT(c.cpui.ECX, 2) == false );

}

TEST_CASE ("Fn8000_0007", "[cpuid-power management]")
{
    auto c = CPUInfo(0x80000007);

    // Check TSC Invariant - ensured to be invariant across all P and C states
    REQUIRE( RD_BIT(c.cpui.EDX, 8) == true );
}

TEST_CASE ("Fn8000_0008", "[cpuid-power extended features]")
{
    auto c = CPUInfo(0x80000008);

    // WBNOINVD support
    REQUIRE( RD_BIT(c.cpui.EBX, 9) == true );

    // CLZERO support
    REQUIRE( RD_BIT(c.cpui.EBX, 0) == true );

    // MCOMMIT support
    REQUIRE( RD_BIT(c.cpui.EBX, 8) == false );

    // RDPRU support
    REQUIRE( RD_BIT(c.cpui.EBX, 4) == false );

    // INVLPGB and TLBSYNC instruction support
    REQUIRE( RD_BIT(c.cpui.EBX, 3) == false );

}

TEST_CASE ("Fn8000_001D", "[cpuid-cache topology info]")
{
    for (size_t i = 0;; ++i)
    {
        auto c = CPUInfo(0x8000001D, i);
        auto type = RD_INT(c.cpui.EAX, 4, 0);
        auto level = RD_INT(c.cpui.EAX, 7, 5);
        auto is_fully_assoc = RD_BIT(c.cpui.EAX, 9);
        auto num_sharing = RD_INT(c.cpui.EAX, 25, 14);
        auto is_self_init = RD_BIT(c.cpui.EAX, 8);

        auto numways = RD_INT(c.cpui.EBX, 31, 22) + 1;
        auto physparts = RD_INT(c.cpui.EBX, 21, 12) + 1;
        auto linesize = RD_INT(c.cpui.EBX, 11, 0) + 1;

        auto numsets = RD_INT(c.cpui.ECX, 31, 0) + 1;

        auto inclusive = RD_BIT(c.cpui.EDX, 1);
        auto wbinvd = RD_BIT(c.cpui.EDX, 0);
        if (type == 0)
            break;
        std::cout
            << "level " << level
            << " type " << type
//            << " is_fully_assoc " << is_fully_assoc
            << " numsharing " << num_sharing
//            << " selfinit " << is_self_init
            << " numways " << numways
            << " physpartitions " << physparts
            << " linesize " << linesize
            << " numsets " << numsets
            << " inclusive " << inclusive
            << " wbinvd " << wbinvd
            << std::endl;
    }
}

// https://lwn.net/Articles/301888/
TEST_CASE ("Fn4000_0000", "[cpuid-hypervisor]")
{
    auto c = CPUInfo(0x40000000);

    // max input value --> VMware
    REQUIRE( c.cpui.EAX == 0x40000010 );
}

TEST_CASE("detect vmware port", "[vmware]")
{
    REQUIRE( CPUInfo::detect_vmware() );
}

TEST_CASE ("Fn4000_0010", "[cpuid-vmware-timing]")
{
    auto c = CPUInfo(0x40000010);
    std::cout
        << "TSC frequency " << c.cpui.EAX
        << " APIC timer " << c.cpui.EBX
        << " ECX " << c.cpui.ECX
        << " EDX " << c.cpui.EDX
        << std::endl;
}

TEST_CASE("smsw cr0", "[cpu]")
{
    uint64_t cr0 {};
    asm volatile( "smsw %0" : "=R" (cr0) );
    REQUIRE( 0 == RD_INT(cr0, 63, 32) );
    REQUIRE( 0 == RD_INT(cr0, 28, 19) );
    REQUIRE( 0 == RD_INT(cr0, 15, 6) );
    REQUIRE( 1 == RD_BIT(cr0, 4) );
    auto WP = RD_BIT(cr0, 16); // 0: supervisor software can write into read-only pages
    // 1: supervisor software cannot write to read-only pages
    auto MP =  RD_BIT(cr0, 1);
    auto TS = RD_BIT(cr0, 3); // Set to 1 on hardware task switch
    if (MP && TS)
        std::cout << "WAIT/FWAIT: device not available" << std::endl;
    else
        std::cout << "WAIT/FWAIT available" << std::endl;

    // 0: internal caches enabled
    // 1: no new data or instructions are brought into internal caches (misses do not affect internal cache)
    //      processor ignores page-level cache-control bits (PWT, PCD located in CR3)
    auto CD = RD_BIT(cr0, 30);

    // 0: page translation disabled
    // 1: page translation enabled (cannot be enabled unless CR0.PE=1)
    auto PG = RD_BIT(cr0, 31);

    // 1: descriptor tables used for segmentation is enabled
    auto PE = RD_BIT(cr0, 0);

    // Not Writethrough Disable Bit
    auto NW = RD_BIT(cr0, 29);

    if (CD)
        std::cout << "Cache Disabled" << std::endl;
    REQUIRE_FALSE( CD );

    if (!CD && !NW)
        std::cout << "Cache enabled with writeback policy" << std::endl;
    REQUIRE( (!CD && !NW) );

    std::cout << "cr0 " << cr0
              << " WP " << WP
              << " PG " << PG
              << " CD " << CD
              << " NW " << NW
              << " EM " << RD_BIT(cr0, 2)
              << " PE " << PE
              << std::endl;
}

TEST_CASE("lmsw cr0", "[cpu]")
{
    uint64_t cr0 {};
    asm volatile( "smsw %0" : "=R" (cr0) );

//    asm volatile( "mov %%cr0, %0" : "=R"(cr0) );

    // can only be used at CPL == 0
//    asm volatile( "lmsw %0" : : "m"(cr0) : );
}
