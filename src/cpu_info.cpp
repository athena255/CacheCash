//
// Created by sage on 1/8/21.
//

#include <stdafx.h>
#include <cpu_info.h>

CPUInfo::CPUInfo(unsigned int const fn_code) : cpui{}
{
    __cpuid(fn_code, cpui.EAX, cpui.EBX, cpui.ECX, cpui.EDX);
}

CPUInfo::CPUInfo(unsigned int const fn_code, unsigned int ecx) : cpui{}
{
    __cpuid_count(fn_code, ecx, cpui.EAX, cpui.EBX, cpui.ECX, cpui.EDX);
}

bool CPUInfo::detect_vm()
{
    auto c = CPUInfo(1);
    return RD_BIT(c.cpui.ECX, 31);
}

bool CPUInfo::detect_vmware()
{
    auto c = CPUInfo(0x40000000);

    // get hypervisor vendor signature
    char vendor[0x20]{};
    *reinterpret_cast<int*>(vendor) = c.cpui.EBX;
    *reinterpret_cast<int*>(vendor+4) = c.cpui.ECX;
    *reinterpret_cast<int*>(vendor+8) = c.cpui.EDX;
    bool sig_match = !strncmp(vendor, "VMwareVMware", 12);

    // read from hypervisor port
    int eax, ebx, ecx, edx;
    asm volatile (
    "inl (%%dx)"
    : "=a"(eax), "=c"(ecx), "=d"(edx), "=b"(ebx)
    : "0"(0x564D5868), "1"(10), "2"(0x5658), "3"(0)
    : "memory"
    );
    bool magic_match = ebx == 0x564D5868;

    return sig_match && magic_match;
}
