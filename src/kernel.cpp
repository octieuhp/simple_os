#include <common/types.h>
#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/driver.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>
#include <memorymanagement.h>
#include <drivers/amd_am79c973.h>
#include <drivers/ata.h>
#include <systemcall.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>

// #define GRAPHICSMODE

using namespace myos;
using namespace myos::net;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;
using namespace myos::net;

char* httpResponse = "HTTP/1.1.200 0K\r\nServer:MyOS\r\nContent-Type: text/html\r\n\r\n<html><head><title>My Operating System</title></head><body><b>My Operating System</b> http://www.AlgorithBoy.me</body></html>\r\n";

void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x = 0, y = 0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch (str[i])
        {
            case '\n':
                y++;
                x = 0;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            y++;
            x = 0;
        }

        if(y >= 25)
        {
            for(y=0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
                x = 0;
                y = 0;
        }
    }
/*
    for(int i = 0; str[i] != '\0'; ++i)
        VideoMemory[i] = (VideoMemory[i] & 0xFF00) | str[i];
*/
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0x0F];
    foo[1] = hex[key&0x0F];
    printf(foo);
}

class PrintfKeyboardEventHandler : public KeyBoardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole: public MouseEventHandler
{
    int8_t x, y;
public:

    MouseToConsole()
    {
        
    }

    virtual void OnMouseActivate()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y=12;
        VideoMemory[80*12 + 40] = ((VideoMemory[80*12+40]&0xF000) >> 4)
                                | ((VideoMemory[80*12+40] & 0x0F00) << 4)
                                | (VideoMemory[80*12+40] & 0x00FF);
    }
    void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;

        VideoMemory[80*y + x] = ((VideoMemory[80*y+x]&0xF000) >> 4)
                                | ((VideoMemory[80*y+x] & 0x0F00) << 4)
                                | (VideoMemory[80*y+x] & 0x00FF);
        x += xoffset;
        if(x < 0) x = 0;
        if(x >= 80) x = 79;
        y += yoffset;
        if(y < 0) y = 0;
        if(y >= 25) y = 24;

        VideoMemory[80*y + x] = ((VideoMemory[80*y+x]&0xF000) >> 4)
                                | ((VideoMemory[80*y+x] & 0x0F00) << 4)
                                | (VideoMemory[80*y+x] & 0x00FF);
    }
};


class PrintfUDPdEventHandler : public UserDatagramProtocolHandler
{
public:
    void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size)
    {
        char* foo = " ";
        for(int i = 0; i < size; i++)
        {
            foo[0] = data[i];
            printf(foo);
        }
    }
};

class PrintfTCPdEventHandler : public TransmissionControlProtocolHandler
{
public:
    bool HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size)
    {
        char* foo = " ";
        for(int i = 0; i < size; i++)
        {
            foo[0] = data[i];
            printf(foo);
        }

        if(size > 9 
            && data[0] == 'G'
            && data[1] == 'E'
            && data[2] == 'T'
            && data[3] == ' '
            && data[4] == '/'
            && data[5] == ' '
            && data[6] == 'H'
            && data[7] == 'T'
            && data[8] == 'T'
            && data[9] == 'P'
        )
        {
            socket->Send((uint8_t*)httpResponse, 184);
            socket->Disconnect();
        }
            return false;

        return true;
    }
};

void sysprintf(char* str)
{
    asm("int $0x80" : : "a" (4), "b" (str));
}

void taskA()
{
    while(true)
        sysprintf("B");
}

void taskB()
{
    while(true)
        sysprintf("A");
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber)
{
    printf("Hello World ( from myos)");
    printf("\nHello world form line 2 ^_^.");  
    GlobalDescriptorTable gdt;

    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);

    printf("\nheap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8) & 0xFF);
    printfHex((heap ) & 0xFF);

    void* allocated; // = memoryManager.malloc(10);
    /* printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8) & 0xFF);
    printfHex(((size_t)allocated ) & 0xFF);
    printf("\n\n"); */

    //allocated = memoryManager.malloc(sizeof(amd_am79c973));
    //allocated = memoryManager.malloc(0);
/*     printf("\nallocated AMD: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8) & 0xFF);
    printfHex(((size_t)allocated ) & 0xFF);
    printf("\n\n"); */

    TaskManager taskManager;
/*     Task task1(&gdt, taskA);
    Task task2(&gdt, taskB);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2); */

    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(0x80, &interrupts);

    printf("\ninitializing Hardware, stage 1");

    #ifdef GRAPHICSMODE
        Desktop desktop(320, 200, 0xA8, 0x00, 0x00);
    #endif

    DriverManager drvManager;

    #ifndef GRAPHICSMODE
        PrintfKeyboardEventHandler kbHandler;
        KeyboardDriver keyboard(&interrupts, &kbHandler);
    #else
        KeyboardDriver keyboard(&interrupts, &desktop);
    #endif
    drvManager.AddDriver(&keyboard);

    #ifndef GRAPHICSMODE
        MouseToConsole mouseHandler;
        MouseDriver mouse(&interrupts, &mouseHandler);
    #else
        MouseDriver mouse(&interrupts, &desktop);
    #endif
    drvManager.AddDriver(&mouse);

    // origin PeripheralComponentInterConnectController PCIController;
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);

    printf("initializing Hardware, stage 2");
    drvManager.ActivateAll();

    printf("\ninitializing Hardware, stage 3");
    
    #ifdef GRAPHICSMODE
        VideoGraphicsArray vga;
        vga.SetMode(320, 200, 8);
        Window win1(&desktop, 10, 10, 20, 20, 0x00, 0xA8, 0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40, 15, 30, 30, 0x00, 0x00, 0x00);
        desktop.AddChild(&win2);
    #endif

    //interrupt 14
/*     AdvancedTechnologyAttachment ata0m(0x1F0, true);
    printf("ATA Primary Master: \n");
    ata0m.Itentify();

    AdvancedTechnologyAttachment ata0s(0x1F0, false);
    printf("ATA Primary Slave: \n");
    ata0s.Itentify();

    char* ataBuffer = "hello ata disk";
    ata0s.Write28(0, (uint8_t*)ataBuffer, 14);
    ata0s.Flush();

    ata0s.Read28(0, (uint8_t*)ataBuffer, 14);

    //interrpt 15
    AdvancedTechnologyAttachment ata1m(0x170, true);
    AdvancedTechnologyAttachment ata1s(0x170, false); */

    // 3rd: 01E8
    // 4th: 0x168

    amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);

    // ip address
    uint8_t ip1 = 10, ip2 = 0, ip3 = 2, ip4 = 15;
    uint32_t ip_be = ((uint32_t)ip4 << 24)
                        | ((uint32_t)ip3 << 16)
                        | ((uint32_t)ip2 << 8)
                        | ((uint32_t)ip1);

    eth0->SetIPAddress(ip_be);

    EtherFrameProvider etherFrame(eth0);

    AddressResolutionProtocol arp(&etherFrame);

    // ip getway
    uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
    uint32_t gip_be = ((uint32_t)gip4 << 24)
                        | ((uint32_t)gip3 << 16)
                        | ((uint32_t)gip2 << 8)
                        | ((uint32_t)gip1);

    // subnet mask
    uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;
    uint32_t subnet_be = ((uint32_t)subnet4 << 24)
                        | ((uint32_t)subnet3 << 16)
                        | ((uint32_t)subnet2 << 8)
                        | ((uint32_t)subnet1);
    InternetProtocolProvider ipv4(&etherFrame, &arp, gip_be, subnet_be);
    InternetControlMessageProtocol icmp(&ipv4);
    UserDatagramProtocolProvider udp(&ipv4);
    TransmissionControlProtocolProvider  tcp(&ipv4);

    // just for test
/*     amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]); // 0: keyboard - 1: mouse - 2 amd_am79c973 
    eth0->Send((uint8_t*)"Hello Network", 13); */
/*     amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);
    EtherFrameProvider etherframe(eth0);
    etherframe.Send(0xFFFFFFFFFFFF, 0x0608, (uint8_t*)"FOO", 3); */


    //Desktop desktop(320, 200, 0x00, 0x00, 0x00);
    //Desktop desktop(320, 200, 0xFF, 0xFF, 0xFF);
    interrupts.Activate();
    printf("\n\\n-\n\n\n\n\n");
    //arp.Resolve(gip_be);
    arp.BroadcastMACAddress(gip_be);
    //icmp.RequestEchoReplay(gip_be);

    //tcp.Connect(gip_be, 1234);
    PrintfTCPdEventHandler tcpHandler;
    TransmissionControlProtocolSocket* tcpSocket = tcp.Listen(1234);
    tcp.Bind(tcpSocket, &tcpHandler);
    //tcpSocket->Send((uint8_t*)"Hello TCP!", 10);

    //PrintfUDPdEventHandler udpHandler;
   /*  UserDatagramProtocolSocket* udpsocket = udp.Connect(gip_be, 1234);
    udp.Bind(udpsocket, &udpHandler);
    udpsocket->Send((uint8_t*)"Hello UDP!", 10); */

    //UserDatagramProtocolSocket* udpsocket = udp.Listen(1234);
    //udp.Bind(udpsocket, &udpHandler);
    
    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    };
}