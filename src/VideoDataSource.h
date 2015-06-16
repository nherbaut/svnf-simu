//
// Created by nherbaut on 09/06/15.
//

#ifndef SVNF_SIMU_VIDEODATASOURCE_H
#define SVNF_SIMU_VIDEODATASOURCE_H

#include <ns3/application.h>
#include <ns3/ptr.h>
#include <ns3/socket.h>
#include <ns3/node.h>
#include <ns3/data-rate.h>
#include <ns3/simulator.h>
#include <ns3/output-stream-wrapper.h>
#include <ns3/log.h>
#include <ns3/pcap-file-wrapper.h>
#include <ns3/ipv4-address.h>
#include <string>
#include <ns3/traced-value.h>


using namespace ns3;
class ClientDataFromDataSource;
namespace  labri {



    class VideoDataSource : public Application {

    public:

        static TypeId GetTypeId(void);

        void Setup(InetSocketAddress control, DataRate channelDataRate);

        virtual void StartApplication(void);

        virtual void StopApplication(void);

        VideoDataSource();

        virtual ~VideoDataSource();


    private:
        InetSocketAddress m_controlAddr;

        Ptr<Socket> m_controlSocket;

        TracedValue<uint32_t> m_channelRate;
        uint32_t remainingRate;
        static const uint32_t writeSize = 1040;
        char buff[writeSize];

        std::map<Ptr<Socket>, std::string> m_socketIpMapping;

        bool HandleConnectionRequest(Ptr<Socket>, const Address &);

        void HandlePeerError(Ptr<Socket>);

        void HandlePeerClose(Ptr<Socket>);

        void HandleSignalingAccept(Ptr<Socket>, const Address &);


        void HandleStreamingRequest(Ptr<Socket> socket);
        void HandleStreamingRequestInternal(const std::string&);

        void SendPacket(Ptr<Socket> clientSocket, unsigned long total, unsigned long packetSize, DataRate dataRate);

        void ScheduleTx(Ptr<Socket> clientSocket, unsigned long total, DataRate dataRate);

        void WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace);

        void pourData(Ptr<Socket> clientSinkSocket);

        void Noop(Ptr<Socket> localSocket, uint32_t txSpace);

        static int client;
    };
}


#endif //SVNF_SIMU_VIDEODATASOURCE_H
