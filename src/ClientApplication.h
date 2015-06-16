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

        ClientApplication() : m_gwSignalingAddr(), m_clientDataSink("") {

        }

        virtual ~ClientApplication() {

        }


        void Setup(Address SignalingAddr, InetSocketAddress clientDataSink, std::string clientDataId) {
            m_gwSignalingAddr = SignalingAddr;
            m_clientDataSink = clientDataSink;
            ClientDataFromDataSource *cdfs=ClientDataFromDataSource::fromId(clientDataId);
            cdfs->setIp(clientDataSink.GetIpv4());
            cdfs->setPort(clientDataSink.GetPort());
            m_clientDatas.push_back(clientDataId);


        }

        void Setup(Address SignalingAddr, InetSocketAddress clientDataSink,
                   std::vector<std::string> clientDataIds) {
            m_gwSignalingAddr = SignalingAddr;
            m_clientDataSink = clientDataSink;
            m_clientDatas.insert(clientDataIds.begin(), clientDataIds.end(), clientDataIds.end());


        }


    private:

        Address m_gwSignalingAddr;
        InetSocketAddress m_clientDataSink;

        std::vector<std::string> m_clientDatas;

        int m_retry = 0;

        virtual void StartApplication(void) {
            NS_LOG_FUNCTION(this);

            for (std::vector<std::string>::const_iterator itr = m_clientDatas.begin();
                 itr != m_clientDatas.end(); ++itr) {


                Simulator::Schedule(ClientDataFromDataSource::fromId(*itr)->getStartDate(), &ClientApplication::askForDataSourceToGW, this,
                                    *itr);

            }


        }


        virtual void askForDataSourceToGW(const std::string& id) {
            NS_LOG_FUNCTION(this);
            ClientDataFromDataSource *clientData = ClientDataFromDataSource::fromId(id);
            Ptr<Socket> socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

            clientData->setSinkIpAddress(m_clientDataSink.GetIpv4());
            clientData->setSinkPort(m_clientDataSink.GetPort());

            socket->Bind();
            socket->Connect(this->m_gwSignalingAddr);


            socket->Send(reinterpret_cast<const uint8_t *>(id.c_str()), id.length(),0);
            socket->Close();

        }

        virtual void StopApplication(void) {


        }


    };

    NS_OBJECT_ENSURE_REGISTERED (ClientApplication);
}


#endif //SVNF_SIMU_STREAMINGAPPLICATIONCLIENT_H
