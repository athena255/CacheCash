//
// Created by sage on 1/8/21.
//

#ifndef CACHECASH_CPU_INFO_H
#define CACHECASH_CPU_INFO_H

class CPUInfo {
public:

    static bool detect_vm();

    static bool detect_vmware();

//private:
    struct CPU_INFO{
        int EAX;
        int EBX;
        int ECX;
        int EDX;
    };

    explicit CPUInfo(unsigned int fn_code);
    CPUInfo(unsigned int fn_code, unsigned int ecx);

    CPU_INFO cpui;
};


#endif //CACHECASH_CPU_INFO_H
