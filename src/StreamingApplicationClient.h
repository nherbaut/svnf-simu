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

using namespace ns3;

namespace labri {

    NS_LOG_COMPONENT_DEFINE ("StreamingApplicationClient");




    class StreamingApplicationClient : public Application {


    public:

        StreamingApplicationClient() : m_socket(0), m_peer() {

        }

        virtual ~StreamingApplicationClient() {
            m_socket = 0;
        }


        void Setup( Address address) {
            m_peer = address;

        }


    private:

        Ptr<Socket> m_socket;
        Address m_peer;

        virtual void StartApplication(void) {

            m_socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());

            int status = m_socket->Bind();
            if (status != 000)
                exit(120);
            status = m_socket->Connect(m_peer);
            if (status != 0) {
                std::cout << m_socket->GetErrno() << std::endl;
                exit(121);
            }

            SendDownloadRequest();

        }

        virtual void StopApplication(void) {
            m_socket->Close();

        }

        void
        ScheduleTx (void)
        {
                Time tNext (MilliSeconds (10));
                Simulator::Schedule (tNext, &StreamingApplicationClient::ReceiveRedirect, this);

        }

        void
        ReceiveRedirect (void)
        {

            Ptr<Packet> resp = m_socket->Recv();


        }

        void HandleNewURI(Ptr<Socket> socket){
            std::ostringstream buf;
            socket->Recv()->CopyData(&buf,1024L);

            NS_LOG_FUNCTION(this << "new URI" << buf.str());


        }

        void SendDownloadRequest() {
            NS_LOG_FUNCTION(this);

            Ptr<Packet> packet = Create<Packet>((const uint8_t *) "abcd", strlen("abcd"));
            int size=m_socket->Send(packet);
            NS_LOG_FUNCTION(this << size << " sent");
            m_socket->SetRecvCallback(MakeCallback(&StreamingApplicationClient::HandleNewURI,this));



        }


    };
}


#endif //SVNF_SIMU_STREAMINGAPPLICATIONCLIENT_H
