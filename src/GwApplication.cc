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
#include <climits>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "commons.h"

#include <bits/stream_iterator.h>

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

    GwApplication::GwApplication() : m_socketClientAccept(0), m_configurationSocket(0), m_local(), m_signalingPOPAddr(),
                                     m_signalingCPAddr(), m_configurationPOPAddr() {

    }

    GwApplication::~GwApplication() {
        m_socketClientAccept = 0;
        m_configurationSocket = 0;
    }

    void GwApplication::Setup(InetSocketAddress local,
                              InetSocketAddress signalingPOPAddr,
                              InetSocketAddress signalingCPAddr,
                              InetSocketAddress configurationPOPAddr) {
        m_local = local;
        m_signalingPOPAddr = signalingPOPAddr;
        m_signalingCPAddr = signalingCPAddr;
        m_configurationPOPAddr = configurationPOPAddr;

    }


    void GwApplication::HandleClientDownloadQuery(Ptr<Socket> socket) {

        Ptr<Packet> data = socket->Recv();
        std::ostringstream buf;
        data->CopyData(&buf, 1024);
        socket->Close();


        ClientDataFromDataSource cdfs(buf.str());

        NS_LOG_FUNCTION (this << cdfs.toString());
        const std::string resource = cdfs.getPayloadId();

        bool isManaged=false;

        if (std::find(m_handlerResources.begin(), m_handlerResources.end(), resource) != m_handlerResources.end()) {
            triggerDownloadFromPOP(buf.str());
        }
        else {
            triggerDownloadFromCP(buf.str());
            notifyPOP(resource);

        }


    }


    bool GwApplication::HandleClientConnectionRequest(Ptr<Socket> socket, const Address &from) {
        NS_LOG_FUNCTION (this);
        return true;
    }

    void GwApplication::HandleClientAccept(Ptr<Socket> socket, const Address &from) {
        NS_LOG_FUNCTION (this);
        socket->SetRecvCallback(MakeCallback(&GwApplication::HandleClientDownloadQuery, this));

    }


    void GwApplication::StartApplication(void) {

        // incoming connection from client
        m_socketClientAccept = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        m_socketClientAccept->Bind(m_local);
        m_socketClientAccept->Listen();
        m_socketClientAccept->SetAcceptCallback(
                MakeCallback(&GwApplication::HandleClientConnectionRequest, this),
                MakeCallback(&GwApplication::HandleClientAccept, this));
        m_socketClientAccept->SetCloseCallbacks(
                MakeCallback(&GwApplication::HandlePeerClose, this),
                MakeCallback(&GwApplication::HandlePeerError, this));


        m_configurationSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        int status = m_configurationSocket->Bind();
        status = m_configurationSocket->Connect(m_configurationPOPAddr);

        m_configurationSocket->SetRecvCallback(MakeCallback(&GwApplication::HandleUpdatedConfiguration, this));


        NS_LOG_FUNCTION (this << "callbacks registered");
    }

    void GwApplication::HandlePeerClose(Ptr<Socket> socket) {
        NS_LOG_FUNCTION (this << socket);

    }

    void GwApplication::HandlePeerError(Ptr<Socket> socket) {
        NS_LOG_FUNCTION (this << socket);

    }


    void GwApplication::StopApplication(void) {
        m_socketClientAccept->Close();
    }


    void GwApplication::triggerDownloadFromPOP(std::string const resource) {
        NS_LOG_FUNCTION (this);

        this->triggerDownload(m_signalingPOPAddr, resource);
    }

    void GwApplication::triggerDownloadFromCP(std::string const resource) {
        NS_LOG_FUNCTION (this);

        this->triggerDownload(m_signalingCPAddr, resource);
    }

    void GwApplication::triggerDownload(Address const target, std::string const resource) {
        NS_LOG_FUNCTION (this);
        Ptr<Socket> socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        socket->Bind();
        socket->Connect(target);
        Ptr<Packet> packet = Create<Packet>(reinterpret_cast<const uint8_t *>(resource.c_str()),
                                            resource.length());
        socket->Send(packet);
        socket->Close();
    }


    void GwApplication::notifyPOP(std::string const resource) {

        NS_LOG_FUNCTION (this << resource << resource.c_str() << resource.length());


        Ptr<Packet> packet = Create<Packet>((uint8_t *) (resource.c_str()),
                                            resource.length());
        size_t len = m_configurationSocket->Send(packet);


    }

    void GwApplication::HandleUpdatedConfiguration(Ptr<Socket> socket) {
        NS_LOG_FUNCTION (this);
        Ptr<Packet> data = socket->Recv();
        std::ostringstream ss;

        data->CopyData(&ss, INT_MAX);

        m_handlerResources.clear();
        std::string res = ss.str();

        boost::split(m_handlerResources, res, boost::is_any_of(";"));

       // dumpConf();


    }




    void GwApplication::dumpConf() {
        NS_LOG(LOG_LEVEL_DEBUG, "GW Conf is");
        for (std::vector<std::string>::const_iterator it = m_handlerResources.begin();
             it != m_handlerResources.end(); ++it) {
            NS_LOG(LOG_LEVEL_DEBUG, *it);
            std::cout << *it << std::endl;

        }

    }
}