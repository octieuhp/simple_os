#ifndef __MYOS_DRIVERS_KEYBOARD_H
#define __MYOS_DRIVERS_KEYBOARD_H

#include "common/types.h"
#include "hardwarecommunication/interrupts.h"
#include "hardwarecommunication/port.h"
#include "drivers/driver.h"

namespace myos
{
    namespace drivers
    {
        class KeyBoardEventHandler
        {
        public:
            KeyBoardEventHandler();

            virtual void OnKeyDown(char);
            virtual void OnkeyUp(char);
        };

        class KeyboardDriver : public myos::hardwarecommunication::InterruptHandler, public Driver
        {
            myos::hardwarecommunication::Port8Bit dataport;
            myos::hardwarecommunication::Port8Bit commandport;

            KeyBoardEventHandler* handler;
        public:
            KeyboardDriver(myos::hardwarecommunication::InterruptManager* manager, KeyBoardEventHandler* handler);
            ~KeyboardDriver();
            virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
            virtual void Activate();
        };
    }
}

#endif