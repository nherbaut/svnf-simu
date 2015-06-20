/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/command-line.h"
#include "ClientApplication.h"
#include "GwApplication.h"
#include "CachingControllerApplication.h"
#include "VideoDataSource.h"
#include <ns3/config.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "ns3/stats-module.h"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include "commons.h"
#include <iomanip>

//
// CLIENT --- GW---- \
// CLIENT --- GW---- \
// CLIENT --- GW-----\
// CLIENT --- GW --- ROUTER --- POP
//                     |
//                     |
//                    CP
//
//


#pragma GCC diagnostic ignored "-Wwrite-strings"


using namespace ns3;


std::map<std::string, ClientDataFromDataSource *> g_clientData;

#ifdef LOG_PACKET
std::map<int, ClientStatObservedFromMain> clientStats;
#endif
extern std::map<int, int> newResourcesStartTime;
boost::uuids::basic_random_generator<boost::mt19937> gen;
NS_LOG_COMPONENT_DEFINE ("SVNF");


void
DataRateTrace(uint32_t oldValue, uint32_t newValue) {
    std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
}


std::map<int, ServerPlotData> cpDr;
std::map<int, ServerPlotData> popDr;
std::map<int, int> totalVidCountMap;

void countTotalVid(double startTimeInSec) {

    int second = (int) std::round(startTimeInSec);
    if (!totalVidCountMap.count(second)) {
        totalVidCountMap[second] = 0;
    }
    totalVidCountMap[second] += 1;

}

void PacketTraceCallBackCP(std::string context, const Ptr<const Packet> packet) {


    int second = (int) std::round(Simulator::Now().GetSeconds());
    if (!cpDr.count(second)) {
        cpDr[second] = ServerPlotData();
    }
    cpDr[second].totalSizeTransmitted += packet->GetSize();
}

void PacketTraceCallBackPOP(std::string context, const Ptr<const Packet> packet) {

    int second = (int) std::round(Simulator::Now().GetSeconds());
    if (!popDr.count(second)) {
        popDr[second] = ServerPlotData();
    }
    popDr[second].totalSizeTransmitted += packet->GetSize();
}


#ifdef LOG_PACKET

void PacketAddressTrace(std::string context, const Ptr<const Packet> packet, const Address &address) {

    int s = boost::lexical_cast<int>(context.substr(10, context.substr(10, context.length() - 10).find("/")));
    clientStats[s].stopDate = Simulator::Now();
    clientStats[s].totalSize += packet->GetSize();
}

#endif

double g_transcodingTime=3;
double g_gwUpdateDelay=10;
uint16_t g_countBeforeCache=1;
int
main(int argc, char *argv[]) {
    bool verbose = true;
    uint32_t nGW = 0;
    double mat = 1;
    Time END = Time::Max();

    double vcs = 1000.;
    double vcv = 200.;

    double peak_vcs = 1000.;
    double peak_vcv = 200.;

    int cpop = 1;



    /////////// PEAK
    uint32_t  peak_vidCount=0;
    double peak_vidStartTimeInSec=0;
    double peak_MeanArrivalTime=0;




    char popDataRate[30] = "100kbps";
    char cpDataRate[30] = "100kbps";
    uint32_t payloadSize = 100000;
    std::string popDelay = "10ms";
    std::string cpDelay = "20ms";
    std::string fname = "server-loads";
    uint32_t nDw = nGW;

    CommandLine cmd;
    cmd.AddValue("nGW", "Number of \"extra\" CSMA nodes/devices", nGW);
    cmd.AddValue("mat", "Mean arrival time for clients", mat);

    cmd.AddValue("as", "payload average size", payloadSize);
    cmd.AddValue("vcs", "video corpus size", vcs);
    cmd.AddValue("vcv", "video corpus variance", vcv);

    cmd.AddValue("PeakVcs", "Peak video corpus size", peak_vcs);
    cmd.AddValue("PeakVcv", "Peak video corpus variance", peak_vcv);

    cmd.AddValue("cpop", "remove pop", cpop);

    cmd.AddValue("popBitrate", "Data Rate for POP", popDataRate);
    cmd.AddValue("cpBitrate", "Data Rate for cp", cpDataRate);
    cmd.AddValue("popDelay", "delay of the pop", popDelay);
    cmd.AddValue("cpDelay", "delay of the pop", cpDelay);
    cmd.AddValue("fname", "file Name", fname);
    cmd.AddValue("nDw", "How many file Download?", nDw);
    cmd.AddValue("transTime", "Average Transcoding time", g_transcodingTime);
    cmd.AddValue("gwUp", "Max GW configuration refresh time", g_gwUpdateDelay);

    cmd.AddValue("pVidCount", "Peak Video Count", peak_vidCount);
    cmd.AddValue("pStart", "peak Start Time", peak_vidStartTimeInSec);
    cmd.AddValue("pMeanArrTime", "Peak Mean arrival Time", peak_MeanArrivalTime);
    cmd.AddValue("countBeforeCache", "How many request must we receive before caching", g_countBeforeCache);




    double g_transcodingTime=3;
    double g_gwUpdateDelay=10;


    cmd.Parse(argc, argv);


    fname = fname + boost::lexical_cast<std::string>(cpop) + ".csv";
    std::cout << "output written in" << fname << std::endl;
    //Config::SetDefault("ns3::TcpSocket::SndBufSize", ns3::UintegerValue(100000));
    //Config::SetDefault("ns3::TcpSocket::RcvBufSize", ns3::UintegerValue(100000));
    //Config::SetDefault("ns3::TcpSocket::SegmentSize", ns3::UintegerValue(1460));
    //Config::SetDefault("ns3::TcpSocketBase::ReTxTimef",ns3::TimeValue(Seconds(1000)));








    //LAN
    PointToPointHelper pointToPointLan;
    pointToPointLan.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    pointToPointLan.SetChannelAttribute("Delay", StringValue("0.3ms"));

    //CSMA
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaHelper.SetChannelAttribute("Delay", StringValue("1ns"));

    //POP
    PointToPointHelper pointToPointPOP;
    pointToPointPOP.SetDeviceAttribute("DataRate", DataRateValue(DataRate(popDataRate)));
    pointToPointPOP.SetChannelAttribute("Delay", StringValue(popDelay.c_str()));

    //CP
    PointToPointHelper pointToPointCP;
    pointToPointCP.SetDeviceAttribute("DataRate", DataRateValue(DataRate(cpDataRate)));
    pointToPointCP.SetChannelAttribute("Delay", StringValue(cpDelay.c_str()));


    Ptr<ExponentialRandomVariable> ev = CreateObject<ExponentialRandomVariable>();
    ev->SetAttribute("Mean", DoubleValue(mat));
    ev->SetAttribute("Bound", DoubleValue(100.));

    Ptr<ParetoRandomVariable> pv = CreateObject<ParetoRandomVariable>();
    pv->SetAttribute("Mean", DoubleValue(payloadSize));
    pv->SetAttribute("Shape", DoubleValue(3));
    pv->SetAttribute("Bound", DoubleValue(10 * payloadSize));

    Ptr<NormalRandomVariable> normal = CreateObject<NormalRandomVariable>();
    normal->SetAttribute("Mean", DoubleValue(vcs));
    normal->SetAttribute("Variance", DoubleValue(vcv));





    //LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_ALL);
    //LogComponentEnable("Simulator", LOG_LEVEL_ALL);
    //LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
    //LogComponentEnable("SVNF", LOG_LEVEL_ALL);
    //LogComponentEnable("NscTcpSocketImpl", LOG_LEVEL_ALL);

    //LogComponentEnable("TcpTxBuffer", LOG_LEVEL_ALL);
    //LogComponentEnable("TcpNewReno", LOG_LEVEL_ALL);$
    //LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_ALL);

    //LogComponentEnable("CachingControllerApplication", LOG_LEVEL_ALL);
    LogComponentEnable("GwApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("ClientApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("VideoDataSource", LOG_LEVEL_ALL);
    //LogComponentEnable("PacketSink", LOG_LEVEL_ALL);


    ///////////////////////////////////////////////::
    // BUILDING THE TOPOLOGY
    ///////////////////////////////////////////////


    Ipv4StaticRoutingHelper ipv4RoutingHelper;


    NodeContainer gwNodes;
    gwNodes.Create(nGW);

    NodeContainer clientNodes;
    clientNodes.Create(nGW);

    NodeContainer servers;
    servers.Create(3);

    Ptr<Node> cp = servers.Get(0);
    Ptr<Node> pop = servers.Get(1);
    Ptr<Node> router = servers.Get(2);


    InternetStackHelper stack;
    stack.Install(gwNodes);
    stack.Install(clientNodes);
    stack.Install(router);
    stack.Install(cp);
    stack.Install(pop);

    //P2P configuration for LAN
    Ipv4AddressHelper addressLanScheme;
    addressLanScheme.SetBase("11.0.0.0", "255.0.0.0");


    //P2P configuration for ROUTER -> CP


    Ipv4AddressHelper addressP2P_Router_CP;
    addressP2P_Router_CP.SetBase("12.0.0.0", "255.0.0.0");

    NodeContainer routerCpContainer;
    routerCpContainer.Add(router);
    routerCpContainer.Add(cp);


    NetDeviceContainer routerCPDeviceContainer;
    routerCPDeviceContainer = pointToPointCP.Install(routerCpContainer);


    Ipv4InterfaceContainer routerCp_addrs = addressP2P_Router_CP.Assign(routerCPDeviceContainer);
    const Ipv4Address cpIpV4Addr = routerCp_addrs.GetAddress(1);

    //P2P configuration for ROUTER -> POP


    Ipv4AddressHelper addressP2P_Router_POP;
    addressP2P_Router_POP.SetBase("13.0.0.0", "255.0.0.0");

    NodeContainer routerPOPContainer;
    routerPOPContainer.Add(router);
    routerPOPContainer.Add(pop);


    NetDeviceContainer routerPopDeviceContainer;
    routerPopDeviceContainer = pointToPointPOP.Install(routerPOPContainer);

    Ipv4InterfaceContainer routerPop_addrs = addressP2P_Router_POP.Assign(routerPopDeviceContainer);
    const Ipv4Address popIpV4Addr = routerPop_addrs.GetAddress(1);



    //CSMA Configuration for GW => Router


    Ipv4AddressHelper wanAddressScheme;
    wanAddressScheme.SetBase("10.0.0.0", "255.0.0.0");

    //plug all the gw with the router

    NodeContainer wanNodes;
    wanNodes.Add(router);
    wanNodes.Add(gwNodes);


    NetDeviceContainer wanDevices;
    wanDevices = csmaHelper.Install(wanNodes);

    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = wanAddressScheme.Assign(wanDevices);

    Ipv4Address routerWanIp = csmaInterfaces.GetAddress(0);


    Ipv4InterfaceContainer lanInterfaceContainers[nGW];
    //now connect the peer to the gw
    for (int i = 0; i < nGW; i++) {

        NodeContainer lan;
        lan.Add(clientNodes.Get(i));
        lan.Add(gwNodes.Get(i));


        NetDeviceContainer lanDevices;
        lanDevices = pointToPointLan.Install(lan);
        lanInterfaceContainers[i] = addressLanScheme.Assign(lanDevices);
        Ipv4Address gwAddress = lanInterfaceContainers[i].GetAddress(1);
        Ipv4Address clientAddress = lanInterfaceContainers[i].GetAddress(0);


        //client to pop and cp via gw
        {
            Ptr<Ipv4StaticRouting> staticRouteClient = ipv4RoutingHelper.GetStaticRouting(
                    lanInterfaceContainers[i].Get(0).first);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("13.0.0.0"), Ipv4Mask("255.0.0.0"), gwAddress, 1, 0);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("12.0.0.0"), Ipv4Mask("255.0.0.0"), gwAddress, 1, 0);


        }


        {
            Ptr<Ipv4StaticRouting> staticRouteClient = ipv4RoutingHelper.GetStaticRouting(
                    lanInterfaceContainers[i].Get(1).first);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("13.0.0.0"), Ipv4Mask("255.0.0.0"), routerWanIp, 1, 0);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("12.0.0.0"), Ipv4Mask("255.0.0.0"), routerWanIp, 1, 0);


        }


    }


    //POP to client via router
    {
        Ptr<Ipv4StaticRouting> staticRoutePOP = ipv4RoutingHelper.GetStaticRouting(routerPop_addrs.Get(1).first);
        staticRoutePOP->AddNetworkRouteTo(Ipv4Address("11.0.0.0"), Ipv4Mask("255.0.0.0"), routerPop_addrs.GetAddress(0),
                                          1, 0);
        staticRoutePOP->AddNetworkRouteTo(Ipv4Address("10.0.0.2"), Ipv4Mask("255.0.0.0"), routerPop_addrs.GetAddress(0),
                                          1, 0);


    }



    //CP to client via router
    {
        Ptr<Ipv4StaticRouting> staticRouteCP = ipv4RoutingHelper.GetStaticRouting(routerCp_addrs.Get(1).first);
        staticRouteCP->AddNetworkRouteTo(Ipv4Address("11.0.0.0"), Ipv4Mask("255.0.0.0"), routerCp_addrs.GetAddress(0),
                                         1, 0);
        staticRouteCP->AddNetworkRouteTo(Ipv4Address("10.0.0.2"), Ipv4Mask("255.0.0.0"), routerCp_addrs.GetAddress(0),
                                         1, 0);


    }



    //Router to client via GW
    {


        Ptr<Ipv4StaticRouting> staticRouterouter = ipv4RoutingHelper.GetStaticRouting(csmaInterfaces.Get(0).first);
        for (int i = 0; i < nGW; i++) {


            staticRouterouter->AddNetworkRouteTo(lanInterfaceContainers[i].GetAddress(0), Ipv4Mask("255.255.255.255"),
                                                 csmaInterfaces.GetAddress(i + 1),
                                                 3, 0);


        }


    }

    ///////////////////////////////////////////:
    // NOW THE APPLICATION PART
    //////////////////////////////////////////

    std::ostringstream ss;
    ss << "\tpop is " << popIpV4Addr << "\n";
    ss << "\tCP is " << cpIpV4Addr << "\n";
    //NS_LOG_UNCOND(ss.str().c_str());

    const uint16_t signalingPort = 18080;
    const uint16_t configurationPort = 18081;
    const uint16_t dataSourcePort = 18082;


    /// APPLICATION THAT WILL SEND THEIR DOWNLOAD REQUESTS


    double currentStartTime = 0;
    std::queue<std::string> clientDataQueue;

    for (int j = 0; j < std::max(nGW, nDw); ++j) {
        int id=normal->GetInteger();
        ClientDataFromDataSource *cdfs = new ClientDataFromDataSource(
                boost::lexical_cast<std::string>(id), pv->GetInteger(), DataRate("320kbps"));

        currentStartTime += ev->GetValue();

        countTotalVid(currentStartTime );
        cdfs->setStartDate(Seconds(currentStartTime ));
        cdfs->setEndDate(END);
        boost::uuids::uuid u = gen();
        cdfs->setId(boost::lexical_cast<std::string>(u));
        g_clientData[cdfs->getId()] = cdfs;


    }

    // PEAK TARFFIC

    Ptr<NormalRandomVariable> peakNormal = CreateObject<NormalRandomVariable>();
    peakNormal ->SetAttribute("Mean", DoubleValue(peak_vcs));
    peakNormal ->SetAttribute("Variance", DoubleValue(peak_vcv));


    Ptr<ExponentialRandomVariable> evPeak = CreateObject<ExponentialRandomVariable>();
    evPeak->SetAttribute("Mean", DoubleValue(peak_MeanArrivalTime));

    for(int i=0;i<peak_vidCount;++i){
    int id=peakNormal ->GetInteger();
        ClientDataFromDataSource *cdfs = new ClientDataFromDataSource(
                boost::lexical_cast<std::string>(id), pv->GetInteger(), DataRate("320kbps"));

        std::cout << id << std::endl;
        peak_vidStartTimeInSec+= evPeak->GetValue();
        countTotalVid(peak_vidStartTimeInSec);
        cdfs->setStartDate(Seconds(peak_vidStartTimeInSec));
        cdfs->setEndDate(END);
        boost::uuids::uuid u = gen();
        cdfs->setId(boost::lexical_cast<std::string>(u));
        g_clientData[cdfs->getId()] = cdfs;

    }


    std::cout << "a total of " << g_clientData.size() << " video will be downloaded" << std::endl;
    //installing app on gw and client
    for (int i = 0; i < nGW; i++) {

        std::ostringstream ss2;


        ss2 << "client" << i << "starts at" << currentStartTime;




        // GATEWAY
        const Ipv4Address gwAddress = lanInterfaceContainers[i].GetAddress(1);
        const Ipv4Address clientAddress = lanInterfaceContainers[i].GetAddress(0);
        //create the app
        Ptr<GwApplication> gwApp = CreateObject<GwApplication>();
        //create the soket to put in the app


        gwApp->Setup(InetSocketAddress(gwAddress, signalingPort),
                     InetSocketAddress(popIpV4Addr, signalingPort),
                     InetSocketAddress(cpIpV4Addr, signalingPort),
                     InetSocketAddress(popIpV4Addr, configurationPort));

        //put the application in the node
        gwNodes.Get(i)->AddApplication(gwApp);

        //setup simulator start/end date
        gwApp->SetStartTime(Seconds(0));
        gwApp->SetStopTime(END);


        // CLIENT
        //an application to trigger the download from a datasource


        Ptr<labri::ClientApplication> clientApp = CreateObject<labri::ClientApplication>();
        clientApp->Setup(InetSocketAddress(gwAddress, signalingPort),
                         InetSocketAddress(clientAddress, dataSourcePort));


        clientNodes.Get(i)->AddApplication(clientApp);


        PacketSinkHelper helper("ns3::TcpSocketFactory", InetSocketAddress(clientAddress, dataSourcePort));
        helper.Install(clientNodes.Get(i));





        //check if client receive the right amount of bytes
#ifdef LOG_PACKET
        std::ostringstream oss;
        oss << "/NodeList/" << clientNodes.Get(i)->GetId() << "/ApplicationList/*/$ns3::PacketSink/Rx";
        Config::Connect(oss.str(), MakeCallback(&PacketAddressTrace));
        ClientStatObservedFromMain stats;
        stats.startDate = Seconds(currentStartTime);
        stats.stopDate = Seconds(currentStartTime);
        clientStats[clientNodes.Get(i)->GetId()] = stats;
#endif


    }

    ////////////////////////////////////////////////////////
    //assign client data to client app
    ////////////////////////////////////////////////////////

    //get random queue of client elts
    std::vector<std::string> clientDataKey;
    transform(::g_clientData.begin(), ::g_clientData.end(), back_inserter(clientDataKey), RetrieveKey());
    std::random_shuffle(clientDataKey.begin(), clientDataKey.end());
    for (std::vector<std::string>::const_iterator it = clientDataKey.begin(); it != clientDataKey.end(); ++it) {
        clientDataQueue.push(*it);
    }

    //while we are elts
    while (!clientDataQueue.empty()) {
        for (int i = 0; (i < nGW) && (!clientDataQueue.empty()); i++) {
            Ptr<labri::ClientApplication> clientApp = DynamicCast<labri::ClientApplication>(
                    clientNodes.Get(i)->GetApplication(0));
            clientApp->addClientData(clientDataQueue.front());
            clientDataQueue.pop();
        }

    }

    //POP Cache Controller
    if (cpop >= 1) {
        Ptr<labri::CachingControllerApplication> popApp = CreateObject<labri::CachingControllerApplication>();
        popApp->Setup(
                InetSocketAddress(popIpV4Addr, configurationPort));

        pop->AddApplication(popApp);
        popApp->SetStartTime(Seconds(0));
        popApp->SetStopTime(END);
    }

    //CP Data Source
    Ptr<labri::VideoDataSource> cpDSApp = CreateObject<labri::VideoDataSource>();
    cpDSApp->Setup("CP", InetSocketAddress(cpIpV4Addr, signalingPort), DataRate(cpDataRate));
    cp->AddApplication(cpDSApp);
    cpDSApp->SetStartTime(Seconds(0));
    cpDSApp->SetStopTime(END);


    //POP Data Source
    Ptr<labri::VideoDataSource> popDSApp = CreateObject<labri::VideoDataSource>();
    popDSApp->Setup("POP", InetSocketAddress(popIpV4Addr, signalingPort), DataRate(popDataRate));
    pop->AddApplication(popDSApp);
    popDSApp->SetStartTime(Seconds(0));
    popDSApp->SetStopTime(END);


    std::cout << "computing routing" << std::endl;

    //Ipv4GlobalRoutingHelper::PopulateRoutingTables();



    std::cout << "computing routing done" << std::endl;


    //pointToPointCP.EnablePcapAll("CP");
    //pointToPointLan.EnablePcapAll("Lan");
    //pointToPointPOP.EnablePcapAll("POP");
    //pointToPointLan.EnablePcap("POP-signaling", routerPopDeviceContainer.Get(1), false);


    //pointToPointLan.EnablePcap("CP", routerCPDeviceContainer.Get(1), false);

    ///////////////////////////////////:
    // Spy for CP and POP datarate
    //////////////////////////////////

    std::ostringstream oss;
    oss << "/NodeList/" << cp->GetId() << "/DeviceList/1/$ns3::PointToPointNetDevice/PhyTxEnd";
    std::string cpConfigPath = oss.str();
    Config::Connect(cpConfigPath, MakeCallback(&PacketTraceCallBackCP));
    oss.str("");
    oss.clear();
    oss << "/NodeList/" << pop->GetId() << "/DeviceList/1/$ns3::PointToPointNetDevice/PhyTxEnd";
    std::string popConfigPath = oss.str();
    Config::Connect(popConfigPath, MakeCallback(&PacketTraceCallBackPOP));

    //std::cout << "sim run" << std::endl;
    Simulator::Run();
    //std::cout << "sim ran" << std::endl;

#ifdef LOG_PACKET
    for (int i = 0; i < nGW; i++) {
        Ptr<PacketSink> clientApp = DynamicCast<PacketSink>(clientNodes.Get(i)->GetApplication(1));
        std::stringstream ss;
        Time duration =
                clientStats[clientNodes.Get(i)->GetId()].stopDate - clientStats[clientNodes.Get(i)->GetId()].startDate;
        uint32_t totalSize = clientStats[clientNodes.Get(i)->GetId()].totalSize;
        ss << "client" << clientNodes.Get(i)->GetId() << " " << clientApp->GetTotalRx() << " (" << totalSize << ")" <<
        " in " <<
        duration.As(Time::S) << " " << clientApp->GetTotalRx() / duration.GetSeconds() / 1000 << " kBps";
        NS_LOG_UNCOND(ss.str());

    }
#endif

    //////////////////////////////::
    // Plot server Loads
    //////////////////////////////

    std::vector<int> keys;
    transform(cpDr.begin(), cpDr.end(), back_inserter(keys), RetrieveKey());
    transform(popDr.begin(), popDr.end(), back_inserter(keys), RetrieveKey());


    std::ofstream ofs(fname, std::ofstream::out);


    std::map<int, int> cumulativeDroppedBySeconds;
    std::map<int, int> cumulativeDroppedBySecondsPOP;
    std::map<int, int> cumulativeDroppedBySecondsCP;

    for (std::map<std::string, ClientDataFromDataSource *>::const_iterator itr = g_clientData.begin();
         itr != g_clientData.end(); ++itr) {
        if ((*itr).second->isDropped()) {
            int second = (int) std::round((*itr).second->getDroppedDate().GetSeconds());

            if (!cumulativeDroppedBySeconds.count(second)) {
                cumulativeDroppedBySeconds[second] = 0;
            }
            cumulativeDroppedBySeconds[second] += 1;

            if (((*itr).second->getDroppedFromId().compare("POP"))==0) {
                if (!cumulativeDroppedBySecondsPOP.count(second)) {
                    cumulativeDroppedBySecondsPOP[second] = 0;
                }
                cumulativeDroppedBySecondsPOP[second] += 1;
            }

            else if (((*itr).second->getDroppedFromId().compare("CP"))==0) {
                if (!cumulativeDroppedBySecondsCP.count(second)) {
                    cumulativeDroppedBySecondsCP[second] = 0;
                }
                cumulativeDroppedBySecondsCP[second] += 1;
            }
            else{
                std::cout << "DROP FROM " << (*itr).second->getDroppedFromId() << " weired" << std::endl;
            }
        }


    }


    int count_dropped = 0;
    int count_dropped_POP = 0;
    int count_dropped_CP = 0;
    int count_total_vid=0;

    std::pair<std::vector<int>::const_iterator, std::vector<int>::const_iterator> minmax = std::minmax_element(
            keys.begin(), keys.end());
    for (
            int i = *(minmax.first);
            i <= *(minmax.second); ++i) {

        double popValue = 0;
        double cpValue = 0;

        int count_dropped = 0;
        int count_dropped_POP = 0;
        int count_dropped_CP = 0;


        if(totalVidCountMap.count(i)!=0){
            count_total_vid+=totalVidCountMap[i];
        }

        if (cpDr.count(i)>0) {
            cpValue = cpDr[i].totalSizeTransmitted;
        }

        if (popDr.count(i)>0) {
            popValue = popDr[i].totalSizeTransmitted;
        }

        if (cumulativeDroppedBySeconds.count(i)>0) {
            count_dropped += cumulativeDroppedBySeconds[i];

        }


        if (cumulativeDroppedBySecondsPOP.count(i)>0) {
            count_dropped_POP += cumulativeDroppedBySecondsPOP[i];

        }

        if (cumulativeDroppedBySecondsCP.count(i)>0) {
            count_dropped_CP += cumulativeDroppedBySecondsCP[i];

        }


        ofs << i << "\t" << std::setprecision(3) << std::setw(10)
        << cpValue << "\t"              //2
        << popValue << "\t"             //3
        << count_dropped << "\t"        //4
        << totalVidCountMap[i]  << "\t" //5
        << count_dropped_POP << "\t"    //6
        << count_dropped_CP  <<         //7
        std::endl;


    }
    ofs.close();


    Simulator::Destroy();

    for (
            std::map<std::string, ClientDataFromDataSource *>::const_iterator itr = g_clientData.begin();
            itr != g_clientData.

                    end();

            ++itr) {
        delete (*itr).
                second;
    }


    return 0;
}




