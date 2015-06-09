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

        static TypeId GetTypeId (void);

        virtual ~GwApplication();

        virtual void Setup(Address address);

    private:

        Ptr<Socket> m_socket;
        Address m_peer;

        void handle_redirect_uri(const char* in, char* out, const size_t max){

            strcpy(out,"newRes");

        }


        void HandleRead(Ptr<Socket> socket);

        bool HandleConnectionRequest(Ptr<Socket> socket, const Address &from);

        void HandleAccept(Ptr<Socket> socket, const Address &from);


        virtual void StartApplication(void);

        void HandlePeerClose(Ptr<Socket> socket);

        void HandlePeerError(Ptr<Socket> socket);


        virtual void StopApplication(void);

        void RespondToClientRequest();


    };

};

#endif //SVNF_SIMU_GWAPPLICATION_H
