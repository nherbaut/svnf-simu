//
// Created by nherbaut on 01/06/15.
//

#ifndef SVNF_SIMU_STREAMINGAPPLICATIONSERVER_H
#define SVNF_SIMU_STREAMINGAPPLICATIONSERVER_H

#include <ns3/log.h>
#include <ns3/packet-sink.h>
#include <ns3/socket.h>
#include <ns3/ptr.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>

using namespace ns3;

namespace labri {


    class StreamingApplicationServer : ns3::PacketSink {


    public:
        virtual void HandleRead(Ptr<Socket> socket) {
            NS_LOG_FUNCTION (this << socket);
            Ptr<Packet> packet;
            Address from;
            while ((packet = socket->RecvFrom(from))) {
                if (packet->GetSize() == 0) { //EOF
                    break;
                }

                if (InetSocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO ("At time " << Simulator::Now().GetSeconds()
                                 << "s packet sink received "
                                 << packet->GetSize() << " bytes from "
                                 << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                 << " port " << InetSocketAddress::ConvertFrom(from).GetPort()
                    );
                }
                else if (Inet6SocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO ("At time " << Simulator::Now().GetSeconds()
                                 << "s packet sink received "
                                 << packet->GetSize() << " bytes from "
                                 << Inet6SocketAddress::ConvertFrom(from).GetIpv6()
                                 << " port " << Inet6SocketAddress::ConvertFrom(from).GetPort()
                    );
                }

            }
        }

    };
}


#endif //SVNF_SIMU_STREAMINGAPPLICATIONSERVER_H
