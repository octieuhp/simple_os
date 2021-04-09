#include <hardwarecommunication/pci.h>
using namespace myos::common;
using namespace myos::hardwarecommunication;
using namespace myos::drivers;

void printf(char* str);
void printfHex(uint8_t);

PeripheralComponentInterConnectDeviceDescriptor::PeripheralComponentInterConnectDeviceDescriptor()
{}

PeripheralComponentInterConnectDeviceDescriptor::~PeripheralComponentInterConnectDeviceDescriptor()
{}

PeripheralComponentInterConnectController::PeripheralComponentInterConnectController()
: dataPort(0xCFC),
  commandport(0xCF8)
{

}

PeripheralComponentInterConnectController::~PeripheralComponentInterConnectController()
{

}

uint32_t PeripheralComponentInterConnectController::Read(int16_t bus, int16_t device, int16_t function, uint32_t registeroffset)
{
    uint32_t id = 
        0x1 << 31
        | ((bus &0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registeroffset & 0xFC);
    commandport.Write(id);
    uint32_t result = dataPort.Read();
    return result >> (8*(registeroffset % 4));
}
void PeripheralComponentInterConnectController::Write(uint16_t bus, uint16_t device, uint16_t function, int32_t registeroffset, int32_t value)
{
    uint32_t id = 
        0x1 << 31
        | ((bus &0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registeroffset & 0xFc);
    commandport.Write(id);
    dataPort.Write(value);
}

bool PeripheralComponentInterConnectController::DeviceHasFunction(uint16_t bus, uint16_t device)
{
    return Read(bus, device, 0, 0x0E) & (1<<7);
}

void PeripheralComponentInterConnectController::SelectDrivers(DriverManager* driverManager)
{
    for(int bus = 0; bus < 8; bus++)
    {
        for(int device = 0; device < 32; device++)
        {
            int numFunctions = DeviceHasFunction(bus, device) ? 8 : 1;
            for(int function = 0; function < numFunctions; function++)
            {
                PeripheralComponentInterConnectDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);
                
                if(dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                    continue;

                printf("PCI BUS ");
                printfHex(bus & 0xFF);

                printf(", DEVICE ");
                printfHex(device & 0xFF);

                printf(", FUNCTION ");
                printfHex(function & 0xFF);

                printf(" = VENDOR ");
                printfHex((dev.vendor_id & 0xFF00) >> 8);
                printfHex(dev.vendor_id & 0xFF);
                printf(", DEVICE ");
                printfHex((dev.device_id& 0xFF00) >> 8);
                printfHex(dev.device_id & 0xFF);
                printf("\n");
            }
        }
    }
}

PeripheralComponentInterConnectDeviceDescriptor PeripheralComponentInterConnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    PeripheralComponentInterConnectDeviceDescriptor result;

    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_id = Read(bus, device, function, 0x00);
    result.device_id = Read(bus, device, function, 0x02);
    
    result.class_id = Read(bus, device, function, 0x0b);
    result.subclass_id = Read(bus, device, function, 0x0a);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision = Read(bus, device, function, 0x08);
    result.interrupt = Read(bus, device, function, 0x3c);

    return result;
}









