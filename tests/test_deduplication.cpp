//
// Created by sage on 1/21/21.
//

#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
#include <lilelf.h>
#include <wait.h>
#include <DuplexPipe.h>

// Using the SCENARIOS Catch2 doesn't work well probably because the setup might invalidate cache lines
TEST_CASE("hardware should deduplicate same images", "[.dedup]")
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

    // When array1_size is loaded in victim2 then flushed in victim1
    load(m_sym2);
    flush(m_sym1);
    {
        // Then array1_size should be flushed for victim2
        auto latency = load(m_sym2);
        D("latency %llu", latency);
        REQUIRE(is_miss(latency));
        REQUIRE(!is_hit(latency));
    }

    // Since array1_size was just loaded
    // Then array1_size should also be loaded for victim1
    {
        auto latency2 = load(m_sym1);
        D("latency2 %llu", latency2);
        REQUIRE(!is_miss(latency2));
        REQUIRE(is_hit(latency2));
    }
}

TEST_CASE("search for pattern", "[.patternsearch]")
{
    auto file_map = LilElf("./VictimStats");
    auto sym_array2 = file_map.get_sym("array2");
    auto m_static_array2 = file_map.get_sym_def<uint8_t*>(sym_array2);
    REQUIRE(m_static_array2[0] == 0xde);
    REQUIRE(m_static_array2[1] == 0xad);
    REQUIRE(m_static_array2[2] == 0xbe);
    REQUIRE(m_static_array2[3] == 0xef);
    uint8_t pattern[] = {0xad, 0xbe, 0xef, 69, 69, 69, 111};
    auto m_match = find_pattern(pattern, sizeof(pattern), m_static_array2, 8, 1);
    REQUIRE(m_match == &m_static_array2[1]);

    auto m_elf_magic = file_map.search_bytes("\x45\x4c", 2);
    REQUIRE(m_elf_magic != nullptr);
    REQUIRE( m_elf_magic[0] == 'E' );
    REQUIRE( m_elf_magic[1] == 'L' );

    auto m_other_match = file_map.search_bytes("\xDE\xAD\xBE\xEF\x45", 5, (m_static_array2 - &file_map[0]));
    REQUIRE(m_other_match != nullptr);
    REQUIRE(m_other_match == m_static_array2);
}

// Apparently, only const symbols work?
TEST_CASE("deduplication on VictimStats", "[DuplexPipe]")
{
    get_thresh(false);
    auto du_pipes = DuplexPipe("./VictimStats", NULL);
    auto victim = LilElf("./VictimStats");
    auto sym_array1_size = victim.get_sym("_ZL11array1_size");
    auto m_array1_size = victim.get_sym_value<void*>(sym_array1_size);

    auto m_array2 = victim.get_sym_value<uint8_t*>(victim.get_sym("_ZL6array2"));

    auto receive_and_print = [&](){
        du_pipes.receive_str();
        du_pipes.print_rx_buf();
    };

    auto send_x = [&](size_t x){
        du_pipes.fmt_send("%llu\n", x);
    };

    auto get_load_time = [&](){
        uint64_t load_time = 0, user_input;
        sscanf(du_pipes.rx_buf, "(%llu): %llu", &user_input, &load_time);
        return load_time;
    };

    receive_and_print();

    // The first one should be a miss
    send_x(0);
    send_x(1);
    send_x(3);
    send_x(3);
    send_x(3);
    send_x(3);

    du_pipes.wait_clear_rx();
    flush(m_array1_size);

    for (auto i = 0; i < 256; ++i)
    {
        flush(&m_array2[i*512]);
    }

    send_x(3);
    receive_and_print();
    REQUIRE(is_miss(get_load_time()));

    send_x(3);
    send_x(3);
    send_x(3);
    send_x(4);

    // Send EOF to end VictimStats
    du_pipes.send("\0");
    // Wait for VictimStats to end
    wait(nullptr);

    receive_and_print();
}

TEST_CASE("Implementation of Proj2_tx", "[.dedup]")
{
    static constexpr size_t num_signals = 200;
    auto proj2 = LilElf("./Proj2_rx");
    auto m_rdy = proj2.get_sym_value<void*>(proj2.get_sym("_ZL3rdy"));
    auto m_zero = proj2.get_sym_value<void*>(proj2.get_sym("_ZL4zero"));
    auto m_one = proj2.get_sym_value<void*>(proj2.get_sym("_ZL3one"));

    auto send_rdy = [&](){ for (auto i = 0; i < num_signals; ++i) {flush(m_rdy);load(m_zero);load(m_one);}};
    auto send_one = [&](){for (auto i = 0; i < num_signals; ++i) {load(m_rdy); flush(m_one); load(m_zero);}};
    auto send_zero = [&](){for (auto i = 0; i < num_signals; ++i) {load(m_rdy); flush(m_zero);load(m_one);}};

    REQUIRE(m_rdy != m_zero);
    REQUIRE(m_zero != m_one);
    REQUIRE(m_one != m_rdy);

    send_rdy();
    send_one();
    send_rdy();
    send_zero();

}