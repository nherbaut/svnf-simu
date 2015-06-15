//
// Created by nherbaut on 09/06/15.
//

#include "VideoDataSource.h"
#include "commons.h"
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

    void VideoDataSource::HandleStreamingRequest(Ptr<Socket> socket) {
        std::ostringstream ss;
        socket->Recv()->CopyData(&ss, INT_MAX);
        const std::string clientQuery = ss.str();
        const std::string clientInetAdd = clientQuery.substr(clientQuery.find('\t') + 1, clientQuery.length());


        const std::string clientIP = clientInetAdd.substr(0, clientInetAdd.find(':'));
        const std::string clientPort = clientInetAdd.substr(clientInetAdd.find(':') + 1, clientInetAdd.length());


        InetSocketAddress clientSinkAddress(clientIP.c_str(), boost::lexical_cast<uint16_t>(clientPort));
        Ptr<Socket> clientSinkSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        clientSinkSocket->Bind();
        clientSinkSocket->Connect(clientSinkAddress);


        ::g_clientData[clientIP]->setCurrentTxBytes(0);

        m_socketIpMapping[clientSinkSocket] = clientIP;


        //DataRate video = DataRate("320kbps");
        Simulator::ScheduleNow(&VideoDataSource::pourData, this, clientSinkSocket);

        socket->Close();


    }

    void VideoDataSource::pourData(Ptr<Socket> clientSinkSocket) {

        clientSinkSocket->SetSendCallback(MakeCallback(&VideoDataSource::WriteUntilBufferFull, this));

        WriteUntilBufferFull(clientSinkSocket, clientSinkSocket->GetTxAvailable());
    }


    char indice='a';
    void VideoDataSource::WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace) {
        indice++;


        uint64_t current = ::g_clientData[m_socketIpMapping[localSocket]]->getCurrentTxBytes();
        uint64_t total = ::g_clientData[m_socketIpMapping[localSocket]]->getTotalTxBytes();
        double elapsedSecond = ::Simulator::Now().GetSeconds() -
                               ::g_clientData[m_socketIpMapping[localSocket]]->getStartDate().GetSeconds();
        double currentDataRate = current / (elapsedSecond);



        DataRate targetDataRate = ::g_clientData[m_socketIpMapping[localSocket]]->getTargetDataRate();
        if (currentDataRate > targetDataRate.GetBitRate() * 1.25) {
            NS_LOG_FUNCTION( this << "we are too high" << currentDataRate << targetDataRate);


        }
        else if ((currentDataRate < targetDataRate.GetBitRate() * 0.5) && elapsedSecond > 30) {
            NS_LOG_FUNCTION( this << "we are too low" << currentDataRate << targetDataRate);
        }


        if (current >= total) {

            localSocket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());
            localSocket->ShutdownSend();
            localSocket->Close();
            return;
        }
        while (
                ::g_clientData[m_socketIpMapping[localSocket]]->getCurrentTxBytes() < total &&
                localSocket->GetTxAvailable() > 0
                ) {

            current = current = ::g_clientData[m_socketIpMapping[localSocket]]->getCurrentTxBytes();;
            uint32_t left = total - current;
            uint32_t dataOffset = current % writeSize;
            uint32_t toWrite = writeSize - dataOffset;
            toWrite = std::min(toWrite, left);
            toWrite = std::min(toWrite, localSocket->GetTxAvailable());
            uint8_t *buff = new uint8_t[toWrite];
            std::memset(buff, indice, sizeof(buff));

            int amountSent = localSocket->Send(buff, sizeof(buff), 0);
            delete []buff;

            if (amountSent < 0) {

                // we will be called again when new tx space becomes available.
                return;
            }
            ::g_clientData[m_socketIpMapping[localSocket]]->setCurrentTxBytes(current + amountSent);
            //NS_LOG_FUNCTION(this<< amountSent << "sent" <<  ::g_clientData[m_socketIpMapping[localSocket]]->getCurrentTxBytes());
        }

        localSocket->Close();
        //NS_LOG_FUNCTION(this << Simulator::Now().As(Time::Unit::S) <<                      m_socketData[localSocket].currentTxBytes << m_socketData[localSocket].totalTxBytes);


    }


}
