//
// Created by nherbaut on 09/06/15.
//

#ifndef SVNF_SIMU_STREAMINGEVENT_H
#define SVNF_SIMU_STREAMINGEVENT_H

#include <ns3/socket.h>
#include <ns3/ptr.h>
#include <ns3/event-id.h>

using namespace ns3;

class StreamingEvent {

public:
    Ptr<Socket> socket;
    unsigned long total;
    EventId eventId;

};


#endif //SVNF_SIMU_STREAMINGEVENT_H
