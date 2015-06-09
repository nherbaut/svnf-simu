//
// Created by nherbaut on 09/06/15.
//
#include <ns3/log-macros-enabled.h>
#include <ns3/application.h>
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
#include "GwApplication.h"
#include <ns3/tcp-socket-factory.h>
#include <string>
#include <sstream>

using namespace ns3;


namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("GwApplication");

    NS_OBJECT_ENSURE_REGISTERED (GwApplication);


    TypeId
    GwApplication::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::GwApplication")
                .SetParent<Application>();
        return tid;
    }

    GwApplication::GwApplication() : m_socket(0) {

    }

    GwApplication::~GwApplication() {
        m_socket = 0;
    }

    void GwApplication::Setup(Address address) {
        m_peer = address;
    }


    void GwApplication::HandleRead(Ptr<Socket> socket) {
        NS_LOG_FUNCTION (this);
        Ptr<Packet> data = socket->Recv();
        std::ostringstream buf;
        data->CopyData(&buf, 1024);

        char newUri[265];
        handle_redirect_uri(buf.str().c_str(), newUri, sizeof(newUri));
        Ptr<Packet> packet = Create<Packet>((const uint8_t *) newUri, strlen(newUri));
        socket->Send(packet);


    }


    bool GwApplication::HandleConnectionRequest(Ptr<Socket> socket, const Address &from) {
        NS_LOG_FUNCTION (this);
        return true;
    }

    void GwApplication::HandleAccept(Ptr<Socket> socket, const Address &from) {
        NS_LOG_FUNCTION (this);
        socket->SetRecvCallback(MakeCallback(&GwApplication::HandleRead, this));

    }


    void GwApplication::StartApplication(void) {


        uint32_t ipTos = 0;
        bool ipRecvTos = true;
        uint32_t ipTtl = 0;
        bool ipRecvTtl = true;


        m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());


        int status = m_socket->Bind(m_peer);
        if (status != 0) {
            exit(100);
        }
        status = m_socket->Listen();
        if (status != 0) {
            exit(200);
        }


        m_socket->SetAcceptCallback(
                MakeCallback(&GwApplication::HandleConnectionRequest, this),
                MakeCallback(&GwApplication::HandleAccept, this));


        m_socket->SetCloseCallbacks(
                MakeCallback(&GwApplication::HandlePeerClose, this),
                MakeCallback(&GwApplication::HandlePeerError, this));

        NS_LOG_FUNCTION (this << "callbacks registered");
    }

    void GwApplication::HandlePeerClose(Ptr<Socket> socket) {
        NS_LOG_FUNCTION (this << socket);
    }

    void GwApplication::HandlePeerError(Ptr<Socket> socket) {
        NS_LOG_FUNCTION (this << socket);
    }


    void GwApplication::StopApplication(void) {
        m_socket->Close();
    }

    void GwApplication::RespondToClientRequest() {

        char *const message = const_cast<char *>("abcd");
        Ptr<Packet> packet = Create<Packet>((const uint8_t *) message, strlen(message));
        m_socket->Send(packet);
        Ptr<Packet> resp = m_socket->Recv();


    }


}