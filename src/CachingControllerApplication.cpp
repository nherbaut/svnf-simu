//
// Created by nherbaut on 09/06/15.
//

#include "CachingControllerApplication.h"

#include <ns3/tcp-socket-factory.h>
#include "MD5.h"
#include <map>
#include <iterator>
#include <climits>
#include <boost/algorithm/string.hpp>
#include <ns3/tcp-newreno.h>

#include <ns3/config.h>
#include <ns3/string.h>

namespace labri {

    NS_LOG_COMPONENT_DEFINE ("CachingControllerApplication");

    NS_OBJECT_ENSURE_REGISTERED (CachingControllerApplication);


    CachingControllerApplication::CachingControllerApplication() : m_configuration("") {

    }

    TypeId
    CachingControllerApplication::GetTypeId(void) {
        static TypeId tid = TypeId("labri::CachingControllerApplication")
                .SetParent<Application>();
        return tid;
    }

    CachingControllerApplication::~CachingControllerApplication() {

    }

    void CachingControllerApplication::Setup(
            InetSocketAddress configurationAddress) {

        NS_LOG_FUNCTION (this << configurationAddress.GetIpv4());
        m_configuration = configurationAddress;
    }


    void CachingControllerApplication::handleExistingResourceAsked(const std::string &str) {
        NS_LOG_FUNCTION(this);

    }


    bool CachingControllerApplication::HandleConnectionRequest(Ptr<Socket> socket, const Address &from) {
        NS_LOG_FUNCTION(this);
        return true;
    }


    void CachingControllerApplication::StartApplication(void) {
        NS_LOG_FUNCTION (this);

        m_socketConfiguration = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        Ptr<TcpNewReno> newReno = DynamicCast<TcpNewReno>(m_socketConfiguration);
        /*Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("80"));
        newReno->SetAttribute("SndBufSize",ns3::UintegerValue(1000000));
        newReno->SetAttribute("SegmentSize",ns3::UintegerValue(1460));*/


        m_socketConfiguration->Bind(m_configuration);
        m_socketConfiguration->Listen();
        m_socketConfiguration->SetAcceptCallback(
                MakeCallback(&CachingControllerApplication::HandleConnectionRequest, this),
                MakeCallback(&CachingControllerApplication::HandleConfigurationAccept, this));

        NS_LOG_FUNCTION (this << "callbacks registered");


    }

    std::string CachingControllerApplication::serializeConf() const {

        std::stringstream ss;
        for (std::set<std::string>::const_iterator it = m_hostedResources.begin();
             it != m_hostedResources.end(); ++it) {
            ss << *it << ";";

        }

        return ss.str();
    }

    void CachingControllerApplication::HandleConfigurationAccept(Ptr<Socket> socket, const Address &from) {
        NS_LOG_FUNCTION (this);
        m_gatewaysConn.push_back(socket);
        socket->SetRecvCallback(MakeCallback(&CachingControllerApplication::HandleClientConfigurationInput, this));


    }


    void CachingControllerApplication::HandlePeerClose(Ptr<Socket> socket) {
        NS_LOG_FUNCTION(this);
        m_acceptedSockets.remove(socket);
    }

    void CachingControllerApplication::HandlePeerError(Ptr<Socket> socket) {
        NS_LOG_FUNCTION(this);
        m_acceptedSockets.remove(socket);
    }


    void CachingControllerApplication::StopApplication(void) {

    }

    void CachingControllerApplication::HandleClientConfigurationInput(Ptr<Socket> HandleClientConfigurationInput) {
        NS_LOG_FUNCTION(this);
        std::ostringstream buf;
        HandleClientConfigurationInput->Recv()->CopyData(&buf, INT_MAX);
        const ClientDataFromDataSource clientData(buf.str());
        HandleNewResourceAsked(clientData);



    }

    void CachingControllerApplication::UpdateGwConfiguration(
    ) {
        NS_LOG_FUNCTION(this);
        const std::string updatedConf = this->serializeConf();
        NS_LOG_FUNCTION(this << updatedConf);
        Ptr<Packet> pkt = Create<Packet>(reinterpret_cast<const uint8_t *>(updatedConf.c_str()), updatedConf.length());
        for (std::list<Ptr<Socket>>::const_iterator it = m_gatewaysConn.begin(); it != m_gatewaysConn.end(); ++it) {
            Ptr<Socket> gw = *it;
            gw->Send(pkt);
        }

    }

    void CachingControllerApplication::HandleNewResourceAsked(
            const ClientDataFromDataSource& clientData) {
        NS_LOG_FUNCTION(this);
        Time tNext(Seconds(10));
        Simulator::Schedule(tNext, &CachingControllerApplication::TranscodingAndDeployingDone, this,clientData);

    }

    void CachingControllerApplication::TranscodingAndDeployingDone(const ClientDataFromDataSource& clientData){
        this->m_hostedResources.insert(clientData.getPayloadId());
        UpdateGwConfiguration();
    }
}
