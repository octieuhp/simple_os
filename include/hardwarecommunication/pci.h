#ifndef __MYOS__HARDWARECOMMUNICATION_PCI_H
#define __MYOS__HARDWARECOMMUNICATION_PCI_H

#include <hardwarecommunication/port.h>
#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>

namespace myos
{
    namespace hardwarecommunication
    {
        enum BaseAddressRegisterType
        {
            MemoryMapping = 0,
            InputOutput = 1
        };

        class BaseAddressRegister
        {
        public:
            bool perfectchable;
            myos::common::uint8_t* address;
            myos::common::uint32_t* size;
            BaseAddressRegisterType type;
        };

        class PeripheralComponentInterConnectDeviceDescriptor
        {
        public:
            common::uint32_t portBase;
            common::uint32_t interrupt;

            common::uint16_t bus;
            common::uint16_t device;
            common::uint16_t function;

            common::uint16_t vendor_id;
            common::uint16_t device_id;

            common::uint8_t class_id;
            common::uint8_t subclass_id;
            common::uint8_t interface_id;

            common::uint8_t revision;

            PeripheralComponentInterConnectDeviceDescriptor();
            ~PeripheralComponentInterConnectDeviceDescriptor();
        };

        class PeripheralComponentInterConnectController
        {
            Port32Bit dataPort;
            Port32Bit commandport;
        public: 
            PeripheralComponentInterConnectController();
            ~PeripheralComponentInterConnectController();

            common::uint32_t Read(common::int16_t bus, common::int16_t device, common::int16_t function, common::uint32_t registeroffset);
            void Write(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::int32_t registeroffset, common::int32_t value);
            bool DeviceHasFunction(common::uint16_t bus, common::uint16_t device);

            void SelectDrivers(drivers::DriverManager* driverManager, myos::hardwarecommunication::InterruptManager* interrupts);
            myos::drivers::Driver* GetDriver(PeripheralComponentInterConnectDeviceDescriptor dev, myos::hardwarecommunication::InterruptManager* interupts);
            PeripheralComponentInterConnectDeviceDescriptor GetDeviceDescriptor(common::uint16_t bus, common::uint16_t device, common::uint16_t function);
            BaseAddressRegister GetBaseAddressRegister(myos::common::uint16_t bus, myos::common::uint16_t device, myos::common::uint16_t function, myos::common::uint16_t bar);
        };
    }
}

#endif
