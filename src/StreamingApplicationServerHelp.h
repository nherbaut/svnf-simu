//
// Created by nherbaut on 01/06/15.
//

#ifndef SVNF_SIMU_STREAMINGAPPLICATIONSERVERHELP_H
#define SVNF_SIMU_STREAMINGAPPLICATIONSERVERHELP_H


#include <ns3/packet-sink-helper.h>
#include <ns3/address.h>




class StreamingApplicationServerHelper : ns3::PacketSinkHelper{


public:
    StreamingApplicationServerHelper(std::string protocol, ns3::Address address) : ns3::PacketSinkHelper(protocol,address){


    }

};


#endif //SVNF_SIMU_STREAMINGAPPLICATIONSERVERHELP_H
