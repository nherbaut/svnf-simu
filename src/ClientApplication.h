//
// Created by nherbaut on 01/06/15.
//

#ifndef SVNF_SIMU_STREAMINGAPPLICATIONCLIENT_H
#define SVNF_SIMU_STREAMINGAPPLICATIONCLIENT_H

#include <ns3/application.h>
#include <ns3/ptr.h>
#include <ns3/socket.h>
#include <ns3/node.h>
#include <ns3/data-rate.h>
#include <ns3/simulator.h>
#include <ns3/output-stream-wrapper.h>
#include <ns3/log.h>
#include <ns3/pcap-file-wrapper.h>

#include "commons.h"

using namespace ns3;

namespace labri {


    NS_LOG_COMPONENT_DEFINE ("ClientApplication");


    class ClientApplication : public Application {


    public:

        ClientApplication() : m_gwSignalingAddr(), m_clientDataSink(""), m_clientData(0) {

        }

        virtual ~ClientApplication() {

        }


        void Setup(Address SignalingAddr,
                   InetSocketAddress clientDataSink, ClientDataFromDataSource* clientData) {
            m_gwSignalingAddr = SignalingAddr;
            m_clientDataSink = clientDataSink;
            m_clientData = clientData;
            m_clientData->setIp(clientDataSink.GetIpv4());

        }


    private:

        Address m_gwSignalingAddr;
        InetSocketAddress m_clientDataSink;
        ClientDataFromDataSource* m_clientData;

        int m_retry = 0;

        virtual void StartApplication(void) {
            NS_LOG_FUNCTION(this);

            askForDataSourceToGW(m_clientData->toString());


        }


        virtual void askForDataSourceToGW(const std::string &resource) {
            NS_LOG_FUNCTION(this);
            Ptr<Socket> socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

            std::stringstream ss;
            ss << resource << "\t" << m_clientDataSink.GetIpv4() << ":" << m_clientDataSink.GetPort();

            socket->Bind();
            socket->Connect(this->m_gwSignalingAddr);

            Ptr<Packet> packet = Create<Packet>(reinterpret_cast<const uint8_t *>(ss.str().c_str()), ss.str().length());
            socket->Send(packet);
            socket->Close();

        }

        virtual void StopApplication(void) {


        }


    };

    NS_OBJECT_ENSURE_REGISTERED (ClientApplication);
}


#endif //SVNF_SIMU_STREAMINGAPPLICATIONCLIENT_H
