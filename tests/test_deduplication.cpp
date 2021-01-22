//
// Created by sage on 1/21/21.
//

#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
#include <lilelf.h>

// Using the SCENARIOS Catch2 doesn't work well probably because the setup might invalidate cache lines
TEST_CASE("hardware should deduplicate same images", "[dedup]")
{
    // Given: two mappings of the same image
    get_thresh(false);
    auto victim1 = LilElf("./Spec1Victim");
    auto victim2 = LilElf("./Spec1Victim");
    auto sym_1 = victim1.get_sym("array1_size");
    auto sym_2 = victim2.get_sym("array1_size");
    auto m_sym1 = victim1.get_sym_value<void*>(sym_1);
    auto m_sym2 = victim2.get_sym_value<void*>(sym_2);

    REQUIRE(sym_1 != sym_2);
    REQUIRE(m_sym1 != m_sym2);

    // When array1_size is flushed from victim1
    flush(m_sym1);
    {
        // Then array1_size should be loaded for victim2
        auto latency = load(m_sym2);
        std::cout << "latency " << latency << std::endl;
        REQUIRE(is_miss(latency));
        REQUIRE(!is_hit(latency));
    }

    // Since array1_size was laoded by victim2
    // Then array1_size should also be loaded for victim1
    {
        auto latency2 = load(m_sym1);
        std::cout << "latency " << latency2 << std::endl;
        REQUIRE(!is_miss(latency2));
        REQUIRE(is_hit(latency2));
    }
}
