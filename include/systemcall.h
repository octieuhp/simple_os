#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

namespace myos
{
    class SyscallHandler : public hardwarecommunication::InterruptHandler
    {
    public:
        SyscallHandler(common::uint8_t interruptNumber, hardwarecommunication::InterruptManager* interruptManager);
        ~SyscallHandler();

        virtual common::uint32_t HandleInterrupt(common::uint32_t esp);
    };
}

#endif