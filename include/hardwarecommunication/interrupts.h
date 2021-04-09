
#ifndef __HARDWARECOMMUNINCATION__INTERRRUPTS_H
#define __HARDWARECOMMUNINCATION__INTERRRUPTS_H

#include "common/types.h"
#include "hardwarecommunication/port.h"
#include "gdt.h"

namespace myos
{
    namespace hardwarecommunication
    {
        class InterruptManager;

        class InterruptHandler
        {
        protected:
            myos::common::uint8_t interruptNumber;
            InterruptManager* interruptManager;

            InterruptHandler(myos::common::uint8_t interruptNumber, InterruptManager* interruptManager);
            ~InterruptHandler();
        public:
            virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
        };

        class InterruptManager
        {
        friend class InterruptHandler;
        protected:

            static InterruptManager* ActiveInterruptManager;
            InterruptHandler* handlers[256];
            struct GateDescriptor
            {
                myos::common::uint16_t handleAddressLowBits;
                myos::common::uint16_t gdt_codeSegmentSelector;
                myos::common::uint8_t reserved;
                myos::common::uint8_t access;
                myos::common::uint16_t handleAddressHighBits;
            } __attribute__((packed));

            static GateDescriptor interruptDescriptorTable[256];

            struct interruptdescriptorTablePointer
            {
                myos::common::uint16_t size;
                myos::common::uint32_t base;

            } __attribute__((packed));

            static void SetInterruptDescriptorTableEntry(
                myos::common::uint8_t interruptNumber,
                myos::common::uint16_t codeSegmentSelectorOffset,
                void (*handler)(),
                myos::common::uint8_t DescriptorPrivilegeLevel,
                myos::common::uint8_t DescriptorType
            );

            Port8BitSlow picMasterCommand;
            Port8BitSlow picMasterData;
            Port8BitSlow picSlaveCommand;
            Port8BitSlow picSlaveData;

        public:

            InterruptManager(GlobalDescriptorTable* gdt);
            ~InterruptManager();

            void Activate();
            void Deactivate();

            static myos::common::uint32_t handleInterrupt(myos::common::uint8_t interruptNumber, myos::common::uint32_t esp);
            myos::common::uint32_t DoHandleInterrupt(myos::common::uint8_t interruptNumber, myos::common::uint32_t esp);

            static void IgnoreInterruptRequest();
            static void HandleInterruptRequest0x00();
            static void HandleInterruptRequest0x01();
            static void HandleInterruptRequest0x0C();
        };
    }
}

#endif