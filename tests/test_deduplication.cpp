//
// Created by sage on 1/21/21.
//

#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
#include <lilelf.h>

SCENARIO("hardware should deduplicate same images", "[dedup]")
{
    GIVEN("two mappings of the same image")
    {
        auto victim1 = LilElf("./Spec1Victim");
        auto victim2 = LilElf("./Spec1Victim");
        auto sym_1 = victim1.get_sym("array1_size");
        auto sym_2 = victim2.get_sym("array1_size");
        auto m_sym1 = victim1.get_sym_value<void*>(sym_1);
        auto m_sym2 = victim2.get_sym_value<void*>(sym_2);
        get_thresh(0);

        REQUIRE(sym_1 != sym_2);
        REQUIRE(m_sym1 != m_sym2);

        WHEN("flush array1_size in victim1")
        {
            flush(m_sym1);
            THEN("array1_size should be loaded for victim2")
            {
                auto latency = load(m_sym2);
                std::cout << "latency " << latency << std::endl;
                REQUIRE(is_miss(latency));
                REQUIRE(!is_hit(latency));
            }
        }

        WHEN("load array1_size in victim1")
        {
            load(m_sym1);
            _mm_mfence();
            THEN("array1_size should be loaded for victim2")
            {
                auto latency = load(m_sym2);
                std::cout << "latency " << latency << std::endl;
                REQUIRE(!is_miss(latency));
                REQUIRE(is_hit(latency));
//                REQUIRE(is_hit(load(m_sym2)));
            }
        }

    }

}