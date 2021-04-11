#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <gdt.h>

namespace myos
{

    struct CPUState
    {
        // push by os
        common::uint32_t eax;
        common::uint32_t ebx;
        common::uint32_t exc;
        common::uint32_t edx;

        common::uint32_t esi; //stack index 
        common::uint32_t edi; //stack data index
        common::uint32_t ebp; // stack base pointer

/*         common::uint32_t gs;
        common::uint32_t fs;
        common::uint32_t es;
        common::uint32_t ds; */

        common::uint32_t error;

        // push by processor
        common::uint32_t eip; // instruction pointer
        common::uint32_t cs; // code segment
        common::uint32_t eflags; // flag
        common::uint32_t esp; // stack pointer
        common::uint32_t ss; // stack segment

    } __attribute__((packed));

    class Task
    {
    friend class TaskManager;
    private:
        common::uint8_t stack[4096]; // 4kib
        CPUState* cpustate;
    public:
        Task(GlobalDescriptorTable* gdt, void entrypoint());
        ~Task();
    };   

    class TaskManager
    {
    private:
        Task* tasks[256];
        int numTasks;
        int currentTask;
    public:
        TaskManager();
        ~TaskManager();
        bool AddTask(Task* task);
        CPUState* Schedule(CPUState* cpustate);
    };
}
#endif