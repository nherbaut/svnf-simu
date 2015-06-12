//
// Created by nherbaut on 09/06/15.
//

#include "VideoDataSource.h"
#include <ns3/tcp-socket-factory.h>
#include <ns3/log.h>
#include <climits>
#include <boost/lexical_cast.hpp>
#include <ns3/traced-value.h>
#include <ns3/string.h>

namespace labri {
    NS_LOG_COMPONENT_DEFINE ("VideoDataSource");

    NS_OBJECT_ENSURE_REGISTERED (VideoDataSource);

    TypeId
    VideoDataSource::GetTypeId(void) {
        static TypeId tid = TypeId("labri::VideoDataSource")
                .SetParent(Application::GetTypeId())
                .AddConstructor<VideoDataSource>()
                .AddTraceSource("bandwidth_available",
                                "Bandwidth Available",
                                MakeTraceSourceAccessor(&VideoDataSource::m_channelRate),
                                "ns3::TracedValue::Uint32Callback");


        return tid;
    }


    void VideoDataSource::Setup(InetSocketAddress control, DataRate channelDataRate) {
        m_controlAddr = control;
        m_channelRate = channelDataRate.GetBitRate();
    }

    void VideoDataSource::StartApplication() {

        NS_LOG_FUNCTION (this);
        m_controlSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

        int status = m_controlSocket->Bind(m_controlAddr);
        status = m_controlSocket->Listen();
        m_controlSocket->SetAcceptCallback(
                MakeCallback(&VideoDataSource::HandleConnectionRequest, this),
                MakeCallback(&VideoDataSource::HandleSignalingAccept, this));
        m_controlSocket->SetCloseCallbacks(
                MakeCallback(&VideoDataSource::HandlePeerClose, this),
                MakeCallback(&VideoDataSource::HandlePeerError, this));

    }

    void VideoDataSource::StopApplication() {

    }

    VideoDataSource::VideoDataSource() : m_controlAddr(""), m_controlSocket(0) {

    }

    VideoDataSource::~VideoDataSource() {

    }


    bool VideoDataSource::HandleConnectionRequest(Ptr<Socket> ptr, const Address &from) {

        return true;
    }

    void VideoDataSource::HandlePeerError(Ptr<Socket> ptr) {
        NS_LOG_FUNCTION(this);
    }

    void VideoDataSource::HandlePeerClose(Ptr<Socket> ptr) {
        NS_LOG_FUNCTION(this << ptr << "done at " << Simulator::Now().As(Time::Unit::S));

    }

    void VideoDataSource::HandleSignalingAccept(Ptr<Socket> ptr, const Address &from) {
        ptr->SetRecvCallback(MakeCallback(&VideoDataSource::HandleStreamingRequest, this));
    }

    void VideoDataSource::HandleStreamingRequest(Ptr<Socket> ptr) {
        std::ostringstream ss;
        ptr->Recv()->CopyData(&ss, INT_MAX);
        const std::string clientQuery = ss.str();
        const std::string clientInetAdd = clientQuery.substr(clientQuery.find('\t') + 1, clientQuery.length());
        const std::string resource = clientQuery.substr(0, clientQuery.find('\t'));
        const std::string clientIP = clientInetAdd.substr(0, clientInetAdd.find(':'));
        const std::string clientPort = clientInetAdd.substr(clientInetAdd.find(':') + 1, clientInetAdd.length());


        InetSocketAddress clientSinkAddress(clientIP.c_str(), boost::lexical_cast<uint16_t>(clientPort));
        Ptr<Socket> clientSinkSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        clientSinkSocket->Bind();
        clientSinkSocket->Connect(clientSinkAddress);

        SocketStats stats;
        stats.currentTxBytes = 0;
        stats.totalTxBytes = 10000000;
        m_socketData[clientSinkSocket] = stats;
        stats.dataSourceId = this->GetNode()->GetId();

        //DataRate video = DataRate("320kbps");
        Simulator::ScheduleNow(&VideoDataSource::pourData, this, clientSinkSocket);


    }

    void VideoDataSource::pourData(Ptr<Socket> clientSinkSocket) {
        clientSinkSocket->SetSendCallback(MakeCallback(&VideoDataSource::WriteUntilBufferFull, this));

        WriteUntilBufferFull(clientSinkSocket, clientSinkSocket->GetTxAvailable());
    }



    void VideoDataSource::WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace) {


        while (m_socketData[localSocket].currentTxBytes < m_socketData[localSocket].totalTxBytes &&
               localSocket->GetTxAvailable() > 0) {
            uint32_t left = m_socketData[localSocket].totalTxBytes - m_socketData[localSocket].currentTxBytes;
            uint32_t dataOffset = m_socketData[localSocket].currentTxBytes % writeSize;
            uint32_t toWrite = writeSize - dataOffset;
            toWrite = std::min(toWrite, left);
            toWrite = std::min(toWrite, localSocket->GetTxAvailable());


            int amountSent = localSocket->Send(reinterpret_cast<const uint8_t*>(buff),toWrite,0);
            if (amountSent < 0) {
                // we will be called again when new tx space becomes available.
                return;
            }
            m_socketData[localSocket].currentTxBytes += amountSent;
        }

        localSocket->Close();
        //NS_LOG_FUNCTION(this << Simulator::Now().As(Time::Unit::S) <<                      m_socketData[localSocket].currentTxBytes << m_socketData[localSocket].totalTxBytes);


    }


}
