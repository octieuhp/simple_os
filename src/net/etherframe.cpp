#include<net/etherframe.h>
using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::net;

void printf(char*);

EtherFrameHandler::EtherFrameHandler(EtherFrameProvider* backend, uint16_t etherType)
{
    printf("Constructor EtherFramHeader--");
    this->etherType_BE = ((etherType & 0x00FF) << 8)
                    | ((etherType & 0xFF00) >> 8);
    this->backend = backend;
    backend->handlers[etherType_BE] = this;
}

EtherFrameHandler::~EtherFrameHandler()
{
    backend->handlers[etherType_BE] = 0;
}

bool EtherFrameHandler::OnEtherFrameReceived(uint8_t* etherframePayLoad, uint32_t size)
{
    return false;
}

void EtherFrameHandler::Send(uint64_t dstMAC_BE, uint8_t* data, uint32_t size)
{
    printf("-Send EtherFH-");
    backend->Send(dstMAC_BE, etherType_BE, data, size);
}
        
EtherFrameProvider::EtherFrameProvider(amd_am79c973 * backend)
: RawDataHandler(backend)
{
    printf("-Cstror EFhandler---");
    for(uint32_t i = 0; i < 65535; i++)
        handlers[i] = 0;
}

EtherFrameProvider::~EtherFrameProvider()
{}

bool EtherFrameProvider::OnRawDataReceived(uint8_t* buffer, uint32_t size)
{
    printf("-onRaw EFPR vider--");
    EtherFrameHeader* frame = (EtherFrameHeader*)buffer;
    bool sendBack = false;

    if(frame->dstMAC_BE == 0xFFFFFFFFFFFF
    || frame->dstMAC_BE == backend->GetMACAddress())
    {
        if(handlers[frame->etherType_BE] != 0)
            sendBack = handlers[frame->etherType_BE]->OnEtherFrameReceived(
                buffer + sizeof(EtherFrameHeader), size - sizeof(EtherFrameHeader));
    }

    if(sendBack)
    {
        frame->dstMAC_BE = frame->srcMAC_BE;
        frame->srcMAC_BE = backend->GetMACAddress();
    }

    return sendBack;
}

void EtherFrameProvider::Send(uint64_t dstMAC_BE, uint16_t etherType_BE, uint8_t* buffer, uint32_t size)
{
    printf("-Send EF provide--");
    uint8_t* buffer2 = (uint8_t*)MemoryManager::activeMemoryManager->malloc(sizeof(EtherFrameHeader) + size);
    EtherFrameHeader* frame = (EtherFrameHeader*)buffer2;

    frame->dstMAC_BE = dstMAC_BE;
    frame->srcMAC_BE = backend->GetMACAddress();
    frame->etherType_BE = etherType_BE;
    printf((char*)buffer);
    uint8_t* src = buffer;
    uint8_t* dst = buffer2 + sizeof(EtherFrameHeader);
    for(uint32_t i = 0; i < size; i++)
        dst[i] = src[i];
    backend->Send(buffer2, size + sizeof(EtherFrameHeader));
}