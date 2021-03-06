//
// Created by nherbaut on 09/06/15.
//

#ifndef SVNF_SIMU_CACHINGCONTROLLERAPPLICATION_H
#define SVNF_SIMU_CACHINGCONTROLLERAPPLICATION_H

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
#include <string>
#include "commons.h"

using namespace ns3;

namespace labri {
    class CachingControllerApplication : public Application {

    public:


        CachingControllerApplication();

        static TypeId GetTypeId(void);

        virtual ~CachingControllerApplication();

        virtual void Setup( InetSocketAddress configurationAddress);

    private:


        Ptr<Socket> m_socketConfiguration;
        std::list<Ptr<Socket> > m_acceptedSockets;
        std::list<Ptr<Socket> > m_gatewaysConn;
        std::set<std::string> m_hostedResources;
        std::set<std::string> m_PendingResources;
        std::map<std::string,int> m_countResourceRequests;
        InetSocketAddress m_configuration;
        bool dirty=false;


        void HandleNewResourceAsked(const std::string& );
        void TranscodingAndDeployingDone(const std::string& );

        void handleExistingResourceAsked(const std::string &str);

        void HandleRead(Ptr<Socket> socket);

        bool HandleConnectionRequest(Ptr<Socket> socket, const Address &from);

        void HandleSignalingAccept(Ptr<Socket> socket, const Address &from);

        void HandleConfigurationAccept(Ptr<Socket> socket, const Address &from);

        std::string serializeConf() const;

        virtual void StartApplication(void);

        void HandlePeerClose(Ptr<Socket> socket);

        void HandlePeerError(Ptr<Socket> socket);


        virtual void StopApplication(void);

        void HandleClientConfigurationInput(Ptr<Socket> socket);


        void UpdateGwConfiguration();


        EventId event_Id;
    };

}


#endif //SVNF_SIMU_CACHINGCONTROLLERAPPLICATION_H
