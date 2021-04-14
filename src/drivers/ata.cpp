#include <drivers/ata.h>
using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;
using namespace myos::drivers;

void printf(char*);

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(uint16_t portBase, bool master)
:   dataPort(portBase),
    errorPort(portBase + 0x1),
    sectorCountPort(portBase + 0x2),
    lbaLowport(portBase + 0x3),
    lbaMidPort(portBase + 0x4),
    lbaHiPort(portBase + 0x5),
    devicePort(portBase + 0x6),
    commandPort(portBase + 0x7),
    controlPort(portBase + 0x206)
{
    bytesPerSector = 512;
    this->master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment()
{

}

void AdvancedTechnologyAttachment::Itentify()
{
    devicePort.Write(master ? 0xA0 : 0xB0);
    controlPort.Write(0);

    devicePort.Write(0xA0);
    uint8_t status = commandPort.Read();
    if(status == 0xFF)
        return;
    
    devicePort.Write(master ? 0xA0 : 0xB0);
    sectorCountPort.Write(0);
    lbaLowport.Write(0);
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
    commandPort.Write(0xEC);

    status = commandPort.Read();
    if(status == 0x00)
        return;
    
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    {
        status = commandPort.Read();
    }

    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }

    for(uint16_t i = 0; i < 256; i++)
    {
        uint16_t data = dataPort.Read();
        char* foo = "  \0";
        foo[1] = (data >> 8) & 0x00FF;
        foo[0] = data & 0x00FF;
        printf(foo);
    }
    printf("\n");
}
void AdvancedTechnologyAttachment::Read28(uint32_t sector, uint8_t* data, int count)
{
    if((sector & 0x0FFFFFFF) || (count > bytesPerSector))
        return;

    devicePort.Write((master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    errorPort.Write(0);
    sectorCountPort.Write(1);

    lbaLowport.Write(sector & 0x000000FF);
    lbaMidPort.Write((sector & 0x0000FF00) >> 8);
    lbaHiPort.Write((sector & 0x00FF0000) >> 16);
    commandPort.Write(0x20);

    uint8_t status = commandPort.Read();
    
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    {
        status = commandPort.Read();
    }

    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }

    printf("Reading from ATA ");

    for(uint16_t i = 0; i < count; i += 2)
    {
        uint16_t rdata = dataPort.Read();

        char* foo = "  \0";
        //foo[1] = (rdata >> 8) & 0x00FF;
        foo[0] = rdata & 0x00FF;
        printf(foo);

        data[i] = rdata & 0x00FF;
        if(i+1 < count)
            foo[1] = (rdata >> 8) & 0x00FF;
        else
        {
            foo[1] = '\0';
        }
    }

    for(uint16_t i = count+(count%2); i < bytesPerSector; i += 2)
    {
        dataPort.Read();
    }
}

void AdvancedTechnologyAttachment::Write28(uint32_t sector, uint8_t* data, int count)
{
    if((sector & 0x0FFFFFFF) || (count > bytesPerSector))
        return;

    devicePort.Write((master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    errorPort.Write(0);
    sectorCountPort.Write(1);

    lbaLowport.Write(sector & 0x000000FF);
    lbaMidPort.Write((sector & 0x0000FF00) >> 8);
    lbaHiPort.Write((sector & 0x00FF0000) >> 16);
    commandPort.Write(0x30);

    printf("Writing to ATA ");

    for(uint16_t i = 0; i < count; i += 2)
    {
        uint16_t wdata = data[i];
        if(i+1 < count)
            wdata |= ((uint16_t)data[i+1]) << 8;
        dataPort.Write(wdata);
      
        char* foo = "  \0";
        foo[0] = (wdata >> 8) & 0x00FF;
        foo[1] = wdata & 0x00FF;
        printf(foo);
    }

    for(uint16_t i = count+(count%2); i < bytesPerSector; i += 2)
    {
        dataPort.Write(0x0000);
    }
}

void AdvancedTechnologyAttachment::Flush()
{
    devicePort.Write(master ? 0xE0 : 0xF0);
    commandPort.Write(0xE7);

    uint8_t status = commandPort.Read();
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    {
        status = commandPort.Read();
    }

    if(status & 0x01)
    {
        printf("ERROR");
        return;
    } 
}