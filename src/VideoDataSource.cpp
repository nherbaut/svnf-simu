//
// Created by nherbaut on 09/06/15.
//

#include "VideoDataSource.h"
#include <ns3/tcp-socket-factory.h>
#include <ns3/log.h>
#include <climits>
#include <boost/lexical_cast.hpp>

namespace labri {
    NS_LOG_COMPONENT_DEFINE ("VideoDataSource");

    NS_OBJECT_ENSURE_REGISTERED (VideoDataSource);

    void VideoDataSource::Setup(InetSocketAddress control) {
        m_controlAddr = control;
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

    }

    void VideoDataSource::HandlePeerClose(Ptr<Socket> ptr) {

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

        NS_LOG_FUNCTION(this << "stream to" << clientIP << clientPort);
        InetSocketAddress clientSinkAddress(clientIP.c_str(),boost::lexical_cast<uint16_t>(clientPort));
        Ptr<Socket> clientSinkSocket= Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        clientSinkSocket->Bind();
        clientSinkSocket->Connect(clientSinkAddress);
        SendPacket(clientSinkSocket,1024*20);
        ptr->Close();

    }

    void
    VideoDataSource::SendPacket (Ptr<Socket> clientSocket, unsigned long total)
    {
        const unsigned long packetSize=1024;
        Ptr<Packet> packet = Create<Packet> (packetSize);
        clientSocket->Send (packet);
        total-=packetSize;
        if (total>0)
        {
            ScheduleTx (clientSocket,total);
        }
    }

    void
    VideoDataSource::ScheduleTx (Ptr<Socket> clientSocket, unsigned long total)
    {

            Time tNext (Seconds (2));
            Simulator::Schedule (tNext, &VideoDataSource::SendPacket, this);

    }
}
