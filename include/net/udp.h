#ifndef __MYOS__NET__UDP_H
#define __MYOS__NET__UDP_H

#include <common/types.h>
#include <net/ipv4.h>

namespace myos
{
    namespace net
    {
        struct UserDatagramProtocolHeader
        {
            common::uint16_t srcPort;
            common::uint16_t dstPort;
            common::uint16_t length;
            common::uint16_t checksum;
        }__attribute__((packed));

        class UserDatagramProtocolSocket;
        class UserDatagramProtocolProvider;

        class UserDatagramProtocolHandler
        {
        public:
            UserDatagramProtocolHandler();
            ~UserDatagramProtocolHandler();
            virtual void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);

        };

        class UserDatagramProtocolSocket
        {
        friend class UserDatagramProtocolProvider;
        protected:
            common::uint16_t remotePort;
            common::uint32_t remoteIP;
            common::uint16_t localPort;
            common::uint32_t localIP;
            UserDatagramProtocolProvider* backend;
            UserDatagramProtocolHandler* handler;
        public:
            UserDatagramProtocolSocket(UserDatagramProtocolProvider*);
            ~UserDatagramProtocolSocket();
            virtual void HandlUserDatagramProtocolMessage(common::uint8_t* data, common::uint16_t size);
            virtual void Send(common::uint8_t* data, common::uint16_t size);
            virtual void Disconnect();
        };

        class UserDatagramProtocolProvider : public InternetProtocolHandler
        {
        protected:
            UserDatagramProtocolSocket* sockets[65535];
            common::int16_t numSockets;
            common::uint16_t freePort;
        public:
            UserDatagramProtocolProvider(InternetProtocolProvider* backend);
            ~UserDatagramProtocolProvider();
            bool OnInternetProtocolReceived(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE,
                                            common::uint8_t* internetprotocolPayload, common::uint32_t size);
            
            virtual UserDatagramProtocolSocket* Connect(common::uint32_t ip, common::uint16_t port);
            virtual void Disconnect(UserDatagramProtocolSocket* socket);
            virtual void Send(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);
        };
    }
}

#endif