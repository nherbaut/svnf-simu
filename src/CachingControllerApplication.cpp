//
// Created by nherbaut on 09/06/15.
//

#include "CachingControllerApplication.h"

#include <ns3/tcp-socket-factory.h>
#include "MD5.h"
#include <map>
#include <iterator>


namespace labri {

    NS_LOG_COMPONENT_DEFINE ("CachingControllerApplication");

    NS_OBJECT_ENSURE_REGISTERED (CachingControllerApplication);


    CachingControllerApplication::CachingControllerApplication() : m_signaling(), m_socketSignaling(0) {

    }

    TypeId
    CachingControllerApplication::GetTypeId(void) {
        static TypeId tid = TypeId("labri::CachingControllerApplication")
                .SetParent<Application>();
        return tid;
    }

    CachingControllerApplication::~CachingControllerApplication() {

    }

    void CachingControllerApplication::Setup(Address signalingAddress, Address configurationAddress) {
        m_signaling = signalingAddress;
        m_configuration = configurationAddress;
    }


    void CachingControllerApplication::HandleRead(Ptr<Socket> socket) {

    }

    void CachingControllerApplication::handleExistingResourceAsked(const std::string &str) {
        NS_LOG_FUNCTION(this);

    }


    bool CachingControllerApplication::HandleConnectionRequest(Ptr<Socket> socket, const Address &from) {
        return true;
    }

    void CachingControllerApplication::HandleSignalingAccept(Ptr<Socket> socket, const Address &from) {
        m_acceptedSockets.push_back(socket);
        socket->SetRecvCallback(MakeCallback(&CachingControllerApplication::HandleRead, this));

    }


    void CachingControllerApplication::StartApplication(void) {
        m_socketSignaling = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        m_socketConfiguration = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());


        int status = m_socketSignaling->Bind(m_signaling);
        status = m_socketSignaling->Listen();
        m_socketSignaling->SetAcceptCallback(
                MakeCallback(&CachingControllerApplication::HandleConnectionRequest, this),
                MakeCallback(&CachingControllerApplication::HandleSignalingAccept, this));
        m_socketSignaling->SetCloseCallbacks(
                MakeCallback(&CachingControllerApplication::HandlePeerClose, this),
                MakeCallback(&CachingControllerApplication::HandlePeerError, this));


        m_socketConfiguration->Bind(m_configuration);
        m_socketConfiguration->Listen();
        m_socketConfiguration->SetAcceptCallback(
                MakeCallback(&CachingControllerApplication::HandleConnectionRequest, this),
                MakeCallback(&CachingControllerApplication::HandleConfigurationAccept, this));


    }

    std::string CachingControllerApplication::serializeConf() const {

        std::stringstream ss;
        for (std::list<std::string>::const_iterator it = m_hostedResources.begin();
             it != m_hostedResources.end(); ++it) {
            ss << *it << ";";
        }

        return ss.str();
    }

    void CachingControllerApplication::HandleConfigurationAccept(Ptr<Socket> socket, const Address &from) {

        const std::string data = this->serializeConf();
        Ptr<Packet> packet = Create<Packet>(reinterpret_cast<const uint8_t *>(data.c_str()), data.length());
        socket->Send(packet);
        socket->Close();

    }


    void CachingControllerApplication::HandlePeerClose(Ptr<Socket> socket) {
        m_acceptedSockets.remove(socket);
    }

    void CachingControllerApplication::HandlePeerError(Ptr<Socket> socket) {
        m_acceptedSockets.remove(socket);
    }


    void CachingControllerApplication::StopApplication(void) {

    }

    void CachingControllerApplication::RespondToClientRequest() {

    }
}
