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


using namespace ns3;
namespace  labri {
    class VideoDataSource : public Application {

    public:
        void Setup(InetSocketAddress control);

        virtual void StartApplication(void);

        virtual void StopApplication(void);

        VideoDataSource();

        virtual ~VideoDataSource();


    private:
        InetSocketAddress m_controlAddr;

        Ptr<Socket> m_controlSocket;

        DataRate m_channelRate;
        DataRate remainingRate;


        bool HandleConnectionRequest(Ptr<Socket>, const Address &);

        void HandlePeerError(Ptr<Socket>);

        void HandlePeerClose(Ptr<Socket>);

        void HandleSignalingAccept(Ptr<Socket>, const Address &);


        void HandleStreamingRequest(Ptr<Socket> ptr);

        void SendPacket(Ptr<Socket> clientSocket, unsigned long total, unsigned long packetSize, DataRate dataRate);

        void ScheduleTx(Ptr<Socket> clientSocket, unsigned long total, DataRate dataRate);
    };
}


#endif //SVNF_SIMU_VIDEODATASOURCE_H
