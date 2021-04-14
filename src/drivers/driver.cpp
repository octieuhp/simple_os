#include <drivers/driver.h>
using namespace myos::drivers;


Driver::Driver()
{

}

Driver::~Driver()
{

}

void printf(char*);

void Driver::Activate()
{
    printf("ACTIVATE DRIVER------------------");
    

}
int Driver::Reset()
{

}
void Driver::Deactivate()
{

}

DriverManager::DriverManager()
{
    numDrivers = 0;
}

void DriverManager::AddDriver(Driver* drv)
{
    drivers[numDrivers] = drv;
    numDrivers++;
}

void DriverManager::ActivateAll()
{
    for (int i = 0; i < numDrivers; i++)
        drivers[i]->Activate();
}