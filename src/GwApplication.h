//
// Created by nherbaut on 08/06/15.
//

#ifndef SVNF_SIMU_GWAPPLICATION_H
#define SVNF_SIMU_GWAPPLICATION_H


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

using namespace ns3;

namespace ns3 {


    class GwApplication : public Application {


    public:


        GwApplication();

        static TypeId GetTypeId(void);

        virtual ~GwApplication();


        virtual void Setup(InetSocketAddress local,
                           InetSocketAddress signalingPOPAddr,
                           InetSocketAddress signalingCPAddr,
                           InetSocketAddress configurationPOPAddr);

    private:

        Ptr<Socket> m_socketClientAccept;
        Ptr<Socket> m_configurationSocket;
        Address m_local;
        Address m_signalingPOPAddr;
        Address m_signalingCPAddr;
        Address m_configurationPOPAddr;

        std::vector<std::string> m_handlerResources;


        void HandleClientDownloadQuery(Ptr<Socket> socket);

        bool HandleClientConnectionRequest(Ptr<Socket> socket, const Address &from);

        void HandleClientAccept(Ptr<Socket> socket, const Address &from);

        void HandleCCConfigurationUpdate(Ptr<Socket> socket);


        virtual void StartApplication(void);

        void HandlePeerClose(Ptr<Socket> socket);

        void HandlePeerError(Ptr<Socket> socket);


        virtual void StopApplication(void);


        void triggerDownloadFromPOP(std::string const resource);

        void triggerDownloadFromCP(std::string const resource);

        void notifyPOP(std::string const resource);

        void triggerDownload(Address target, std::string const resource);

        void HandleUpdatedConfiguration(Ptr<Socket> socket);

        void dumpConf();
    };

};

#endif //SVNF_SIMU_GWAPPLICATION_H
