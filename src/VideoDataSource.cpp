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
        NS_LOG_FUNCTION(this);
        ptr->SetRecvCallback(MakeCallback(&VideoDataSource::HandleStreamingRequest, this));
    }


    void VideoDataSource::HandleStreamingRequestInternal(ClientDataFromDataSource *clientData) {


        InetSocketAddress clientSinkAddress(clientData->getIp().c_str(), clientData->getPort());
        Ptr<Socket> clientSinkSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        clientSinkSocket->Bind();
        clientSinkSocket->Connect(clientSinkAddress);
        m_socketIpMapping[clientSinkSocket] = clientData->getIp();
        Simulator::ScheduleNow(&VideoDataSource::pourData, this, clientSinkSocket);


    }


    int VideoDataSource::client = 0;

    void VideoDataSource::HandleStreamingRequest(Ptr<Socket> socket) {
        NS_LOG_FUNCTION(this << "we handle " << ++client << " clients");
        std::ostringstream ss;
        socket->Recv()->CopyData(&ss, INT_MAX);
        const std::string clientQuery = ss.str();

        const std::string clientInetAdd = clientQuery.substr(clientQuery.find('\t') + 1, clientQuery.length());


        const std::string clientIP = clientInetAdd.substr(0, clientInetAdd.find(':'));
        const uint16_t port = boost::lexical_cast<uint16_t>(
                clientInetAdd.substr(clientInetAdd.find(':') + 1, clientInetAdd.length()));

        ::g_clientData[clientIP]->setPort(port);
        ::g_clientData[clientIP]->setCurrentTxBytes(0);

        this->HandleStreamingRequestInternal(::g_clientData[clientIP]);

        socket->Close();


    }

    void VideoDataSource::pourData(Ptr<Socket> clientSinkSocket) {
        /*NS_LOG_FUNCTION(this << ::g_clientData[m_socketIpMapping[clientSinkSocket]]->getIp() << "remaining" <<
                        (::g_clientData[m_socketIpMapping[clientSinkSocket]]->getTotalTxBytes() -
                         ::g_clientData[m_socketIpMapping[clientSinkSocket]]->getCurrentTxBytes()));*/
        clientSinkSocket->SetSendCallback(MakeCallback(&VideoDataSource::WriteUntilBufferFull, this));

        WriteUntilBufferFull(clientSinkSocket, clientSinkSocket->GetTxAvailable());
    }


    void VideoDataSource::WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace) {

        //NS_LOG_FUNCTION(this << ::g_clientData[m_socketIpMapping[localSocket]]->getIp());


        uint64_t current = ::g_clientData[m_socketIpMapping[localSocket]]->getCurrentTxBytes();
        uint64_t total = ::g_clientData[m_socketIpMapping[localSocket]]->getTotalTxBytes();
        double elapsedSecond = ::Simulator::Now().GetSeconds() -
                               ::g_clientData[m_socketIpMapping[localSocket]]->getStartDate().GetSeconds();
        double currentDataRate = current / (elapsedSecond);


        DataRate targetDataRate = ::g_clientData[m_socketIpMapping[localSocket]]->getTargetDataRate();

        if (currentDataRate > targetDataRate.GetBitRate() * 1.5) {

            //NS_LOG_FUNCTION(this << "high threshold");
            //unplug the callback for buffer space available
            localSocket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());


            Time next(Seconds(((1.0 * current / targetDataRate.GetBitRate()) - elapsedSecond)));
            //NS_LOG_FUNCTION(this << elapsedSecond << " --> " << (  (1.0*current / targetDataRate.GetBitRate()) -elapsedSecond));
            //NS_LOG_FUNCTION(this << "we are too high (" << current << " bits sent in " << elapsedSecond << " s ==> currDr=" << currentDataRate << " vs " << targetDataRate << ") rescheduling at " << (Simulator::Now().GetSeconds()+next.GetSeconds()));
            Simulator::Schedule(next, &VideoDataSource::HandleStreamingRequestInternal, this,
                                ::g_clientData[m_socketIpMapping[localSocket]]
            );

            m_socketIpMapping.erase(localSocket);
            localSocket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());
            localSocket->ShutdownSend();
            localSocket->Close();
            return;


        }
        else if ((currentDataRate < targetDataRate.GetBitRate() * 0.5) && elapsedSecond > 30) {
            //NS_LOG_FUNCTION(this << "low threshold");
            NS_LOG_FUNCTION(this << "we are too low" << currentDataRate << targetDataRate);
            ::g_clientData[m_socketIpMapping[localSocket]]->setDropped(true);
            ::g_clientData[m_socketIpMapping[localSocket]]->setDroppedDate(Simulator::Now());
            m_socketIpMapping.erase(localSocket);
            localSocket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());
            localSocket->ShutdownSend();
            localSocket->Close();

        }


        if (current >= total) {
            NS_LOG_FUNCTION(this << "done transmitting");
            m_socketIpMapping.erase(localSocket);
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
            Ptr<Packet> packet = Create<Packet>(toWrite);
            uint8_t* buff=new uint8_t[toWrite];
            //NS_LOG_FUNCTION(this << localSocket->GetErrno());
            int amountSent = localSocket->Send(packet);
            delete buff;

            if (amountSent < 0) {

                //NS_LOG_FUNCTION(this << "done transmitting with " << amountSent << " byte sent and " << toWrite <<                                " requested. Errno is" << localSocket->GetErrno());
                // we will be called again when new tx space becomes available.
                return;
            }
            ::g_clientData[m_socketIpMapping[localSocket]]->setCurrentTxBytes(current + amountSent);
            //NS_LOG_FUNCTION(this<< amountSent << "sent" <<  ::g_clientData[m_socketIpMapping[localSocket]]->getCurrentTxBytes());
        }
        //NS_LOG_FUNCTION(this << "done transmitting with " << localSocket->GetTxAvailable() << " remaining in buffer");

        localSocket->Close();
        //NS_LOG_FUNCTION(this << Simulator::Now().As(Time::Unit::S) <<                      m_socketData[localSocket].currentTxBytes << m_socketData[localSocket].totalTxBytes);


    }


}
