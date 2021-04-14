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

// #define GRAPHICSMODE

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

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

void taskA()
{
    while(true)
        printf("B");
}

void taskB()
{
    while(true)
        printf("A");
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

    printf("\ninitializing Hardware, stage 1\n");

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

    PeripheralComponentInterConnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);

    printf("\ninitializing Hardware, stage 2");
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
    AdvancedTechnologyAttachment ata0m(0x1F0, true);
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
    AdvancedTechnologyAttachment ata1s(0x170, false);

    // 3rd: 01E8
    // 4th: 0x168

    // just for test
/*     amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]); // 0: keyboard - 1: mouse - 2 amd_am79c973 
    eth0->Send((uint8_t*)"Hello Network", 13); */


    //Desktop desktop(320, 200, 0x00, 0x00, 0x00);
    //Desktop desktop(320, 200, 0xFF, 0xFF, 0xFF);
    interrupts.Activate();
    
    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    };
}